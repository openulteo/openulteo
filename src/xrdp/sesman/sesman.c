/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   xrdp: A Remote Desktop Protocol server.
   Copyright (C) Jay Sorg 2005-2008
   Copyright (C) 2012 Ulteo SAS
   http://www.ulteo.com
   Author David Lechevalier <david@ulteo.com> 2012
*/

/**
 *
 * @file sesman.c
 * @brief Main program file
 * @author Jay Sorg
 *
 */

#include "sesman.h"
#include "thread_calls.h"
#include "thread_pool.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>




int g_sck;
int g_pid;
unsigned char g_fixedkey[8] = { 23, 82, 107, 6, 35, 78, 88, 7 };
struct config_sesman* g_cfg; /* defined in config.h */

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
tbus g_term_event = 0;
tbus g_sync_event = 0;
int stop = 0;

extern int g_thread_sck; /* in thread.c */
static THREAD_POOL* pool;
static char pid_file[256];


void DEFAULT_CC
sesman_stop(void)
{
	stop = 1;
	lock_stopwait_acquire();
	g_exit(1);
}


/******************************************************************************/
/**
 *
 * @brief Starts sesman main loop
 *
 */
static void DEFAULT_CC
sesman_main_loop(void)
{
	int in_sck;
	int error;
	int robjs_count;
	int cont;
	tbus sck_obj;
	tbus robjs[8];

	/*main program loop*/
	log_message(&(g_cfg->log), LOG_LEVEL_INFO, "sesman[sesman_main_loop]: "
				"listening...");
	g_sck = g_tcp_socket();
	g_tcp_set_non_blocking(g_sck);
	error = scp_tcp_bind(g_sck, g_cfg->listen_address, g_cfg->listen_port);
	if (error == 0)
	{
		error = g_tcp_listen(g_sck);
		if (error == 0)
		{
			sck_obj = g_create_wait_obj_from_socket(g_sck, 0);
			cont = 1;
			while (cont)
			{
				/* build the wait obj list */
				robjs_count = 0;
				robjs[robjs_count++] = sck_obj;
				robjs[robjs_count++] = g_term_event;
				robjs[robjs_count++] = g_sync_event;
				/* wait */
				if (g_obj_wait(robjs, robjs_count, 0, 0, -1) != 0)
				{
					/* error, should not get here */
					g_sleep(100);
				}
				if (g_is_wait_obj_set(g_term_event)) /* term */
				{
					break;
				}
				if (g_is_wait_obj_set(g_sync_event)) /* sync */
				{
					g_reset_wait_obj(g_sync_event);
					session_sync_start();
				}
				if (g_is_wait_obj_set(sck_obj)) /* incomming connection */
				{
					in_sck = g_tcp_accept(g_sck);
					if ((in_sck == -1) && g_tcp_last_error_would_block(g_sck))
					{
						/* should not get here */
						g_sleep(100);
					}
					else if (in_sck == -1)
					{
						/* error, should not get here */
						break;
					}
					else
					{
						/* we've got a connection, so we pass it to scp code */
						log_message(&(g_cfg->log), LOG_LEVEL_INFO, "sesman[sesman_main_loop]: "
								"new connection");
						thread_scp_start(in_sck);
						/* todo, do we have to wait here ? */
					}
				}
			}
			g_delete_wait_obj_from_socket(sck_obj);
		}
		else
		{
			log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[sesman_main_loop]: "
					"listen error %d (%s)", g_get_errno(), g_get_strerror());
		}
	}
	else
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "bind error on "
				"port '%s': %d (%s)", g_cfg->listen_port, g_get_errno(), g_get_strerror());
	}
	g_tcp_close(g_sck);
}



/************************************************************************/
int DEFAULT_CC
xml_get_xpath(xmlDocPtr doc, char* xpath, char* value)
{
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr context;
	xmlNodeSetPtr nodeset;
	xmlChar *keyword;

	context = xmlXPathNewContext(doc);
	if (context == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_get_xpath]: "
				"error in xmlXPathNewContext");
		return 1;
	}
	xpathObj = xmlXPathEvalExpression((xmlChar*) xpath, context);
	xmlXPathFreeContext(context);
	nodeset = xpathObj->nodesetval;
	if(xmlXPathNodeSetIsEmpty(nodeset))
	{
		xmlXPathFreeObject(xpathObj);
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[xml_get_xpath]: "
				"no result");
		return 1;
	}
	keyword = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
	if( keyword == 0)
	{
		xmlXPathFreeObject(xpathObj);
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_get_xpath]: "
				"Unable to get keyword");
		return 1;
	}
	g_strcpy(value, (const char*)keyword);
	xmlXPathFreeObject(xpathObj);
	xmlFree(keyword);
	return 0;
}

/************************************************************************/
int DEFAULT_CC
is_authorized(management_connection* client, char* user)
{
	int uid = -1;

	if (g_cfg->api.authentication == 0)
	{
		return 1;
	}

	if (user != NULL)
	{
		g_getuser_info(user, NULL, &uid, NULL, NULL, NULL);
	}

	if (client->cred == g_getuid() || client->cred == uid)
	{
		return 1;
	}

	return 0;
}

/************************************************************************/
int DEFAULT_CC
xml_send_info(management_connection* client, xmlDocPtr doc)
{
	xmlChar* xmlbuff;
	int buff_size, size;
	struct stream* s;

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_send_info]: "
			"data send : %s\n",xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size)
	size = s->p - s->data;
	if (g_tcp_can_send(client->socket, 10))
	{
		int sended = 0;
		int send = 0;
		while(sended < size)
		{
			send = (size-sended) > 2048 ? 2048 : size-sended;
			sended += g_tcp_send(client->socket, s->data+sended, send, 0);
			if (sended < size)
			{
				if (g_get_errno() != 0)
				{
					log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_send_info]: "
							"Error while send %s\n",g_get_strerror());
					goto end;
				}
			}
		}
		if (sended != size)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_send_info]: "
					"Error while sending data %i != %i\n",sended, size);
		}
	}
	else
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_send_info]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, strerror(g_get_errno()));
	}
end:
	free_stream(s);
	xmlFree(xmlbuff);
	return buff_size;
}

/************************************************************************/
int DEFAULT_CC
xml_send_error(management_connection* client, const char* message)
{
	xmlChar* xmlbuff;
	xmlDocPtr doc;
	xmlNodePtr node;
	struct stream* s;
	int buff_size, size;
	xmlChar* version;
	xmlChar* error;
	xmlChar* msg;


	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_send_error]: "
				"Unable to create the document");
		xmlFree(version);
		return 0;
	}
	error = xmlCharStrdup("error");
	msg = xmlCharStrdup(message);
	doc->encoding = xmlCharStrdup("UTF-8");
	node = xmlNewNode(NULL, error);
	xmlNodeSetContent(node, msg);
	xmlDocSetRootElement(doc, node);

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_send_error]: "
			"data send : %s",xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size)
	size = s->p - s->data;
	if (g_tcp_can_send(client->socket, 10))
	{
		buff_size = g_tcp_send(client->socket, s->data, size, 0);
	}
	else
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_send_error]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, strerror(g_get_errno()));
	}
	free_stream(s);
	xmlFreeDoc(doc);
	xmlFree(xmlbuff);
	xmlFree(version);
	xmlFree(error);
	xmlFree(msg);
	return buff_size;
}

/************************************************************************/
int DEFAULT_CC
xml_send_success(management_connection* client, char* message)
{
	xmlChar* xmlbuff;
	xmlDocPtr doc;
	xmlNodePtr node;
	struct stream* s;
	int buff_size, size;

	doc = xmlNewDoc(xmlCharStrdup("1.0"));
	if (doc == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_send_success]: "
				"Unable to create the document");
		return 0;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	node = xmlNewNode(NULL, xmlCharStrdup("response"));
	xmlNodeSetContent(node, xmlCharStrdup(message));
	xmlDocSetRootElement(doc, node);

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[xml_send_success]: "
			"Data send : %s", xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size)
	size = s->p - s->data;
	if (g_tcp_can_send(client->socket, 10))
	{
		buff_size = g_tcp_send(client->socket, s->data, size, 0);
	}
	else
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[xml_send_success]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, strerror(g_get_errno()));
	}
	free_stream(s);
	xmlFree(xmlbuff);
	return buff_size;
}


/************************************************************************/
int DEFAULT_CC
xml_send_key_value(management_connection* client, char* username, char* key, char* value)
{
	xmlNodePtr node, node2;
	xmlDocPtr doc;

	if (! is_authorized(client, username))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		return;
	}

	doc = xmlNewDoc(xmlCharStrdup("1.0"));
	if (doc ==NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[xml_send_key_value]: "
				"Unable to create the document");
		return 1;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	node = xmlNewNode(NULL, xmlCharStrdup("response"));
	node2 = xmlNewNode(NULL, xmlCharStrdup("user_conf"));
	xmlSetProp(node2, xmlCharStrdup("key"), xmlCharStrdup(key) );
	xmlSetProp(node2, xmlCharStrdup("username"), xmlCharStrdup(username) );
	xmlSetProp(node2, xmlCharStrdup("value"), xmlCharStrdup(value) );
	xmlAddChild(node, node2);
	xmlDocSetRootElement(doc, node);
	xml_send_info(client, doc);
	xmlFreeDoc(doc);
	return 0;
}


/************************************************************************/
int DEFAULT_CC
xml_set_key_value(management_connection* client, char* username, char* key, char* value)
{
	if (! is_authorized(client, NULL))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		return;
	}

	if (session_set_user_pref(username, key, value) == 0 )
	{
		xml_send_success(client, "SUCCESS");
	}
	else
	{
		xml_send_error(client, "Unable to set preference");
	}
	return 0;
}

/************************************************************************/
int DEFAULT_CC
xml_process_internal_command(management_connection* client, const char* request_action, char* username)
{
	if (! is_authorized(client, NULL))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		return 0;
	}

	if( g_strcmp(request_action, "disconnect") == 0)
	{
		session_update_status_by_user(username, SESMAN_SESSION_STATUS_DISCONNECTED);
		return 0;
	}

	if( g_strcmp(request_action, "logoff") == 0)
	{
		session_update_status_by_user(username, SESMAN_SESSION_STATUS_TO_DESTROY);
		return 0;
	}

	xml_send_error(client, "Unknown internal command");
	return 1;
}



/************************************************************************/
xmlDocPtr DEFAULT_CC
xml_receive_message(management_connection* client)
{
	struct stream* s;
	int data_length;
	int res = 0;
	make_stream(s);
	init_stream(s, 1024);
	xmlDocPtr doc;

	res= g_tcp_recv(client->socket, s->data, sizeof(int), 0);

	if (res != sizeof(int))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[xml_received_message]: "
				"Unable to read size header with error %s", strerror(g_get_errno()));
		return NULL;
	}
	in_uint32_be(s,data_length);
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_received_message]: "
			"data_length : %i", data_length);
	free_stream(s);
	make_stream(s);
	init_stream(s, data_length + 1);

	g_tcp_recv(client->socket, s->data, data_length, 0);
	s->data[data_length] = 0;
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[xml_received_message]: "
			"data : %s",s->data);
	doc = xmlReadMemory(s->data, data_length, "noname.xml", NULL, 0);
	free_stream(s);
	return doc;
}


/************************************************************************/
int DEFAULT_CC
send_sessions(management_connection* client)
{
	struct session_item* sess;
	xmlNodePtr node, node2, node3;
	xmlDocPtr doc;
	int count, i;
	xmlChar* version;
	xmlChar* encoding;
	xmlChar* s_node;
	xmlChar* s_node2;

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[send_sessions]: "
			"request for sessions list");

	if (! is_authorized(client, NULL))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		return;
	}

	lock_chain_acquire();
	sess = (struct session_item*)session_list_session(&count);
	lock_chain_release();

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[send_sessions]: "
			"%i count sessions",count);
	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc ==NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[send_sessions]: "
				"Unable to create the document");
		return 1;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	s_node = xmlCharStrdup("response");
	node = xmlNewNode(NULL, s_node);
	s_node2 = xmlCharStrdup("sessions");
	node2 = xmlNewNode(NULL, s_node2);
	xmlAddChild(node, node2);
	char prop[128];

	for ( i=0 ; i<count ; i++)
	{
		g_sprintf(prop, "%i", sess[i].display);
		xmlChar* s_session = xmlCharStrdup("session");
		xmlChar* s_id = xmlCharStrdup("id");
		xmlChar* s_id_value = xmlCharStrdup(prop);
		xmlChar* s_username = xmlCharStrdup("username");
		xmlChar* s_username_value = xmlCharStrdup(sess[i].name);
		xmlChar* s_status = xmlCharStrdup("status");
		xmlChar* s_status_value = xmlCharStrdup(session_get_status_string(sess[i].status));

		node3 = xmlNewNode(NULL, s_session);
		xmlSetProp(node3, s_id, s_id_value );
		xmlSetProp(node3, s_username,	s_username_value);
		xmlSetProp(node3, s_status, s_status_value );
		xmlAddChild(node2, node3);
		xmlFree(s_session);
		xmlFree(s_id);
		xmlFree(s_id_value);
		xmlFree(s_username);
		xmlFree(s_username_value);
		xmlFree(s_status);
		xmlFree(s_status_value);
	}
	xmlAddChild(node, node2);
	xmlDocSetRootElement(doc, node);
	xml_send_info(client, doc);

	xmlFree(version);
	xmlFree(s_node);
	xmlFree(s_node2);
	g_free(sess);
	xmlFreeDoc(doc);
	return 0;
}

/************************************************************************/
int DEFAULT_CC
send_session(management_connection* client, int session_id, char* user)
{
	struct session_item* sess = NULL;
	xmlNodePtr node, node2;
	xmlDocPtr doc;
	xmlChar* version;
	xmlChar* response;
	xmlChar* session;
	xmlChar* id;
	xmlChar* id_value;
	xmlChar* username;
	xmlChar* username_value;
	xmlChar* status;
	xmlChar* status_value;

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[send_session]: "
			"request for session\n");

	lock_chain_acquire();
	if (user == NULL || user[0] == '\0')
	{
		sess = session_get_by_display(session_id);
	}
	else
	{
		sess = session_get_bydata(user);
	}
	lock_chain_release();

	if( sess == NULL && session_id != 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[send_session]: "
				"the session %i did not exist",session_id);

		sess = g_malloc(sizeof(struct session_item), 1);
		sess->display = session_id;
		sess->status = SESMAN_SESSION_STATUS_UNKNOWN;
		g_snprintf(sess->name, sizeof(sess->name), "UNKNOW");
	}

	if (! is_authorized(client, sess->name))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		g_free(sess);
		return 1;
	}

	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[send_session]: "
				"Unable to create the document");
		if (sess != NULL)
		{
			g_free(sess);
		}
		return 1;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	response = xmlCharStrdup("response");
	session = xmlCharStrdup("session");
	node = xmlNewNode(NULL, response);
	node2 = xmlNewNode(NULL, session);

	if (sess != NULL)
	{
		char prop[128] = {0};
		g_sprintf(prop, "%i", sess->display);
		id = xmlCharStrdup("id");
		id_value = xmlCharStrdup(prop);
		username = xmlCharStrdup("username");
		username_value = xmlCharStrdup(sess->name);
		status = xmlCharStrdup("status");
		status_value = xmlCharStrdup(session_get_status_string(sess->status));

		xmlSetProp(node2, id, id_value);
		xmlSetProp(node2, username, username_value);
		xmlSetProp(node2, status, status_value );
		xmlAddChild(node, node2);
	}

	xmlDocSetRootElement(doc, node);
	xml_send_info(client, doc);

	xmlFree(response);
	xmlFree(session);
	xmlFree(version);

	xmlFreeDoc(doc);
	if (sess != NULL)
	{
		xmlFree(id);
		xmlFree(id_value);
		xmlFree(username);
		xmlFree(username_value);
		xmlFree(status);
		xmlFree(status_value);
		g_free(sess);
	}
	return 0;
}


/************************************************************************/
int DEFAULT_CC
change_status(management_connection* client, int session_id, int new_status)
{
	struct session_item* sess;
	xmlNodePtr node, node2;
	xmlDocPtr doc;
	xmlChar* version;
	xmlChar* response;
	xmlChar* session;
	xmlChar* username;
	xmlChar* username_value;
	xmlChar* id;
	xmlChar* id_value;
	xmlChar* status;
	xmlChar* status_value;


	char prop[128];
	int display;

	if (session_id == 0) {
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[change_status]: "
				"%i is not a valid session id", session_id);
		return 1;
	}

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[change_status]: "
			"request session status change to: %s", session_get_status_string(new_status));

	lock_chain_acquire();
	sess = session_get_by_display(session_id);
	lock_chain_release();

	if( sess == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[change_status]: "
				"The session %i did not exist", session_id);
		xml_send_error(client, "the session id of the request did not exist");
		return 1;
	}

	if (! is_authorized(client, sess->name))
	{
		xml_send_error(client, XML_AUTHENTICATION_ERROR);
		g_free(sess);
		return 1;
	}

	session_update_status_by_user(sess->name, new_status);
	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[change_status]: "
				"Unable to create the document");
		g_free(sess);
		xmlFree(version);
		xmlFreeDoc(doc);
		return 1;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	response = xmlCharStrdup("response");
	session = xmlCharStrdup("session");
	node = xmlNewNode(NULL, response);
	node2 = xmlNewNode(NULL, session);
	sprintf(prop, "%i", display);

	id = xmlCharStrdup("id");
	id_value = xmlCharStrdup(prop);
	username = xmlCharStrdup("username");
	username_value = xmlCharStrdup(sess->name);
	status = xmlCharStrdup("status");
	status_value = xmlCharStrdup(session_get_status_string(new_status));
	xmlSetProp(node2, id, id_value);
	xmlSetProp(node2, username, username_value);
	xmlSetProp(node2, status, status_value);
	xmlAddChild(node, node2);
	xmlDocSetRootElement(doc, node);
	xml_send_info(client, doc);

	xmlFreeDoc(doc);
	xmlFree(version);
	xmlFree(response);
	xmlFree(session);
	xmlFree(username);
	xmlFree(username_value);
	xmlFree(id);
	xmlFree(id_value);
	xmlFree(status);
	xmlFree(status_value);
	g_free(sess);
	return 0;
}


int close_management_connection(xmlDocPtr doc, management_connection* client)
{
	if (doc != NULL)
	{
		xmlFreeDoc(doc);
	}

	g_tcp_close(client->socket);

	return 0;
}

void* thread_routine(void* val)
{
	int job = 0;
	management_connection client;

	while(1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[thread_routine]: "
				"Wait job");

		thread_pool_wait_job(pool);

		job = thread_pool_pop_job(pool);
		if (job == 0)
		{
			continue;
		}

		client.cred = -1;
		client.socket = job;
		process_request(&client);
	}
}

/************************************************************************/
int DEFAULT_CC
process_request(management_connection* client)
{
	int session_id = 0;
	char request_type[128];
	char request_action[128];
	char session_id_string[12];
	char username[256];
	xmlDocPtr doc = NULL;

	if (g_cfg->api.authentication)
	{
		char username[1024] = {0};

		if ((g_unix_get_socket_user_cred(client->socket, &client->cred) == 0) && (g_getuser_name(username, client->cred) == 0))
		{
			log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[process_request]: "
					"New request from account %s", username);
		}
		else
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
					"Authentication error on connection %i", client->socket);

			xml_send_error(client, "Authentication Error");
			return close_management_connection(doc, client);
		}
	}

	doc = xml_receive_message(client);
	if ( doc == NULL)
	{
		return close_management_connection(NULL, client);
	}

	if (xml_get_xpath(doc, "/request/@type", request_type) == 1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[process_request]: "
				"Unable to get the request type");
		xml_send_error(client, "Unable to get the request type");
		return close_management_connection(doc, client);
	}

	if (xml_get_xpath(doc, "/request/@action", request_action) == 1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[process_request]: "
				"Unable to get the request action");
		xml_send_error(client, "Unable to get the request type");
		return close_management_connection(doc, client);
	}
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[process_request]: "
				"Request_type : '%s' ", request_type);
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[process_request]: "
				"Request_action : '%s' ", request_action);

	if( g_strcmp(request_type, "sessions") == 0)
	{
		if( g_strcmp(request_action, "list") != 0)
		{
			xml_send_error(client, "For session request type only"
					"the list action is supported");
			return close_management_connection(doc, client);
		}
		send_sessions(client);
		//xml_send_error(client,"test");
		return close_management_connection(doc, client);
	}

	if( g_strcmp(request_type, "session") == 0)
	{
		if (xml_get_xpath(doc, "/request/@id", session_id_string) == 1)
		{
			session_id_string[0] = '\0';
		}
		if (xml_get_xpath(doc, "/request/@username", username) == 1)
		{
			username[0] = '\0';
		}
		if (session_id_string[0] != '\0')
		{
			session_id = g_atoi(session_id_string);
			if(session_id == 0)
			{
				log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
						"%i is not a numeric value", session_id);
				xml_send_error(client, "Unable to convert the session id");
				return close_management_connection(doc, client);
			}
		}
		if( g_strcmp(request_action, "status") == 0)
		{
			send_session(client, session_id, username);
			return close_management_connection(doc, client);
		}
		if( g_strcmp(request_action, "logoff") == 0)
		{
			change_status(client, session_id, SESMAN_SESSION_STATUS_TO_DESTROY);
			return close_management_connection(doc, client);
		}
		if( g_strcmp(request_action, "disconnect") == 0)
		{
			change_status(client, session_id, SESMAN_SESSION_STATUS_DISCONNECTED);
			return close_management_connection(doc, client);
		}
		xml_send_error(client, "Unknown message for session");
		return close_management_connection(doc, client);
	}
	if( g_strcmp(request_type, "internal") == 0)
	{
		char username[256];
		if (xml_get_xpath(doc, "/request/@username", username) == 1)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
					"Unable to get the username\n");
			xml_send_error(client, "Unable to get the username");
			return close_management_connection(doc, client);
		}

		xml_process_internal_command(client, request_action, username);
		return close_management_connection(doc, client);
	}
	if( g_strcmp(request_type, "user_conf") == 0)
	{
		char username[256];
		char key[128];
		char value[256];
		if (xml_get_xpath(doc, "/request/@username", username) == 1)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
					"Unable to get the username\n");
			xml_send_error(client, "Unable to get the username");
			return close_management_connection(doc, client);
		}
		if (xml_get_xpath(doc, "/request/@key", key) == 1)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
					"Unable to get the key in the request");
			xml_send_error(client, "Unable to get the key in the request");
			return close_management_connection(doc, client);
		}
		if( g_strcmp(request_action, "set") == 0)
		{
			if (xml_get_xpath(doc, "/request/@value", value) == 1)
			{
				log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
						"Unable to get the value int the request\n");
				xml_send_error(client, "Unable to get the value in the request");
				return close_management_connection(doc, client);
			}

			xml_set_key_value(client, username, key, value);
			return close_management_connection(doc, client);
		}

		if( g_strcmp(request_action, "get") == 0)
		{
			if(session_get_user_pref(username, key, value) == 0)
			{
				xml_send_key_value(client, username, key, value);
			}
			else
			{
				xml_send_error(client, "Unable to get preference");
			}
			return close_management_connection(doc, client);
		}
		xml_send_error(client, "Unknown message for internal");
		return close_management_connection(doc, client);
	}
	xml_send_error(client, "Unknown message");
	return close_management_connection(doc, client);
}

THREAD_RV THREAD_CC
admin_thread(void* param)
{
	int server = g_create_unix_socket(MANAGEMENT_SOCKET_NAME);
	g_chmod_hex(MANAGEMENT_SOCKET_NAME, 0xFFFF);

	log_message(&(g_cfg->log), LOG_LEVEL_INFO, "sesman[admin_thread]: "
			"Create pool thread");
	pool = thread_pool_init_pool(g_cfg->sess.management_thread_count);

	log_message(&(g_cfg->log), LOG_LEVEL_INFO, "sesman[admin_thread]: "
			"Start pool thread");
	thread_pool_start_pool_thread(pool, thread_routine);

	xmlInitParser();
	while(1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[admin_thread]: "
				"wait connection");
		int client = g_wait_connection(server);
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[process_request]: "
				"New client connection [%i]",client);

		if (client < 0)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[process_request]: "
					"Unable to get client from management socket [%s]", strerror(g_get_errno()));
			continue;
		}

		thread_pool_push_job(pool, client);
	}
}

THREAD_RV THREAD_CC
monit_thread(void* param)
{
	lock_stopwait_acquire();
	while (stop == 0)
	{
		g_sleep(g_cfg->sess.monitoring_delay);

		while (g_waitchild() > 0);

		lock_chain_acquire();
		session_monit();
		lock_chain_release();
	}
	session_sigkill_all();

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[monit_thread]: remove XRDP temp dir");
	g_remove_dirs(XRDP_TEMP_DIR);

	g_snprintf(pid_file, 255, "%s/xrdp-sesman.pid", XRDP_PID_PATH);
	g_file_delete(pid_file);
	lock_stopwait_release();

}


/******************************************************************************/
int DEFAULT_CC
main(int argc, char** argv)
{
	int fd;
	int error;
	int daemon = 1;
	int pid;
	char pid_s[8];
	char text[256];
	char * stdout_buffer;
	char * stderr_buffer;
	int stream_buffer_size = 8096;

	if(g_is_root() != 0){
		g_printf("Error, xrdp-sesman service must be start with root privilege\n");
		return 0;
	}


	g_snprintf(pid_file, 255, "%s/xrdp-sesman.pid", XRDP_PID_PATH);
	if (1 == argc)
	{
		stdout_buffer = g_malloc(stream_buffer_size, 0);
		stderr_buffer = g_malloc(stream_buffer_size, 0);
		
		setvbuf(stdout, stdout_buffer, _IOFBF, stream_buffer_size);
		setvbuf(stderr, stderr_buffer, _IOFBF, stream_buffer_size);
	  
		/* no options on command line. normal startup */
		g_printf("starting sesman...");
		daemon = 1;
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--nodaemon")) ||
			(0 == g_strcasecmp(argv[1], "-n")) ||
			(0 == g_strcasecmp(argv[1], "-ns"))))
	{
		/* starts sesman not daemonized */
		g_printf("starting sesman in foregroud...");
		daemon = 0;
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--help")) ||
			(0 == g_strcasecmp(argv[1], "-h"))))
	{
		/* help screen */
		g_printf("sesman - xrdp session manager\n\n");
		g_printf("usage: sesman [command]\n\n");
		g_printf("command can be one of the following:\n");
		g_printf("-n, -ns, --nodaemon	starts sesman in foreground\n");
		g_printf("-k, --kill           kills running sesman\n");
		g_printf("-h, --help           shows this help\n");
		g_printf("if no command is specified, sesman is started in background");
		g_exit(0);
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--kill")) ||
			(0 == g_strcasecmp(argv[1], "-k"))))
	{
		/* killing running sesman */
		/* check if sesman is running */
		if (!g_file_exist(pid_file))
		{
			g_printf("sesman is not running (pid file not found - %s)\n", pid_file);
			g_exit(1);
		}

		fd = g_file_open(pid_file);

		if (-1 == fd)
		{
			g_printf("error opening pid file[%s]: %s\n", pid_file, g_get_strerror());
			return 1;
		}

		error = g_file_read(fd, pid_s, 7);
		if (-1 == error)
		{
			g_printf("error reading pid file: %s\n", g_get_strerror());
			g_file_close(fd);
			g_exit(error);
		}
		g_file_close(fd);
		pid = g_atoi(pid_s);

		error = g_sigterm(pid);
		if (0 != error)
		{
			g_printf("error killing sesman: %s\n", g_get_strerror());
		}
		else
		{
			g_file_delete(pid_file);
		}

		g_exit(error);
	}
	else
	{
		/* there's something strange on the command line */
		g_printf("sesman - xrdp session manager\n\n");
		g_printf("error: invalid command line\n");
		g_printf("usage: sesman [ --nodaemon | --kill | --help ]\n");
		g_exit(1);
	}

	if (g_file_exist(pid_file))
	{
		g_printf("sesman is already running.\n");
		g_printf("if it's not running, try removing ");
		g_printf(pid_file);
		g_printf("\n");
		g_exit(1);
	}

	/* reading config */
	g_cfg = g_malloc(sizeof(struct config_sesman), 1);
	if (0 == g_cfg)
	{
		g_printf("error creating config: quitting.\n");
		g_exit(1);
	}
	g_cfg->log.fd = -1; /* don't use logging before reading its config */
	if (0 != config_read(g_cfg))
	{
		g_printf("error reading config: %s\nquitting.\n", g_get_strerror());
		g_exit(1);
	}

	/* starting logging subsystem */
	error = log_start(&(g_cfg->log));

	if (error != LOG_STARTUP_OK)
	{
		switch (error)
		{
			case LOG_ERROR_MALLOC:
				g_printf("error on malloc. cannot start logging. quitting.\n");
				break;
			case LOG_ERROR_FILE_OPEN:
				g_printf("error opening log file [%s]. quitting.\n", g_cfg->log.log_file);
				break;
		}
		g_exit(1);
	}

	/* libscp initialization */
	scp_init(&(g_cfg->log));

	if (daemon)
	{
		char *stdout_buffer_bis;
		char *stderr_buffer_bis;
		
		stdout_buffer_bis = g_malloc(stream_buffer_size, 0);
		stderr_buffer_bis = g_malloc(stream_buffer_size, 0);
		
		g_memcpy((void*)stdout_buffer_bis, (void*)stdout_buffer, stream_buffer_size);
		g_memcpy((void*)stderr_buffer_bis, (void*)stderr_buffer, stream_buffer_size);
		
		g_memset((void*)stdout_buffer, '\0', stream_buffer_size);
		g_memset((void*)stderr_buffer, '\0', stream_buffer_size);
		
		/* start of daemonizing code */
		if (g_daemonize(pid_file) == 0)
		{
			g_writeln("problem daemonize");
			g_exit(1);
		}
		
		freopen(g_cfg->log.log_file, "a", stdout);
		freopen(g_cfg->log.log_file, "a", stderr);
		
		setvbuf(stdout, NULL, _IOLBF, 4096);
		setvbuf(stderr, NULL, _IOLBF, 4096);
		
		fputs(stdout_buffer_bis, stdout);
		fputs(stderr_buffer_bis, stderr);
		
		g_free(stdout_buffer_bis);
		g_free(stderr_buffer_bis);
		g_free(stdout_buffer);
		g_free(stderr_buffer);
	}

	/* initializing locks */
	lock_init();

	/* signal handling */
	g_pid = g_getpid();
	/* old style signal handling is now managed synchronously by a
	 * separate thread. uncomment this block if you need old style
	 * signal handling and comment out thread_sighandler_start()
	 * going back to old style for the time being
	 * problem with the sigaddset functions in sig.c - jts */
#if 1
	g_signal_hang_up(sig_sesman_reload_cfg); /* SIGHUP	*/
	g_signal_user_interrupt(sig_sesman_shutdown); /* SIGINT	*/
	g_signal_kill(sig_sesman_shutdown); /* SIGKILL */
	g_signal_terminate(sig_sesman_shutdown); /* SIGTERM */
//	g_signal_child_stop(sig_sesman_session_end); /* SIGCHLD */
#endif
#if 0
	thread_sighandler_start();
#endif

	/* start program main loop */
	log_message(&(g_cfg->log), LOG_LEVEL_INFO,
			"starting sesman with pid %d", g_pid);

	/* make sure the /tmp/.X11-unix directory exist */
	if (!g_directory_exist("/tmp/.X11-unix"))
	{
		g_create_dir("/tmp/.X11-unix");
		g_chmod_hex("/tmp/.X11-unix", 0x1777);
	}

	if (!g_directory_exist(XRDP_SOCKET_PATH))
	{
		g_create_dir(XRDP_SOCKET_PATH);
		g_chmod_hex(XRDP_SOCKET_PATH, 0x1777);
	}

	g_snprintf(text, 255, "xrdp_sesman_%8.8x_main_term", g_pid);
	g_term_event = g_create_wait_obj(text);
	g_snprintf(text, 255, "xrdp_sesman_%8.8x_main_sync", g_pid);
	g_sync_event = g_create_wait_obj(text);

	scp_init_mutex();
	tc_thread_create(admin_thread, 0);
	tc_thread_create(monit_thread, 0);
	sesman_main_loop();
	scp_remove_mutex();

	g_delete_wait_obj(g_term_event);
	g_delete_wait_obj(g_sync_event);

	if (!daemon)
	{
		log_end(&(g_cfg->log));
	}

	return 0;
}

