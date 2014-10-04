/**
 * Copyright (C) 2010-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010, 2012
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

#include "printer.h"
#include <cups/cups.h>
//#include <cups/i18n.h> /* hardy problems */
#include <errno.h>
#include <zlib.h>
#include <sys/inotify.h>
#include <dirent.h>

extern struct log_config *l_config;


/************************************************************************/
static const char *
password_handler(const char *prompt)
{
	//TODO: find a way to manage password
	(void)prompt;
	return NULL;
}

/************************************************************************/
static char*
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
int
printer_server_connect(http_t **http )
{
	if (! *http)
	{
		*http = httpConnectEncrypt(cupsServer(), ippPort(), cupsEncryption());
		if (*http == NULL)
		{
			return 1;
		}
	}
	cupsSetPasswordCB(password_handler);
	return 0;
}

/************************************************************************/
int
printer_server_disconnect(http_t *http )
{
	httpClose(http);
	return 0;
}

/************************************************************************/
int
printer_add_printer(http_t* http, char* lp_name)
{
	ipp_t *request = NULL;         /* IPP Request */
	ipp_t *response =NULL;         /* IPP Response */
	char uri[HTTP_MAX_URI] = {0};  /* URI for printer/class */

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_add_printer]: "
			"Add_printer %s of type %s ", lp_name, DEVICE_URI);
	request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
			"localhost", 0, "/printers/%s", lp_name);

	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
			"printer-uri", NULL, uri);

	ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI, "device-uri", NULL, DEVICE_URI);

	if ((response = cupsDoRequest(http, request, "/admin/")) == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_add_printer]: "
				" %s", cupsLastErrorString());
		return 1;
	}
	else if (response->request.status.status_code > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_add_printer]: "
				" %s", cupsLastErrorString());
		ippDelete(response);
		return 1;
	}
    ippDelete(response);
    return (0);
}

/************************************************************************/
struct list*
printer_get_printer_list(http_t* http)
{
	ipp_t *request = NULL;         /* IPP Request */
	ipp_t *response =NULL;         /* IPP Response */
	char uri[HTTP_MAX_URI] = {0};  /* URI for printer/class */
	ipp_attribute_t *attr = NULL;  /* IPP attribute */
	struct list* printers = NULL;
	char* printer_name = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_get_printer_list]: "
			"Get printer list");
	request = ippNewRequest(CUPS_GET_PRINTERS);

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
			"localhost", 0, "/printers");

	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
			"printer-uri", NULL, uri);

	ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI, "device-uri", NULL, DEVICE_URI);

	if ((response = cupsDoRequest(http, request, "/admin/")) == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_get_printer_list]: "
				"%s", cupsLastErrorString());
		goto fail;
	}
	else if (response->request.status.status_code > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_get_printer_list]: "
				"%s", cupsLastErrorString());
		goto fail;
	}
	printers = list_create();

	for (attr = response->attrs; attr != NULL; attr = attr->next) {
		if (attr->name == NULL)
		{
			continue;
		}

		if (g_strcmp(attr->name, "printer-name") == 0)
		{
			printer_name = attr->values[0].string.text;
		}

		if ((g_strcmp(attr->name, "device-uri") == 0) && (g_strcmp(attr->values[0].string.text, DEVICE_URI) == 0))
		{
			list_add_item(printers, (long)g_strdup(printer_name));
		}
	}

	ippDelete(response);
	return printers;

fail:
	g_free(printers);
	ippDelete(response);
	return NULL;
}

/************************************************************************/
int
printer_del_printer(http_t* http, char* lp_name)
{
	ipp_t *request;         /* IPP Request */
	ipp_t *response;        /* IPP Response */
	char uri[HTTP_MAX_URI];	/* URI for printer/class */

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_del_printer]: "
			"delete_printer %s", lp_name);
	request = ippNewRequest(CUPS_DELETE_PRINTER);
	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                 "localhost", 0, "/printers/%s", lp_name);
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
             "printer-uri", NULL, uri);

	if ((response = cupsDoRequest(http, request, "/admin/")) == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_del_printer]: "
				" %s", cupsLastErrorString());
		return 1;
	}
	else if (response->request.status.status_code > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_del_printer]: "
				" %s", cupsLastErrorString());
		ippDelete(response);
		return 1;
	}
	ippDelete(response);
	return 0;
}



/************************************************************************/
int
printer_set_ppd(http_t *http, char* lp_name, char* ppd_file)
{
	ipp_t *request;             /* IPP Request */
	ipp_t *response;            /* IPP Response */
	char uri[HTTP_MAX_URI];     /* URI for printer/class */
	char tempfile[1024];        /* Temporary filename */
	int fd;                     /* Temporary file */
	gzFile *gz;                 /* GZIP'd file */
	char buffer[8192];          /* Copy buffer */
	int bytes;                  /* Bytes in buffer */
	int size;


	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_set_ppd]: "
			"set ppd file %s",ppd_file);

	/*
	 * See if the file is gzip'd; if so, unzip it to a temporary file and
	 * send the uncompressed file.
	 */

	if (!strcmp(ppd_file + strlen(ppd_file) - 3, ".gz"))
	{
		/*
		 * Yes, the file is compressed; uncompress to a temp file...
		 */

		if ((fd = cupsTempFd(tempfile, sizeof(tempfile))) < 0)
		{
			log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_set_ppd]: "
					"Unable to create temporary file");
			return 1;
		}

		if ((gz = (gzFile*)gzopen(ppd_file, "rb")) == NULL)
		{
			log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_set_ppd]: "
					"Unable to open file \"%s\": %s",ppd_file, strerror(errno));
			close(fd);
			unlink(tempfile);
			return (1);
		}

		while ((bytes = gzread(gz, buffer, sizeof(buffer))) > 0)
		{
			size = write(fd, buffer, bytes);
		}

		close(fd);
		gzclose(gz);

		ppd_file = tempfile;
	}
	/*
	 * Build a CUPS_ADD_PRINTER request, which requires the following
	 * attributes:
	 *
	 *    attributes-charset
	 *    attributes-natural-language
	 *    printer-uri
	 */

	request = ippNewRequest(CUPS_ADD_PRINTER);

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
					"localhost", 0, "/printers/%s", lp_name);
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
					"printer-uri", NULL, uri);

	/*
	 * Do the request and get back a response...
	 */

	response = cupsDoFileRequest(http, request, "/admin/", ppd_file);
	ippDelete(response);

	/*
	 * Remove the temporary file as needed...
	 */

	if (ppd_file == tempfile)
	{
		unlink(tempfile);
	}

	if (cupsLastError() > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_set_ppd]: "
				"%s", cupsLastErrorString());

		return 1;
	}
	return 0;
}

/************************************************************************/
int
printer_do_operation(http_t * http, int operation, char* lp_name)
{
	ipp_t *request;              /* IPP Request */
	char uri[HTTP_MAX_URI];      /* URI for printer/class */
	char* reason = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_do_operation]: "
				"do operation (%i => %p, \"%s\")", operation, http, lp_name);
	request = ippNewRequest(operation);

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
				"localhost", 0, "/printers/%s", lp_name);
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
				"printer-uri", NULL, uri);

	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
				"requesting-user-name", NULL, cupsUser());


	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
				"printer-state-message", NULL, reason);

	ippDelete(cupsDoRequest(http, request, "/admin/"));

	if (cupsLastError() > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_do_operation]: "
				"operation failed: %s", ippErrorString(cupsLastError()));
		return 1;
	}

	request = ippNewRequest(IPP_PURGE_JOBS);

	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
				"printer-uri", NULL, uri);

	ippDelete(cupsDoRequest(http, request, "/admin/"));

	if (cupsLastError() > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_do_operation]: "
				"%s", cupsLastErrorString());
		return 1;
	}
	return 0;
}

/************************************************************************/
static struct list*
printer_remove_user_from_list(struct list* user_list, const char* user_name)
{
	struct list* new_user_list = NULL;
	char* current_user;
	int i = 0;

	new_user_list = list_create();
	new_user_list->auto_free = 1;

	if (user_list == NULL)
	{
		return new_user_list;
	}

	for (i = 0 ; i < user_list->count ; i++)
	{
		current_user = (char*)list_get_item(user_list, i);
		if (current_user && g_strncmp(current_user, user_name, g_strlen(user_name)) != 0)
		{
			list_add_item(new_user_list, (long)g_strdup(current_user));
		}
	}
	return new_user_list;
}

/************************************************************************/
static int
printer_contain_user(struct list* user_list, const char* user_name)
{
	char* current_user;
	int i = 0;

	for (i = 0 ; i < user_list->count ; i++)
	{
		current_user = (char*)list_get_item(user_list, i);
		if (current_user && g_strncmp(current_user, user_name, g_strlen(user_name)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/************************************************************************/
struct list*
printer_get_restricted_user_list(http_t* http, char* printer_name)
{
	ipp_t	*request;		/* IPP Request */
	ipp_t	*response;
	char uri[HTTP_MAX_URI];	/* URI for printer/class */
	ipp_attribute_t *attr;
	int i=0;
	struct list* user_list = NULL;
	char* current_user = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_get_restricted_user_list]: "
				"Get user list for the printer '%s'", printer_name);

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL, "localhost", 0, "/printers/%s", printer_name);
	request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
	response = cupsDoRequest(http, request, "/admin/");
	if (cupsLastError() > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_get_restricted_user_list]: "
					"Operation failed: %s", ippErrorString(cupsLastError()));
		goto failed;
	}
	attr = ippFindAttribute(response, "requesting-user-name-allowed", IPP_TAG_NAME);
	if (attr == 0 || attr->num_values == 0)
	{
		goto failed;
	}
	user_list = list_create();
	user_list->auto_free = 1;

	for(i=0 ; i < attr->num_values ; i++)
	{

		if (g_strlen(attr->values[i].string.text) > 0)
		{
			list_add_item(user_list, (long)g_strdup(attr->values[i].string.text));
		}
	}

	ippDelete(response);
	return user_list;

failed:
	ippDelete(response);
	g_free(user_list);
	return NULL;

}



/************************************************************************/
int
printer_set_restrict_user_list(http_t* http, char* printer_name, struct list* user_list)
{
	int num_options = 0;
	cups_option_t	*options;
	ipp_t	*request;		/* IPP Request */
	char uri[HTTP_MAX_URI];	/* URI for printer/class */
	char user_list_str[2048] = {0};
	char* current_user = NULL;
	int i = 0;


	for (i = 0 ; i < user_list->count ; i++)
	{
		current_user = (char*)list_get_item(user_list, i);
		if (current_user && g_strcmp(current_user, "") !=0)
		{
			g_strcat(user_list_str, current_user);
			g_strcat(user_list_str, ",");
		}
	}
	/*
	* Add the options...
	*/

	httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL, "localhost", 0, "/printers/%s", printer_name);

	request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);

	// Configure user allowed to print, the other can not print
	num_options = cupsAddOption("requesting-user-name-allowed",
                          user_list_str, num_options,&options);
	cupsEncodeOptions2(request, num_options, options, IPP_TAG_PRINTER);
	ippDelete(cupsDoRequest(http, request, "/admin/"));

	if (cupsLastError() > IPP_OK_CONFLICT)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_set_restrict_user_list]: "
						"Error while processing cups request %s", cupsLastErrorString());
		return 1;
	}
	return 0;
}





/************************************************************************/
int APP_CC
printer_add(const char* username, const char* printer_name)
{
	http_t *http = NULL;
	struct list* user_list = NULL;
	char* cups_printer_name = NULL;

	cups_printer_name = printer_convert_name(printer_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_add]: "
				"Try to connect to cups server");
	if (printer_server_connect(&http) == 1)
	{
	  log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_add]: "
				"Unable to connect to printer server");
		return 1;
	}
	if( printer_add_printer(http, cups_printer_name) !=0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_add]: "
					"Failed to add printer");
		goto error_disconnect;
	}
	if(printer_set_ppd(http, cups_printer_name, PPD_FILE) !=0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "printerd[printer_add]: "
					"Failed to set ppd file");
		goto error_disconnect;
	}
	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_add]: "
				"Succedd to add printer: %s", printer_name);

	printer_do_operation(http, IPP_RESUME_PRINTER, cups_printer_name);
	printer_do_operation(http, CUPS_ACCEPT_JOBS, cups_printer_name);

	user_list = printer_get_restricted_user_list(http, cups_printer_name);
	if( user_list == NULL)
	{
		user_list = list_create();
		user_list->auto_free = 1;
	}
	list_add_item(user_list, (long)g_strdup(username));
	printer_set_restrict_user_list(http, cups_printer_name, user_list);


	printer_server_disconnect(http);
	g_free(cups_printer_name);
	list_delete(user_list);
	return 0;

error_disconnect:
	printer_server_disconnect(http);
	g_free(cups_printer_name);
	list_delete(user_list);
	return 1;
}

/************************************************************************/
int APP_CC
printer_del(const char* username, const char* printer_name)
{
	struct list* user_list = NULL;
	struct list* new_user_list = NULL;
	char* cups_printer_name = NULL;
	http_t *http = NULL;

	cups_printer_name = printer_convert_name(printer_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_del]: "
			"Printer to remove : %s", cups_printer_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_del]: "
				"Try to connect to cups server");
	if (printer_server_connect(&http) == 1)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_del]: "
				"Unable to connect to printer server\n");
		goto fail;
	}

	user_list = printer_get_restricted_user_list(http, cups_printer_name);
	new_user_list = printer_remove_user_from_list(user_list, username);

	if( new_user_list->count == 0 )
	{
		if (printer_del_printer(http, cups_printer_name) == 1 )
		{
			log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_del]: "
					"Unable to delete printer %s", cups_printer_name);
			goto fail;
		}
	}
	else
	{
		if( new_user_list->count == user_list->count )
		{
			goto fail;
		}
		if ( printer_set_restrict_user_list(http, cups_printer_name, new_user_list) == 1 )
		{
			log_message(l_config, LOG_LEVEL_WARNING, "printerd[printer_del]: "
					"Unable to restrict user %s on printer %s", username, cups_printer_name);
			goto fail;
		}
	}

	g_free(cups_printer_name);
	list_delete(user_list);
	list_delete(new_user_list);
	printer_server_disconnect(http);
	return 0;

fail:
	g_free(cups_printer_name);
	list_delete(user_list);
	list_delete(new_user_list);
	printer_server_disconnect(http);
	return 1;
}

/************************************************************************/
int APP_CC
printer_purge(const char* username)
{
	http_t *http = NULL;
	struct list* printer_list = NULL;
	char* current_printer = NULL;
	int i=0;

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_purge]: ");

	if (printer_server_connect(&http) == 1)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_purge]: "
				"Unable to connect to printer server");
		return 1;
	}

	printer_list = printer_get_printer_list(http);
	if (printer_list == NULL)
	{
		printer_server_disconnect(http);
		return 0;
	}

	for(i = 0 ; i < printer_list->count ; i++)
	{
		current_printer = (char*)list_get_item(printer_list, i);
		if (current_printer != NULL)
		{
			printer_del(username, current_printer);
		}
	}
	printer_server_disconnect(http);
	list_delete(printer_list);
	return 0;
}

/************************************************************************/
int APP_CC
printer_purge_all()
{
	http_t *http = NULL;
	struct list* printer_list = NULL;
	char* current_printer = NULL;
	int i=0;

	log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_purge_all]: ");

	if (printer_server_connect(&http) == 1)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "printerd[printer_purge_all]: "
				"Unable to connect to printer server");
		return 1;
	}

	printer_list = printer_get_printer_list(http);
	if (printer_list == NULL)
	{
		printer_server_disconnect(http);
		return 0;
	}

	for(i = 0 ; i < printer_list->count ; i++)
	{
		current_printer = (char*)list_get_item(printer_list, i);
		if (current_printer != NULL)
		{
			printer_del_printer(http, current_printer);
		}
	}

	printer_server_disconnect(http);
	list_delete(printer_list);
	return 0;
}




