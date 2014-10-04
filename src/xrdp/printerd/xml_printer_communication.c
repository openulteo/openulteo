/**
 * Copyright (C) 2008 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010
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

#include "xml_printer_communication.h"

extern struct log_config *l_config;


/************************************************************************/
int DEFAULT_CC
xml_get_xpath(xmlDocPtr doc, const char* xpath, char* value)
{
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr context;
	xmlNodeSetPtr nodeset;
	xmlChar *keyword;

	context = xmlXPathNewContext(doc);
	if (context == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_get_xpath]: "
				"error in xmlXPathNewContext");
		return 1;
	}
	xpathObj = xmlXPathEvalExpression((xmlChar*) xpath, context);
	xmlXPathFreeContext(context);
	nodeset = xpathObj->nodesetval;
	if(xmlXPathNodeSetIsEmpty(nodeset))
	{
		xmlXPathFreeObject(xpathObj);
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_get_xpath]: "
				"no result");
		return 1;
	}
	keyword = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
	if( keyword == 0)
	{
		xmlXPathFreeObject(xpathObj);
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_get_xpath]: "
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
xml_send_info(int client, xmlDocPtr doc)
{
	xmlChar* xmlbuff;
	int buff_size, size;
	struct stream* s;

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "printerd[xml_send_info]: "
			"data send : %s\n",xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size);
	size = s->p - s->data;
	if (g_tcp_can_send(client, 10))
	{
		buff_size = g_tcp_send(client, s->data, size, 0);
	}
	else
	{
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "printerd[xml_send_info]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, strerror(g_get_errno()));
	}
	free_stream(s);
	xmlFree(xmlbuff);
	return buff_size;
}

/************************************************************************/
int DEFAULT_CC
xml_send_error(int client, const char* message, ...)
{
	xmlChar* xmlbuff;
	xmlDocPtr doc;
	xmlNodePtr node;
	struct stream* s;
	int buff_size, size;
	xmlChar* version;
	xmlChar* error;
	xmlChar* msg;
	va_list ap;
	char formated_message[1024] = {0};

	va_start(ap, message);
	vsnprintf(formated_message, sizeof(formated_message), message, ap);
	va_end(ap);

	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_send_error]: "
				"Unable to create the document");
		xmlFree(version);
		return 0;
	}
	error = xmlCharStrdup("error");
	msg = xmlCharStrdup(formated_message);
	doc->encoding = xmlCharStrdup("UTF-8");
	node = xmlNewNode(NULL, error);
	xmlNodeSetContent(node, msg);
	xmlDocSetRootElement(doc, node);

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_send_error]: "
			"data send : %s",xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size);
	size = s->p - s->data;
	if (g_tcp_can_send(client, 10))
	{
		buff_size = g_tcp_send(client, s->data, size, 0);
	}
	else
	{
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "printerd[xml_send_error]: "
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
xml_send_success(int client, char* message, ...)
{
	xmlChar* xmlbuff;
	xmlDocPtr doc;
	xmlNodePtr node;
	struct stream* s;
	int buff_size, size;
	va_list ap;
	char formated_message[1024] = {0};

	va_start(ap, message);
	vsnprintf(formated_message, sizeof(formated_message), message, ap);
	va_end(ap);

	doc = xmlNewDoc(xmlCharStrdup("1.0"));
	if (doc == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_send_success]: "
				"Unable to create the document");
		return 0;
	}
	doc->encoding = xmlCharStrdup("UTF-8");
	node = xmlNewNode(NULL, xmlCharStrdup("response"));
	xmlNodeSetContent(node, xmlCharStrdup(formated_message));
	xmlDocSetRootElement(doc, node);

	xmlDocDumpFormatMemory(doc, &xmlbuff, &buff_size, 1);
	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[xml_send_success]: "
			"Data send : %s", xmlbuff);

	make_stream(s);
	init_stream(s, buff_size + 6);
	out_uint32_be(s,buff_size);
	out_uint8p(s, xmlbuff, buff_size)
	size = s->p - s->data;
	if (g_tcp_can_send(client, 10))
	{
		buff_size = g_tcp_send(client, s->data, size, 0);
	}
	else
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[xml_send_success]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, strerror(g_get_errno()));
	}
	free_stream(s);
	xmlFree(xmlbuff);
	return buff_size;
}


/************************************************************************/
int DEFAULT_CC
xml_send_key_value(int client, char* username, char* key, char* value)
{
	xmlNodePtr node, node2;
	xmlDocPtr doc;

	doc = xmlNewDoc(xmlCharStrdup("1.0"));
	if (doc ==NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[xml_send_key_value]: "
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
xmlDocPtr DEFAULT_CC
xml_receive_message(int client)
{
	struct stream* s;
	int data_length;
	int res = 0;
	make_stream(s);
	init_stream(s, 1024);
	xmlDocPtr doc;

	res= g_tcp_recv(client, s->data, sizeof(int), 0);

	if (res != sizeof(int))
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[xml_received_message]: "
				"Unable to read size header with error %s", strerror(g_get_errno()));
		return NULL;
	}
	in_uint32_be(s,data_length);
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "printerd[xml_received_message]: "
			"data_length : %i", data_length);
	free_stream(s);
	make_stream(s);
	init_stream(s, data_length + 1);

	g_tcp_recv(client, s->data, data_length, 0);
	s->data[data_length] = 0;
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "printerd[xml_received_message]: "
			"data : %s",s->data);
	doc = xmlReadMemory(s->data, data_length, "noname.xml", NULL, 0);
	free_stream(s);
	return doc;
}

int close_management_connection(xmlDocPtr doc, int socket)
{
	if (doc != NULL)
	{
		xmlFreeDoc(doc);
	}
	g_tcp_close(socket);
}


char*
XML_GET_PARAMETER(int client, xmlDocPtr doc, const char* xpath, const char* error_msg )
{
	char* parameter = NULL;
	parameter = (char*)g_malloc(256, 1);

	if (parameter == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "xrdp-printerd[XML_GET_PARAMETER]: "
				"Unable to allocate memory for xml property %s", strerror(g_get_errno()));
		return NULL;
	}
	if (xml_get_xpath(doc, xpath, parameter) == 1)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "xrdp-printerd[XML_GET_PARAMETER]: %s", error_msg);
		xml_send_error(client, error_msg);
		return NULL;
	}
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "xrdp-printerd[xml_printerd_process_request]: "
				"value for '%s' : '%s'", xpath, parameter);
	return parameter;
}

/************************************************************************/
int
xml_printerd_process_request(int client)
{
	char* request_type = NULL;
	char* request_action = NULL;
	char* username = NULL;
	char* printer_name = NULL;
	xmlDocPtr doc = NULL;

	doc = xml_receive_message(client);
	if ( doc == NULL)
	{
		goto end;
	}

	request_type = XML_GET_PARAMETER(client, doc, REQUEST_TYPE_XPATH, "Unable to get the request type" );
	request_action = XML_GET_PARAMETER(client, doc, REQUEST_ACTION_XPATH, "Unable to get the request action" );


	if( request_type && g_strcmp(request_type, REQUEST_TYPE_PRINTERS) == 0)
	{
		if( request_action && g_strcmp(request_action, REQUEST_ACTION_PURGE) == 0)
		{
			if (printer_purge_all() == 0)
			{
				xml_send_success(client, "Succeed to purge all printers");
			}
			else
			{
				xml_send_error(client, "Unable to purge all printers");
			}
			goto end;
		}
		xml_send_error(client, "Unknown action for printers");
		goto end;		
	}

	if( request_type && g_strcmp(request_type, REQUEST_TYPE_PRINTER) == 0)
	{
		username = XML_GET_PARAMETER(client, doc, USER_NAME_XPATH, "Unable to get the request username");
		if( username && g_strcmp(request_action, REQUEST_ACTION_PURGE) == 0)
		{
			if (printer_purge(username) == 0)
			{
				xml_send_success(client, "Succeed to purge printers for user [%s]", username);
			}
			else
			{
				xml_send_error(client, "Unable to purge printers for user [%s]", username);
			}
			goto end;
		}

		printer_name = XML_GET_PARAMETER(client, doc, PRINTER_NAME_XPATH, "Unable to get the request printer name");
		if( printer_name && g_strcmp(request_action, REQUEST_ACTION_ADD) == 0)
		{
			if (printer_add(username, printer_name) == 0)
			{
				xml_send_success(client, "Succeed to add printer [%s] for the user [%s]", printer_name, username);
			}
			else
			{
				xml_send_error(client, "Unable to add printer [%s] for the user [%s]", printer_name, username);
			}
			goto end;
		}
		if( request_action && g_strcmp(request_action, REQUEST_ACTION_DELETE) == 0)
		{
			if (printer_del(username, printer_name) == 0)
			{
				xml_send_success(client, "Succeed to delete printer [%s] for the user [%s]", printer_name, username);
			}
			else
			{
				xml_send_error(client, "Unable to delete printer [%s] for the user [%s]", printer_name, username);
			}
			goto end;
		}
		xml_send_error(client, "Unknown action for printer");
		goto end;
	}
	xml_send_error(client, "Unknown message");

end:
	g_free(request_type);
	g_free(request_action);
	g_free(username);
	g_free(printer_name);

	close_management_connection(doc, client);
}
