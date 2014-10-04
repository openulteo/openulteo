/**
 * Copyright (C) 2010-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010, 2011, 2012, 2013
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

#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include <vchannel.h>
#include <os_calls.h>
#include <file.h>
#include "proto.h"
#include "ukbrdr.h"

static int ukbrdr_channel;
static int scim_client;
struct log_config* l_config;
static int running;

static int is_fragmented_packet = 0;
static int fragment_size;
static struct stream* splitted_packet;
static int lastCaretX = 0;
static int lastCaretY = 0;


int ukbrdr_send(char* data, int len) {
	int rv;

	rv = vchannel_send(ukbrdr_channel, data, len);
	if (rv != 0) {
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_ukbrdr[ukbrdr_send]: "
    		"Unable to send message");
	}

	return rv;
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


void handler(int sig) {
	int pid, statut;
	pid = waitpid(-1, &statut, 0);
	log_message(l_config, LOG_LEVEL_DEBUG, "ukbrdr[handler]: "
  		"A processus has ended");
	return;
}


int ukbrdr_send_init() {
	struct ukb_msg msg;

	msg.header.type = UKB_INIT;
	msg.header.flags = 0;
	msg.header.len = sizeof(msg.u.init);

	msg.u.init.version = UKB_VERSION;

	return ukbrdr_send((char*)&msg, sizeof(msg.header) + msg.header.len);
}


int ukbrdr_send_ime_status(int status) {
	struct ukb_msg msg;

	msg.header.type = UKB_IME_STATUS;
	msg.header.flags = 0;
	msg.header.len = sizeof(msg.u.ime_status);

	msg.u.ime_status.state = status;

	return ukbrdr_send((char*)&msg, sizeof(msg.header) + msg.header.len);
}


int ukbrdr_send_caret_position(int x, int y) {
	struct ukb_msg msg;

	msg.header.type = UKB_CARET_POS;
	msg.header.flags = 0;
	msg.header.len = sizeof(msg.u.caret_pos);

	msg.u.caret_pos.x = x;
	msg.u.caret_pos.y = y;

	return ukbrdr_send((char*)&msg, sizeof(msg.header) + msg.header.len);
}


void ukbrdr_process_message(struct stream* packet, int length, int total_length) {
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_message]: "
			"New message for ukbchannel");

	struct stream* s;
	struct ukb_msg* msg;
	int str_size;
	char* data;

	if(length != total_length) {
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_ukbrdr[ukbrdr_process_message]: "
			"Packet is fragmented");
		if(is_fragmented_packet == 0) {
			log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_ukbrdr[ukbrdr_process_message]: "
				"Packet is fragmented : first part");
			is_fragmented_packet = 1;
			fragment_size = length;
			make_stream(splitted_packet);
			init_stream(splitted_packet, total_length);
			g_memcpy(splitted_packet->p, packet->p, length);
			return;
		}
		else {
			g_memcpy(splitted_packet->p+fragment_size, packet->p, length);
			fragment_size += length;
			if (fragment_size == total_length) {
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[ukbrdr_process_message]: "
					"Packet is fragmented : last part");
				s = splitted_packet;
			}
			else {
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[ukbrdr_process_message]: "
					"Packet is fragmented : next part");
				return;
			}
		}
	}
	else {
		s = packet;
	}

	msg = (struct ukb_msg*)s->p;

	switch (msg->header.type) {
	case UKB_PUSH_COMPOSITION:
		str_size = msg->header.len * 2;
		data = g_malloc(str_size, 1);

		s->p = &msg->u.update_composition;
		uni_rdp_in_str(s, data, str_size, msg->header.len);

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_message]: "
								"Process composition message %s", data);

		msg->header.len = strlen(data) + 1;

		data_send(scim_client, (char*)msg, sizeof(msg->header));
		data_send(scim_client, data, msg->header.len);

		g_free(data);
		break;

	case UKB_STOP_COMPOSITION:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_message]: "
								"Process stop composition message");
		data_send(scim_client, (char*)msg, sizeof(msg->header));
		break;

	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_message]: "
				"Unknow message type : %i", msg->header.type);

	}

	if(is_fragmented_packet == 1) {
		is_fragmented_packet = 0;
		fragment_size = 0;
		free_stream(s);
	}
}

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


void process_scim_connection () {
	char scim_socket[250];
	int display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	int status;
	int i;

	// Connect to scim panel
	for (i = 0 ; i < 10 ; i++) {
		g_snprintf(scim_socket, 250, "/var/spool/xrdp/%d/ukbrdr_internal", display_num);
		scim_client = g_unix_connect(scim_socket);
		if (scim_client > 0) {
			break;
		}

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_scim_connection]: "
							"Failed to connect to scim panel");
		g_sleep(1000);
	}

	if (scim_client <= 0) {
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_ukbrdr[process_scim_connection]: "
							"Failed to connect to scim panel");
		pthread_exit (0);
	}

	while (ukbrdr_channel > 0) {
		if (g_tcp_can_recv(scim_client, 100)) {
			struct ukb_msg msg;

			status = data_recv(scim_client, (char*)&msg, sizeof(msg.header));

			if (status > 0) {
				switch(msg.header.type) {
				case UKB_IME_STATUS:
					status = data_recv(scim_client, (char*)&msg.u.ime_status, msg.header.len);
					if (status <= 0) {
						break;
					}

					log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_scim_connection]: "
																"new IME status %i", msg.u.ime_status);

					ukbrdr_send((char*)&msg, sizeof(msg.header) + msg.header.len);
					break;

				case UKB_CARET_POS:
					log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_scim_connection]: "
																"new CARET POSITION");

					status = data_recv(scim_client, (char*)&msg.u.ime_status, msg.header.len);
					if (status <= 0) {
						break;
					}

					ukbrdr_send((char*)&msg, sizeof(msg.header) + msg.header.len);
					lastCaretX = msg.u.caret_pos.x;
					lastCaretY = msg.u.caret_pos.y;
					break;


				default:
					log_message(l_config, LOG_LEVEL_WARNING, "vchannel_ukbrdr[process_scim_connection]: "
																				"Unknown message type");
					break;
				}
			}

			if (status == -1) {
				/* got an error */
				log_message(l_config, LOG_LEVEL_WARNING, "vchannel_ukbrdr[process_scim_connection]: "
											"Error while receiving data from scim panel");
				running = 0;
				continue;
			}

			if (status == 0) {
				/* disconnection */
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_scim_connection]: "
											"Scim panel is stopped");
				running = 0;
				continue;
			}
		}

		g_sleep(100);
	}
}


void *thread_scim_process (void * arg) {
	while(1) {
		process_scim_connection();

		sleep(2);
		while(ukbrdr_channel <= 0) {
			sleep(1);
		}
	}
	pthread_exit(0);
}


void process_rdp_connection() {
	struct stream* s = NULL;
	int rv;
	int length;
	int total_length;

	while(ukbrdr_channel > 0) {
		make_stream(s);
		init_stream(s, 1600);

		rv = vchannel_receive(ukbrdr_channel, s->data, &length, &total_length);
		if (rv == ERROR) {
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_ukbrdr[process_rdp_connection]: "
					"Invalid message");
			vchannel_close(ukbrdr_channel);
			free_stream(s);
			return;
		}

		switch(rv)
		{
		case ERROR:
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_ukbrdr[process_rdp_connection]: "
					"Invalid message");
			break;

		case STATUS_CONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_rdp_connection]: "
					"Status connected");
			ukbrdr_send_init();
			ukbrdr_send_caret_position(lastCaretX, lastCaretY);

			break;

		case STATUS_DISCONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_ukbrdr[process_rdp_connection]: "
					"Status disconnected");
			vchannel_close(ukbrdr_channel);
			ukbrdr_channel = 0;
			break;

		default:
			if (length == 0) {
				running = false;
				break;
			}

			ukbrdr_process_message(s, length, total_length);
			break;
		}
		free_stream(s);
	}
}

void *thread_vchannel_process(void *arg) {
	while(1) {
		process_rdp_connection();

		sleep(2);
		while(ukbrdr_channel <= 0) {
			ukbrdr_channel = vchannel_try_open("ukbrdr");
			if(ukbrdr_channel == ERROR) {
				sleep(1);
			}
		}
	}
	pthread_exit(0);
}


int ukbrdr_init() {
	char filename[256];
	struct list* names;
	struct list* values;
	char* name;
	char* value;
	int index;
	int display_num;

	display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	if(display_num == 0) {
		g_printf("ukbrdr[ukbrdr_init]: Display must be different of 0\n");
		return ERROR;
	}

	l_config = g_malloc(sizeof(struct log_config), 1);
	l_config->program_name = "ukbrdr";
	l_config->log_file = 0;
	l_config->fd = 0;
	l_config->log_level = LOG_LEVEL_DEBUG;
	l_config->enable_syslog = 0;
	l_config->syslog_level = LOG_LEVEL_DEBUG;

	names = list_create();
	names->auto_free = 1;
	values = list_create();
	values->auto_free = 1;

	g_snprintf(filename, 255, "%s/ukbrdr.conf", XRDP_CFG_PATH);
	if (file_by_name_read_section(filename, UKBRDR_CFG_GLOBAL, names, values) == 0) {
		for (index = 0; index < names->count; index++) {
			name = (char*)list_get_item(names, index);
			value = (char*)list_get_item(values, index);

			if (0 == g_strcasecmp(name, UKBRDR_CFG_NAME)) {
				if( g_strlen(value) > 1) {
					l_config->program_name = (char*)g_strdup(value);
				}
			}
		}
	}

	if (file_by_name_read_section(filename, UKBRDR_CFG_LOGGING, names, values) == 0) {
		for (index = 0; index < names->count; index++) {
			name = (char*)list_get_item(names, index);
			value = (char*)list_get_item(values, index);

			if (0 == g_strcasecmp(name, UKBRDR_CFG_LOG_LEVEL)) {
				l_config->log_level = log_text2level(value);
			}
		}
	}

	list_delete(names);
	list_delete(values);

	if(log_start(l_config) != LOG_STARTUP_OK) {
		g_printf("vchannel[vchannel_init]: Unable to start log system\n");
		return ERROR;
	}
	else {
		return LOG_STARTUP_OK;
	}

	return 0;
}


int main(int argc, char** argv, char** environ) {
	pthread_t Scim_thread, Vchannel_thread;
	void *ret;
	l_config = g_malloc(sizeof(struct log_config), 1);

	signal(SIGCHLD, handler);

	if (ukbrdr_init() != LOG_STARTUP_OK) {
		g_printf("ukbrdr[main]: Unable to init log system\n");
		g_free(l_config);
		return 1;
	}

	if (vchannel_init() == ERROR) {
		g_printf("ukbrdr[main]: Unable to init channel system\n");
		g_free(l_config);
		return 1;
	}

	ukbrdr_channel = vchannel_open("ukbrdr");
	if( ukbrdr_channel == ERROR) {
		log_message(l_config, LOG_LEVEL_ERROR, "ukbrdr[main]: "
				"Error while connecting to vchannel provider");
		g_free(l_config);
		return 1;
	}

	running = 1;

	if (pthread_create (&Scim_thread, NULL, thread_scim_process, (void*)0) < 0) {
		log_message(l_config, LOG_LEVEL_ERROR, "ukbrdr[main]: "
				"Pthread_create error for thread : Xevent_thread");
		g_free(l_config);
		return 1;
	}

	if (pthread_create (&Vchannel_thread, NULL, thread_vchannel_process, (void*)0) < 0) {
		log_message(l_config, LOG_LEVEL_ERROR, "ukbrdr[main]: "
				"Pthread_create error for thread : Vchannel_thread");
		g_free(l_config);
		return 1;
	}

	(void)pthread_join (Scim_thread, &ret);
	//(void)pthread_join (Vchannel_thread, &ret);
	g_free(l_config);
	return 0;
}
