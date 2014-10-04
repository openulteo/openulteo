/**
 * Copyright (C) 2008-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010, 2013
 * Author James B. MacLean <macleajb@ednet.ns.ca> 2012
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


#include "printers.h"

extern struct log_config *l_config;
extern char username[256];


char* DEFAULT_CC
printer_convert_name(const char *name)
{
	char *new_name = NULL;
	char *pnew_name = NULL;
	char *pname = NULL;
	char *last = NULL;

	new_name = g_malloc(g_strlen(name) + 1, 1);
	pnew_name = new_name;
	pname = (char*)name;
	last = pnew_name;

	while(*pname)
	{
		if (*pname == '@')
		{
			if (last != new_name && last != pnew_name && *last != '_')
			{
				*pnew_name = '_';
				last = pnew_name;
				pnew_name++;
			}
		}
		else
		{
			if ((*pname >= 0 && *pname <= ' ') || *pname == 127 || *pname == '/' || *pname == '#' || *pname == '!' || *pname == ':' || *pname == '_')
			{
				if (last != new_name && last != pnew_name && *last != '_')
				{
					*pnew_name = '_';
					last = pnew_name;
					pnew_name++;
				}
			}
			else
			{
				*pnew_name = *pname;
				last = pnew_name;
				pnew_name++;
			}
		}
		pname++;
	}
	return new_name;
}

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
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[xml_get_xpath]: "
				"error in xmlXPathNewContext");
		return 1;
	}
	xpathObj = xmlXPathEvalExpression((xmlChar*) xpath, context);
	xmlXPathFreeContext(context);
	nodeset = xpathObj->nodesetval;
	if(xmlXPathNodeSetIsEmpty(nodeset))
	{
		xmlXPathFreeObject(xpathObj);
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[xml_get_xpath]: "
				"no result");
		return 1;
	}
	keyword = nodeset->nodeTab[0]->content;

	if( keyword == NULL)
	{
		xmlXPathFreeObject(xpathObj);
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[xml_get_xpath]: "
				"Unable to get keyword");
		return 1;
	}
	g_strcpy(value, (const char*)keyword);
	xmlXPathFreeObject(xpathObj);

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
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[xml_send_info]: "
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
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[xml_send_info]: "
				"Unable to send xml response: %s, cause: %s", xmlbuff, g_get_strerror());
	}
	free_stream(s);
	xmlFree(xmlbuff);
	return buff_size;
}

/************************************************************************/
static int DEFAULT_CC
printer_send_request(int sock, const char* request_type, const char* action, const char* username, const char* printer)
{
	xmlNodePtr node = NULL;
	xmlDocPtr doc = NULL;
	xmlChar* version = NULL;
	xmlChar* request = NULL;
	xmlChar* action_attr = NULL;
	xmlChar* action_value = NULL;
	xmlChar* type_attr = NULL;
	xmlChar* type_value = NULL;
	xmlChar* printer_attr = NULL;
	xmlChar* printer_value = NULL;
	xmlChar* username_attr = NULL;
	xmlChar* username_value = NULL;


	version = xmlCharStrdup("1.0");
	doc = xmlNewDoc(version);
	if (doc == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[printer_send_request]: "
				"Unable to create the document");
		xmlFree(version);
		return 1;
	}

	doc->encoding = xmlCharStrdup("UTF-8");
	request = xmlCharStrdup("request");
	node = xmlNewNode(NULL, request);

	if (action)
	{
		action_attr = xmlCharStrdup("action");
		action_value = xmlCharStrdup(action);
		xmlSetProp(node, action_attr, action_value);
	}

	if (request_type)
	{
		type_attr = xmlCharStrdup("type");
		type_value = xmlCharStrdup(request_type);
		xmlSetProp(node, type_attr, type_value);
	}

	if (username)
	{
		username_attr = xmlCharStrdup("username");
		username_value = xmlCharStrdup(username);
		xmlSetProp(node, username_attr, username_value);
	}

	if (printer)
	{
		printer_attr = xmlCharStrdup("printer");
		printer_value = xmlCharStrdup(printer);
		xmlSetProp(node, printer_attr, printer_value);
	}

	xmlDocSetRootElement(doc, node);
	xml_send_info(sock, doc);

	xmlFree(request);
	xmlFree(version);

	xmlFreeDoc(doc);

	if (action)
	{
		xmlFree(action_attr);
		xmlFree(action_value);
	}
	if (request_type)
	{
		xmlFree(type_attr);
		xmlFree(type_value);
	}
	if (username)
	{
		xmlFree(username_attr);
		xmlFree(username_value);
	}
	if (printer)
	{
		xmlFree(printer_attr);
		xmlFree(printer_value);
	}
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
	xmlDocPtr doc = NULL;

	res= g_tcp_recv(client, s->data, sizeof(int), 0);

	if (res != sizeof(int))
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[xml_received_message]: "
				"Unable to read size header with error %s", g_get_strerror());
		return NULL;
	}
	in_uint32_be(s,data_length);
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[xml_received_message]: "
			"data_length : %i", data_length);
	free_stream(s);
	make_stream(s);
	init_stream(s, data_length + 1);

	g_tcp_recv(client, s->data, data_length, 0);
	s->data[data_length] = 0;
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[xml_received_message]: "
			"data : %s",s->data);
	doc = xmlReadMemory(s->data, data_length, "noname.xml", NULL, 0);
	free_stream(s);
	return doc;
}

/************************************************************************/
static int DEFAULT_CC
printer_process_response(int sock, char* status_msg)
{
	xmlDocPtr doc = NULL;
	int rv = 0;

	doc = xml_receive_message(sock);
	if ( doc == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[printer_do_request]: "
				"Unable to get response from printerd service");
		return 1;
	}
	if (xml_get_xpath(doc, XML_RESPONSE_PATH, status_msg) == 0)
	{
		rv = 0;
		goto end;
	}
	if (xml_get_xpath(doc, XML_ERROR_PATH, status_msg) == 0)
	{
		rv = 0;
		goto end;
	}
	g_strcat(status_msg, "Unknow error");
	rv = 1;

end:
	if (doc != NULL)
	{
		xmlFreeDoc(doc);
	}
	return rv;

}

/************************************************************************/
static int DEFAULT_CC
printer_do_request(const char* request_type, const char* action, const char* username, const char* printer)
{
	int printerd_socket = 0;
	char* status_msg = NULL;
	int res = 0;

	printerd_socket = g_unix_connect(PRINTER_SOCKET_NAME);
	if (printerd_socket < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[printer_do_request]: "
				"Unable to connect to printerd service %s", g_get_strerror());
		return 1;
	}

	if (printer_send_request(printerd_socket, request_type, action, username, printer))
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[printer_do_request]: "
				"Unable to send printer request to printer service");
		g_tcp_close(printerd_socket);
		return 1;
	}

	status_msg = g_malloc(1024,1);
	if (printer_process_response(printerd_socket, status_msg))
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[printer_do_request]: "
				"Failed to do printer request, the printer service return %s", status_msg);
		res = 1;
	}

	g_tcp_close(printerd_socket);
	g_free(status_msg);
	return res;

}

/************************************************************************/
int DEFAULT_CC
printer_init()
{
	xmlInitParser();
	return printer_purge(username);
}

/************************************************************************/
int DEFAULT_CC
printer_dinit()
{
	xmlCleanupParser();
	return printer_purge(username);
}

/************************************************************************/
int DEFAULT_CC
printer_add(const char* username, const char* printer)
{
	return printer_do_request(REQUEST_TYPE_PRINTER, REQUEST_ACTION_ADD, username, printer);
}

/************************************************************************/
int DEFAULT_CC
printer_del(const char* username, const char* printer)
{
	return printer_do_request(REQUEST_TYPE_PRINTER, REQUEST_ACTION_DELETE, username, printer);
}

/************************************************************************/
int DEFAULT_CC
printer_purge(const char* username)
{
	return printer_do_request(REQUEST_TYPE_PRINTER, REQUEST_ACTION_PURGE, username, NULL);
}
