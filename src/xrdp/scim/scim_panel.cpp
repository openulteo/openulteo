/**
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2013
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#define Uses_C_STDIO
#define Uses_C_STDLIB
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_DEBUG
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_AGENT
#include <scim.h>

#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <syslog.h>
#include "proto.h"

#define XRDP_SOCKET_TIMEOUT 100

/* Declarations picked from os_calls.h.
   Needed because direct include of this file
   makes symbol collision problems with glib */
extern "C" {
	void g_printf(const char* format, ...);
	int g_tcp_send(int sck, const void* ptr, int len, int flags);
	int g_tcp_recv(int sck, void* ptr, int len, int flags);
	int g_tcp_can_send(int sck, int millis);
	int g_tcp_can_recv(int sck, int millis);
	int g_tcp_last_error_would_block(int sck);
	int g_create_unix_socket(const char *socket_filename);
	int g_tcp_accept(int sck);
	int g_file_close(int fd);
}

#define DEBUG

#ifdef DEBUG
  #define log_message(args...) log_message_internal(args);
#else
  #define log_message(args...)
#endif


int log_message_internal(const char* msg, ...) {
	va_list ap;
	char buffer[2048] = {0};
	int len;

	va_start(ap, msg);
	len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	openlog("uxda-scim", LOG_PID|LOG_CONS, LOG_USER);
	syslog(LOG_ERR, "%s", buffer);
	closelog();

	return 0;
}

using namespace scim;

static PanelAgent *_panel_agent = 0;
static GThread *_panel_agent_thread = 0;
static ConfigModule *_config_module = 0;
static ConfigPointer _config;
static GThread *_xrdp_scim_server_thread = 0;
static int imeState = 0;
static int caretX = 0;
static int caretY = 0;

static void slot_update_factory_info(const PanelFactoryInfo &info) { } 
static void slot_show_help(const String &help) { }
static void slot_show_factory_menu(const std::vector <PanelFactoryInfo> &factories) { }


static void slot_update_spot_location(int x, int y);
static void slot_update_preedit_string(const String &str, const AttributeList &attrs) { }
static void slot_update_aux_string(const String &str, const AttributeList &attrs) { }
static void slot_update_lookup_table(const LookupTable &table) { }
static void slot_register_properties(const PropertyList &props) { }
static void slot_update_property(const Property &prop) { }
static void slot_register_helper_properties(int id, const PropertyList &props) { }
static void slot_update_helper_property(int id, const Property &prop) { }
static void slot_register_helper(int id, const HelperInfo &helper) { }
static void noop_v(void) { }
static void noop_i(int) { }

static int data_recv(int socket, char* data, int len) {
	int rcvd;

	while (len > 0) { 
		rcvd = g_tcp_recv(socket, data, len, 0);

		if (rcvd == -1) { 
			if (g_tcp_last_error_would_block(socket)) {
				/* not an error, must retry */
				g_tcp_can_recv(socket, 10);
			} else {
				/* error, stop */
				return -1;
			}
		} else if (rcvd == 0) {
			/* disconnection, stop */
			return 0;
		} else {
			/* got data */
			data += rcvd;
			len -= rcvd;
		}
	}
	return 1;
}

static int data_send(int socket, char* data, int len) {
	int sent;

	while (len > 0) { 
		sent = g_tcp_send(socket, data, len, 0);

		if (sent == -1) { 
			if (g_tcp_last_error_would_block(socket)) {
				/* not an error, must retry */
				g_tcp_can_send(socket, 10);
			} else {
				/* error, stop */
				return -1;
			}
		} else if (sent == 0) {
			/* disconnection, stop */
			return 0;
		} else {
			/* got data */
			data += sent;
			len -= sent;
		}
	}
	return 1;
}

static int ukbrdr_send_caret_position(int socket, int x, int y) {
	if (socket > 0) {
		struct ukb_msg msg;

		msg.header.type = UKB_CARET_POS;
		msg.header.flags = 0;
		msg.header.len = sizeof(msg.u.caret_pos);
		msg.u.caret_pos.x = x;
		msg.u.caret_pos.y = y;

		return data_send(socket, (char*)&msg, sizeof(msg.header) + msg.header.len);
	}

	return 1;
};


static int ukbrdr_send_ime_status(int socket, int status) {
	if (socket > 0) {
		struct ukb_msg msg;

		msg.header.type = UKB_IME_STATUS;
		msg.header.flags = 0;
		msg.header.len = sizeof(msg.u.ime_status);
		msg.u.ime_status.state = status;

		return data_send(socket, (char*)&msg, sizeof(msg.header) + msg.header.len);
	}

	return 1;
};

static gpointer panel_agent_thread_func(gpointer data) {
	if (!_panel_agent->run()) {
		fprintf(stderr, "Failed to run Panel.\n");
	}

	gdk_threads_enter();
	gtk_main_quit();
	gdk_threads_leave();
	g_thread_exit(NULL);
	return ((gpointer) NULL);
}

static bool run_panel_agent(void) {
	_panel_agent_thread = NULL;
	if (_panel_agent && _panel_agent->valid()) {
		_panel_agent_thread = g_thread_create(panel_agent_thread_func, NULL, TRUE, NULL);
	}
	return (_panel_agent_thread != NULL);
}

static int server_process_ime_message(int socket) {
	char* preedit_string = NULL;
	struct ukb_msg msg;
	int status = 1;

	if (socket == 0 || !g_tcp_can_recv(socket, 0))
		goto failed;

	status = data_recv(socket, (char*)&msg, sizeof(msg.header));
	if (status <= 0) {
		goto failed;
	}

	if (msg.header.type == UKB_PUSH_COMPOSITION) {
		preedit_string = new char[msg.header.len];
		status = data_recv(socket, preedit_string, msg.header.len);
		if (status <= 0)
			goto failed;

		log_message("new preedit string %s", preedit_string);

		_panel_agent->trigger_property(preedit_string);
		_panel_agent->move_preedit_caret(0);
	}

	if (msg.header.type == UKB_STOP_COMPOSITION) {
		log_message("new stop message");
		_panel_agent->select_candidate(0);
	}

failed:
	if (preedit_string != NULL)
		delete[] preedit_string;

	return status;
}

static int server_process_client_message(int socket) {
	int status;
	struct ukb_msg msg;

	if (socket == 0 || !g_tcp_can_recv(socket, 0)) {
		return 1;
	}

	log_message("Client message !!!!");

	status = data_recv(socket, (char*)&msg, sizeof(msg.header));
	if (status != 1) {
		return status;
	}

	if (msg.header.type == UKB_CARET_POS) {
		status = data_recv(socket, (char*) &msg.u.caret_pos, msg.header.len);
		if (status != 1) {
			return status;
		}

		log_message("New Caret position : %d, %d\n", msg.u.caret_pos.x, msg.u.caret_pos.y);
		slot_update_spot_location(msg.u.caret_pos.x, msg.u.caret_pos.y);
	}

	return 1;
}

static void server_process_loop(int xrdp_client, int vchannel_socket, int extclient_socket) {
	int vchannel_client;
	int extclient_client;
	int status = 1;
	int lastCaretX = 0;
	int lastCaretY = 0;
	int lastImeState = 0;

	/* Initial values */
	imeState = 0;
	caretX = caretY = 0;

	/* Server mainloop */
	while (status == 1) {
		unsigned int unicode;

		if (g_tcp_can_recv(xrdp_client, XRDP_SOCKET_TIMEOUT)) {
			status = data_recv(xrdp_client, (char*)(&unicode), sizeof(uint32));
			if(status != 1) { break; }

			/* Dirty hack Used to send an unicode symbol without a keypress */
			_panel_agent->select_candidate(unicode);
		}

		/* read ImeState spool */
		if (imeState != lastImeState) {
			char msg[] = {0, 1};
			int status1, status2;

			lastImeState = imeState;
			log_message("IME status change %i", imeState);

			/* send it */

			status1 = data_send(xrdp_client, &(msg[imeState]) , 1);
			status2 = ukbrdr_send_ime_status(vchannel_client, imeState);

			status = (status1 == 1 ) ? status2 : status1;
			if(status != 1) { break; }
		}

		/* manage caret position */
		if (lastCaretX != caretX || lastCaretY != caretY) {
			lastCaretX = caretX;
			lastCaretY = caretY;
			log_message("Caret position change : %d, %d", caretX, caretY);

			status = ukbrdr_send_caret_position(vchannel_client, caretX, caretY);

			if(status != 1) { break; }
		}

		/* manage ime messages (pre-edit string) */ 
		status = server_process_ime_message(vchannel_client);
		if(status != 1) { break; }

		/* manage client messages (pre-edit string) */ 
		status = server_process_client_message(extclient_client);
		if(status != 1) { break; }

		/* Manage vchannel client */
		if (vchannel_client == 0 && g_tcp_can_recv(vchannel_socket, 0)) {
			if ((vchannel_client = g_tcp_accept(vchannel_socket)) == -1) {
				log_message("Failed to accept internal vchannel socket");
				vchannel_client = 0;
			}
			log_message("VChannel client %i", vchannel_client);
		}

		/* Manage extclient client */
		if (extclient_client == 0 && g_tcp_can_recv(extclient_socket, 0)) {
			if ((extclient_client = g_tcp_accept(extclient_socket)) == -1) {
				log_message("Failed to accept internal extClient socket");
				extclient_client = 0;
			}
			log_message("ExtClient client %i", extclient_client);
		}
	}

	if (status == -1) {
		/* got an error */
		fprintf(stderr, "Connection error\n");
		return;
	}

	if (status == 0) {
		/* disconnection */
		fprintf(stderr, "Disconnected\n");
		return;
	}

}

static gpointer xrdp_scim_server_thread_func(gpointer data) {
	int num;
	char *p = getenv("DISPLAY");
	char xrdp_socket_name[250];
	char vchannel_socket_name[250];
	char extclient_socket_name[250];
	int xrdp_socket, xrdp_client;
	int vchannel_socket;
	int extclient_socket;

	/* socket name */
	sscanf(p, ":%d", &num);
	g_snprintf(xrdp_socket_name, 250, "/var/spool/xrdp/xrdp_scim_socket_72%d", num);
	g_snprintf(vchannel_socket_name, 250, "/var/spool/xrdp/%d/ukbrdr_internal", num);
	g_snprintf(extclient_socket_name, 250, "/var/spool/xrdp/%d/ukbrdr_internal_client", num);

	/* Listen loop */
	while(true) {

		/* destroy file if it already exist */
		unlink(xrdp_socket_name);
		unlink(vchannel_socket_name);
		unlink(extclient_socket_name);

		/* XRDP Connection */
		if ((xrdp_socket = g_create_unix_socket(xrdp_socket_name)) == 1) {
			/* error creating socket */
			fprintf(stderr, "Failed to create socket : %s\n", xrdp_socket_name);
			return ((gpointer) NULL);
		}

		/* VChannel Connection */
		if ((vchannel_socket = g_create_unix_socket(vchannel_socket_name)) == 1) {
			/* error creating socket */
			fprintf(stderr, "Failed to create socket : %s\n", vchannel_socket_name);
			return ((gpointer) NULL);
		}

		/* Client Connection */
		if ((extclient_socket = g_create_unix_socket(extclient_socket_name)) == 1) {
			/* error creating socket */
			fprintf(stderr, "Failed to create socket : %s\n", extclient_socket_name);
			return ((gpointer) NULL);
		}

		log_message("Waiting for incomming connection\n");

		if ((xrdp_client = g_tcp_accept(xrdp_socket)) == -1) {
			fprintf(stderr, "Failed to accept\n");
			return ((gpointer) NULL);
		}

		/* Process loop */
		server_process_loop(xrdp_client, vchannel_socket, extclient_socket);
	}

	gdk_threads_enter();
	gtk_main_quit();
	gdk_threads_leave();
	g_thread_exit(NULL);
	return ((gpointer) NULL);
}

static bool run_xrdp_scim_server(void) {
	_xrdp_scim_server_thread = NULL;
	_xrdp_scim_server_thread = g_thread_create(xrdp_scim_server_thread_func, NULL, TRUE, NULL);
	return (_xrdp_scim_server_thread != NULL);
}

static void slot_turn_on(void) {
	imeState = 1;
	log_message("IME turn on");
	fprintf(stderr, "IME enabled\n");
}

static void slot_turn_off(void) {
	imeState = 0;
	log_message("IME turn off");
	fprintf(stderr, "IME disabled\n");
}


static void slot_update_spot_location(int x, int y) {
	log_message("slot_update_spot_location %i %i", x, y);
	caretX = x;
	caretY = y;
}

static void slot_transaction_start(void) {
	gdk_threads_enter();
}

static void slot_transaction_end(void) {
	gdk_threads_leave();
}

static bool initialize_panel_agent(const String &config, const String &display, bool resident) {
	_panel_agent = new PanelAgent();
	if (!_panel_agent->initialize(config, display, resident)) {
		return false;
	}

	_panel_agent->signal_connect_transaction_start(slot(slot_transaction_start));
	_panel_agent->signal_connect_transaction_end(slot(slot_transaction_end));
	_panel_agent->signal_connect_reload_config(slot(noop_v));
	_panel_agent->signal_connect_turn_on(slot(slot_turn_on));
	_panel_agent->signal_connect_turn_off(slot(slot_turn_off));
	_panel_agent->signal_connect_update_screen(slot(noop_i));
	_panel_agent->signal_connect_update_spot_location(slot(slot_update_spot_location));
	_panel_agent->signal_connect_update_factory_info(slot(slot_update_factory_info));
	_panel_agent->signal_connect_show_help(slot(slot_show_help));
	_panel_agent->signal_connect_show_factory_menu(slot(slot_show_factory_menu));
	_panel_agent->signal_connect_show_preedit_string(slot(noop_v));
	_panel_agent->signal_connect_show_aux_string(slot(noop_v));
	_panel_agent->signal_connect_show_lookup_table(slot(noop_v));
	_panel_agent->signal_connect_hide_preedit_string(slot(noop_v));
	_panel_agent->signal_connect_hide_aux_string(slot(noop_v));
	_panel_agent->signal_connect_hide_lookup_table(slot(noop_v));
	_panel_agent->signal_connect_update_preedit_string(slot(slot_update_preedit_string));
	_panel_agent->signal_connect_update_preedit_caret(slot(noop_i));
	_panel_agent->signal_connect_update_aux_string(slot(slot_update_aux_string));
	_panel_agent->signal_connect_update_lookup_table(slot(slot_update_lookup_table));
	_panel_agent->signal_connect_register_properties(slot(slot_register_properties));
	_panel_agent->signal_connect_update_property(slot(slot_update_property));
	_panel_agent->signal_connect_register_helper_properties(slot(slot_register_helper_properties));
	_panel_agent->signal_connect_update_helper_property(slot(slot_update_helper_property));
	_panel_agent->signal_connect_register_helper(slot(slot_register_helper));
	_panel_agent->signal_connect_remove_helper(slot(noop_i));
	_panel_agent->signal_connect_lock(slot(noop_v));
	_panel_agent->signal_connect_unlock(slot(noop_v));

	return true;
}

int main(int argc, char *argv []) {
	String config_name("simple");
	String display_name("");
	char *p;

	//load config module
	_config_module = new ConfigModule(config_name);

	if (!_config_module || !_config_module->valid()) {
		fprintf(stderr, "Can not load \"simple\" Config module.\n");
		return 1;
	}

	//create config instance
	_config = _config_module->create_config();

	if (_config.null()) {
		fprintf(stderr, "Failed to create Config instance from \"simple\" Config module.\n");
		return 1;
	}

	// init threads and gtk
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	p = getenv("DISPLAY");
	display_name = String(p);

	if (!initialize_panel_agent(config_name, display_name, true)) {
		fprintf(stderr, "Failed to initialize Panel Agent!\n");
		return 1;
	}

	scim_daemon();

	if (!run_panel_agent()) {
		fprintf(stderr, "Failed to run Socket Server!\n");
		return 1;
	}

	if (!run_xrdp_scim_server()) {
		fprintf(stderr, "Failed to run Socket Server!\n");
		return 1;
	}

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	// Exiting...
	g_thread_join(_panel_agent_thread);
	_config.reset();
	fprintf(stderr, "Successfully exited.\n");
	return 0;
}
