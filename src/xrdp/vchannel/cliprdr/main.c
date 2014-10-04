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
#include "cliprdr.h"
#include "Clipboard.h"
#include "xutils.h"
#include "uni_rdp.h"

/* external function declaration */
extern char** environ;

static Display* display;
static int cliprdr_channel;
struct log_config* l_config;
static Window wclip;
static int running;
pthread_cond_t reply_cond;
pthread_mutex_t mutex;

static int is_fragmented_packet = 0;
static int fragment_size;
static struct stream* splitted_packet;

/* Atoms of the two X selections we're dealing with: CLIPBOARD (explicit-copy) and PRIMARY (selection-copy) */
static Atom clipboard_atom;
static Atom primary_atom;
static Atom targets_atom;
static Atom timestamp_atom;
static Atom format_string_atom;
static Atom format_utf8_string_atom;
static Atom format_unicode_atom;
static Atom format_file_gnome_atom;
static Atom format_file_text_uri_list_atom;
static Atom xrdp_clipboard;
Clipboard clipboard;

/*****************************************************************************/
void APP_CC
cliprdr_wait_reply()
{

  if (pthread_cond_wait(&reply_cond, &mutex) != 0) {
    perror("pthread_cond_timedwait() error");
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_cliprdr[cliprdr_wait_reply]: "
				"pthread_mutex_lock()");
    return;
  }
}

/*****************************************************************************/
int
error_handler(Display* display, XErrorEvent* error)
{
  char text[256];
  XGetErrorText(display, error->error_code, text, 255);
  log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[error_handler]: "
  		" Error [%s]", text);
  return 0;
}

/*****************************************************************************/
int APP_CC
cliprdr_send(struct stream* s){
  int rv;
  int length;

	length = (int)(s->end - s->data);

  rv = vchannel_send(cliprdr_channel, s->data, length);
  if (rv != 0)
  {
    log_message(l_config, LOG_LEVEL_ERROR, "vchannel_cliprdr[cliprdr_send]: "
    		"Unable to send message");
  }
  return rv;
}

/*****************************************************************************/
void
handler(int sig)
{
	int pid, statut;
	pid = waitpid(-1, &statut, 0);
	log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[handler]: "
  		"A processus has ended");
  return;
}

/*****************************************************************************/
void cliprdr_send_capability()
{
	/* this message is ignored by rdp applet */
	struct stream* s;

	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_capability]:");
	/* clip header */
	out_uint16_le(s, CB_CLIP_CAPS);                        /* msg type */
	out_uint16_le(s, 0x00);                                /* msg flag */
	out_uint32_le(s, 16);                                  /* msg size */
	/* we only support one capability for now */
	out_uint16_le(s, 1);                                   /* cCapabilitiesSets */
	out_uint8s(s, 2);                                      /* pad */
	/* CLIPRDR_CAPS_SET */
	out_uint16_le(s, CB_CAPSTYPE_GENERAL);                 /* capabilitySetType */
	out_uint16_le(s, 12);                                  /* lengthCapability */
	out_uint32_le(s, CB_CAPS_VERSION_1);                   /* version */
	out_uint32_le(s, 0);                                   /* general flags */


	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}

/*****************************************************************************/
void cliprdr_send_ready()
{
	struct stream* s;

	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_ready]:");
	/* clip header */
	out_uint16_le(s, CB_MONITOR_READY);                    /* msg type */
	out_uint16_le(s, 0x00);                                /* msg flag */
	out_uint32_le(s, 0);                                   /* msg size */

	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}

/*****************************************************************************/
void cliprdr_send_format_list()
{
	struct stream* s;

	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_format_list_response]:");
	/* clip header */
	out_uint16_le(s, CB_FORMAT_LIST);                      /* msg type */
	out_uint16_le(s, 0);                                   /* msg flag */
	out_uint32_le(s, 72);                                  /* msg size */
	/* we only send one format */
	out_uint32_le(s, CF_TEXT);                             /* Format Id */
	out_uint8s(s, 32);
	out_uint32_le(s, CF_UNICODETEXT);                      /* Format Id */
	out_uint8s(s, 32);

	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}

/*****************************************************************************/
void cliprdr_send_format_list_response()
{
	struct stream* s;

	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_format_list_response]:");
	/* clip header */
	out_uint16_le(s, CB_FORMAT_LIST_RESPONSE);             /* msg type */
	out_uint16_le(s, CB_RESPONSE_OK);                      /* msg flag */
	out_uint32_le(s, 0);                                   /* msg size */

	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}

/*****************************************************************************/
void cliprdr_send_data_request()
{
	struct stream* s;

	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_data_request]:");
	/* clip header */
	out_uint16_le(s, CB_FORMAT_DATA_REQUEST);              /* msg type */
	out_uint16_le(s, 0);                                   /* msg flag */
	out_uint32_le(s, 1);                                   /* msg size */
	out_uint32_le(s, CF_UNICODETEXT);                      /* we want CF_UNICODE */

	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}

/*****************************************************************************/
void cliprdr_send_data(int request_type)
{
	struct stream* s;
	int clipboard_size = clipboard_get_current_clipboard_data_size(&clipboard, format_utf8_string_atom);
	char* clipboard_data = (char*)clipboard_get_current_clipboard_data(&clipboard, format_utf8_string_atom);

	int uni_clipboard_len = (clipboard_size+1)*2;
	int packet_len = uni_clipboard_len + 12;
	char* temp;

	make_stream(s);
	init_stream(s,packet_len);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_send_data]:");
	/* clip header */
	out_uint16_le(s, CB_FORMAT_DATA_RESPONSE);             /* msg type */
	out_uint16_le(s, 0);                                   /* msg flag */
	out_uint32_le(s, uni_clipboard_len);                   /* msg size */
	temp = s->p;
	uni_rdp_out_str(s, clipboard_data, uni_clipboard_len);


	s_mark_end(s);
	cliprdr_send(s);
	free_stream(s);

}



/*****************************************************************************/
int cliprdr_process_format_list(struct stream* s, int msg_flags, int size)
{
	int format_number;

	/* long format announce */
	if (msg_flags == CB_ASCII_NAMES)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_cliprdr[cliprdr_process_format_list]: "
				"Long format list is not yet supported");
		return 1;
	}

	/* short format announce */
	format_number = size / 36;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_format_list]: "
			"%i formats announced", format_number);

	s->p = s->end;

	cliprdr_send_format_list_response();
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_format_list]: "
			"get the clipboard");

	// We only support unicode content
	clipboard_current_clipboard_clear(&clipboard);
	clipboard_add_current_clipboard_format(&clipboard, format_utf8_string_atom);

	cliprdr_send_data_request();

	XSetSelectionOwner(display, clipboard_atom, wclip, CurrentTime);
	if (XGetSelectionOwner(display, clipboard_atom) != wclip)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_cliprdr[cliprdr_process_format_list]: "
				"Unable to set clipboard owner");
	}
	return 0;
}


/*****************************************************************************/
int cliprdr_process_data_request(struct stream* s, int msg_flags, int size)
{
	int request_type = 0;
	char* clipboard_data = (char*)clipboard_get_current_clipboard_data(&clipboard, format_utf8_string_atom);
	if (clipboard_data == 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_cliprdr[cliprdr_process_data_request]: "
				"No clipboard data");
		return 1;
	}

	in_uint32_le(s, request_type);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_data_request]: "
			"Request for data of type %i", request_type);
	cliprdr_send_data(request_type);
	return 0;
}

/*****************************************************************************/
int cliprdr_process_data_request_response(struct stream* s, int msg_flags, int size)
{
	int clipboard_size = 0;
	unsigned char* clipboard_data = NULL;

	if (msg_flags == CB_RESPONSE_FAIL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_cliprdr[cliprdr_process_data_request_response]: "
				"Unable to get clipboard data");
		return 1;
	}
	log_hexdump(l_config, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)s->p, size);
	if (XGetSelectionOwner(display, clipboard_atom) != wclip)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_cliprdr[cliprdr_process_data_request_response]: "
				"Xrpd is not the owner of the selection");
		return 1;
	}
	if (clipboard_data != 0)
	{
		g_free(clipboard_data);
	}

	/* An UTF-16 String form RDP is always on 16bits. An UTF-8 string can be coded either
	 * on 8, 16, 24 or 32 bits. So it can be AT MOST input size*2
	 */

	clipboard_data = g_malloc(size*2, 1);
	clipboard_size = uni_rdp_in_str(s, (char*)clipboard_data, size*2, size);

	// Remove NULL characters from clipboard data end, it is not a null terminated string
	if (clipboard_data)
	{
		while (clipboard_size > 0 && clipboard_data[clipboard_size-1] == '\0')
		{
			clipboard_size--;
		}
	}

	clipboard_add_current_clipboard_data(&clipboard, clipboard_data, clipboard_size, format_utf8_string_atom);

	return 0;
}


/*****************************************************************************/
void cliprdr_process_message(struct stream* packet, int length, int total_length) {
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
			"New message for clipchannel");
	int msg_type;
	int msg_flags;
	int msg_size;

	struct stream* s;

	if(length != total_length)
	{
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_cliprdr[cliprdr_process_message]: "
			"Packet is fragmented");
		if(is_fragmented_packet == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_cliprdr[cliprdr_process_message]: "
				"Packet is fragmented : first part");
			is_fragmented_packet = 1;
			fragment_size = length;
			make_stream(splitted_packet);
			init_stream(splitted_packet, total_length);
			g_memcpy(splitted_packet->p, packet->p, length);
			return;
		}
		else
		{
			g_memcpy(splitted_packet->p+fragment_size, packet->p, length);
			fragment_size += length;
			if (fragment_size == total_length)
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_message]: "
					"Packet is fragmented : last part");
				s = splitted_packet;
			}
			else
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_message]: "
					"Packet is fragmented : next part");
				return;
			}
		}
	}
	else
	{
		s = packet;
	}

	in_uint16_le(s, msg_type);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
			"Message type : %04x", msg_type);
	in_uint16_le(s, msg_flags);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
			"Message flags : %04x", msg_flags);
	in_uint32_le(s, msg_size);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
			"Message size : %i", msg_size);

	switch (msg_type)
	{
	case CB_FORMAT_LIST :
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
				"Client format list announce");
		cliprdr_process_format_list(s, msg_flags, msg_size);
		//cliprdr_send_format_list();
		break;

	case CB_FORMAT_DATA_RESPONSE :
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
				"Client data request response");
		cliprdr_process_data_request_response(s, msg_flags, msg_size);
		pthread_cond_signal(&reply_cond);

		break;
	case CB_FORMAT_LIST_RESPONSE :
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
				"Client format list response");
		break;

	case CB_FORMAT_DATA_REQUEST :
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
				"Client data request");
		cliprdr_process_data_request(s, msg_flags, msg_size);
		break;

	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[process_message]: "
				"Unknow message type : %i", msg_type);

	}
	if(is_fragmented_packet == 1)
	{
		is_fragmented_packet = 0;
		fragment_size = 0;
		free_stream(s);
	}
}

/*****************************************************************************/
static void
cliprdr_clear_selection(XEvent* e)
{
	Window newOwner = XGetSelectionOwner(e->xselectionclear.display, clipboard_atom);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[clear_selection]: "
			"New windows owner : %i", (int)newOwner);

	clipboard_current_clipboard_clear(&clipboard);
	XConvertSelection(display, clipboard_atom, targets_atom, xrdp_clipboard, wclip, CurrentTime);
	XSync (e->xselectionclear.display, False);
}

/*****************************************************************************/
static void
cliprdr_get_clipboard(XEvent* e)
{
	Atom type;
	unsigned long len, bytes_left, dummy;
	int format, result;
	unsigned char *data;


	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
			"New owner %i", e->xselection.requestor);

	XGetWindowProperty (e->xselection.display,
											e->xselection.requestor,
											e->xselection.property,
											0, 0,
											False,
											AnyPropertyType,
											&type,
											&format,
											&len, &bytes_left,
											&data);
	// Check Format list
	if (type == XA_ATOM || format == 32)
	{
		result = XGetWindowProperty (e->xselection.display, e->xselection.requestor, e->xselection.property, 0, bytes_left, 0, XA_ATOM, &type, &format, &len, &dummy, &data);
		if (result == Success)
		{
			int i = 0;
			Atom atom;
			for (i = 0; i < len ; i++)
			{
				atom = ((Atom*)data)[i];
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
						"format : %s", XGetAtomName(display, atom));
				if (clipboard_format_supported(&clipboard, atom))
				{
					clipboard_add_current_clipboard_format(&clipboard, atom);
				}
			}
			if (clipboard_current_clipboard_size(&clipboard) > 0)
			{
				atom = clipboard_get_current_clipboard_format(&clipboard, 0);
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
						"Request for format %s", XGetAtomName(display, atom));

				XConvertSelection(display, clipboard_atom, atom, xrdp_clipboard, wclip, CurrentTime);
				XSync (e->xselectionclear.display, False);
			}
			return;
		}

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
				"Failed to parse atom list");
		return;
	}

	// DATA is There
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
			"Data type : %s\n", XGetAtomName(e->xselection.display, e->xselection.property));

	if (bytes_left > 0 && clipboard_format_supported(&clipboard, e->xselection.target))
	{
		unsigned char* clipboard_data = NULL;
		int clipboard_size = 0;

		result = XGetWindowProperty(e->xselection.display, e->xselection.requestor, e->xselection.property, 0, bytes_left, 0, e->xselection.target, &type, &format, &len, &dummy, &data);
		if (result == Success)
		{
			log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_cliprdr[cliprdr_get_clipboard]: "
					"New data in clipboard: %s", data);
			int index = -1;
			index = clipboard_get_current_clipboard_format_index(&clipboard, e->xselection.target);

			// Don't forget the null terminated character
			clipboard_data = g_malloc(bytes_left + 1, 1);
			clipboard_size = bytes_left;
			g_memcpy(clipboard_data, data, bytes_left);
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
					"clipboard %s[%i] updated with '%s'", XGetAtomName(display, e->xselection.target), index, data);

			clipboard_add_current_clipboard_data(&clipboard, clipboard_data, clipboard_size, e->xselection.target);

			if (index < (clipboard_current_clipboard_size(&clipboard) - 1))
			{
				Atom format = clipboard_get_current_clipboard_format(&clipboard, index + 1);
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
						"Request for format %s", XGetAtomName(display, format));

				XConvertSelection(display, clipboard_atom, format, xrdp_clipboard, wclip, CurrentTime);
				XSync (e->xselectionclear.display, False);
			}
			else
			{
				XSetSelectionOwner(display, clipboard_atom, wclip, CurrentTime);
				XSync(display, False);
				// File content is not supported for now
				if ((! clipboard_current_clipboard_format_exist(&clipboard, format_file_gnome_atom)) && (! clipboard_current_clipboard_format_exist(&clipboard, format_file_text_uri_list_atom)))
					cliprdr_send_format_list();
			}
		}
		else
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_get_clipboard]: "
					"Failed to get clipboard content");
		}
		XFree (data);
	}
}

/*****************************************************************************/
static void
cliprdr_process_selection_request(XEvent* e)
{
	XSelectionRequestEvent *req;
	XEvent respond;
	int clipboard_size;
	unsigned char* clipboard_data = NULL;

	req=&(e->xselectionrequest);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_selection_request]: "
			"Request for %s\n", XGetAtomName(display, req->target));

	if (clipboard_format_supported(&clipboard, req->target))
	{
		if (!clipboard_current_clipboard_format_exist(&clipboard, req->target))
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_selection_request]: "
					"Unable to find format %s", XGetAtomName(display, req->target));
			return;
		}

		clipboard_data = clipboard_get_current_clipboard_data(&clipboard, req->target);
		clipboard_size = clipboard_get_current_clipboard_data_size(&clipboard, req->target);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_selection_request]: "
				"Update data with '%s'", clipboard_data);

		XChangeProperty (req->display,
			req->requestor,
			req->property,
			req->target,
			8,
			PropModeReplace,
			(unsigned char*) clipboard_data,
			clipboard_size);
		respond.xselection.property=req->property;
	}
	else if (req->target == targets_atom)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[cliprdr_process_selection_request]: "
				"Targets : '%s'", XGetAtomName(req->display, req->property));
		XChangeProperty (req->display, req->requestor, req->property, XA_ATOM, 32, PropModeReplace, (unsigned char*)clipboard.current_clipboard_formats->items, clipboard.current_clipboard_formats->count);
		respond.xselection.property=req->property;
	}
	else // Strings only please
	{
		printf ("No String %i\n",
			(int)req->target);
		respond.xselection.property= None;
	}
	respond.xselection.type= SelectionNotify;
	respond.xselection.display= req->display;
	respond.xselection.requestor= req->requestor;
	respond.xselection.selection=req->selection;
	respond.xselection.target= req->target;
	respond.xselection.time = req->time;
	XSendEvent (req->display, req->requestor,0,0,&respond);
	XFlush (req->display);
}



/*****************************************************************************/
void *thread_Xvent_process (void * arg)
{
	Window root_windows;
	XEvent ev;

  primary_atom = XInternAtom(display, "PRIMARY", False);
	clipboard_atom = XInternAtom(display, "CLIPBOARD", False);
	targets_atom = XInternAtom(display, "TARGETS", False);
	timestamp_atom = XInternAtom(display, "TIMESTAMP", False);
	format_string_atom = XInternAtom(display, "STRING", False);
	format_utf8_string_atom = XInternAtom(display, "UTF8_STRING", False);
	format_unicode_atom = XInternAtom(display, "text/unicode", False);
	format_file_gnome_atom = XInternAtom(display, "x-special/gnome-copied-files", False);
	format_file_text_uri_list_atom = XInternAtom(display, "text/uri-list", False);
	xrdp_clipboard = XInternAtom(display,"XRDP_CLIPBOARD", False);

	clipboard_init(&clipboard, 3);
	clipboard_add_format_support(&clipboard, format_utf8_string_atom);
	clipboard_add_format_support(&clipboard, format_file_gnome_atom);
	clipboard_add_format_support(&clipboard, format_file_text_uri_list_atom);

	root_windows = DefaultRootWindow(display);
	log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
				"Windows root ID : %i", (int)root_windows);
	wclip = XCreateSimpleWindow(display, root_windows,1,1,1,1,1,1,0);

	XSelectInput(display, wclip, PropertyChangeMask);
	log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
				"Begin the event loop ");

	while (running) {
		XNextEvent (display, &ev);
		switch (ev.type)
		{

		case SelectionClear :
			log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
						"XSelectionClearEvent");
			cliprdr_clear_selection(&ev);
			break;

		case SelectionRequest :
			log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
						"XSelectionRequestEvent");
			cliprdr_process_selection_request(&ev);
			break;

		case SelectionNotify :
			log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
						"SelectionNotify");
			cliprdr_get_clipboard(&ev);
			break;

		case ClientMessage:
			break;

		default:
			log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[thread_Xvent_process]: "
						"event type :  %i", ev.type);
			break;
		}
	}

	clipboard_release(&clipboard);

	pthread_exit (0);
}


void send_dummy_event() {
	XClientMessageEvent dummy_event;

	dummy_event.type = ClientMessage;
	dummy_event.window = wclip;
	dummy_event.format = 32;
	XSendEvent(display, wclip, 0, 0, (XEvent*)&dummy_event);

	XFlush(display);
}

/*****************************************************************************/
void *thread_vchannel_process (void * arg)
{
	struct stream* s = NULL;
	int rv;
	int length;
	int total_length;

	signal(SIGCHLD, handler);
	cliprdr_send_ready();
	cliprdr_send_capability();

	while(running){
		make_stream(s);
		init_stream(s, 1600);

		rv = vchannel_receive(cliprdr_channel, s->data, &length, &total_length);
		if( rv == ERROR )
		{
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_cliprdr[thread_vchannel_process]: "
					"Invalid message");
			vchannel_close(cliprdr_channel);
			pthread_exit ((void*)1);
		}
		switch(rv)
		{
		case ERROR:
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_cliprdr[thread_vchannel_process]: "
					"Invalid message");
			break;
		case STATUS_CONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[thread_vchannel_process]: "
					"Status connected");
			break;
		case STATUS_DISCONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_cliprdr[thread_vchannel_process]: "
					"Status disconnected");
			running = 0;

			// Send a dummy event in order to unblock XNextEvent function
			send_dummy_event();

			break;
		default:
			if (length == 0)
			{
				running = false;
				send_dummy_event();
				pthread_exit (0);
			}

			cliprdr_process_message(s, length, total_length);
			break;
		}
		free_stream(s);
	}
	pthread_exit (0);
}

/*****************************************************************************/
int
cliprdr_init()
{
  char filename[256];
  struct list* names;
  struct list* values;
  char* name;
  char* value;
  int index;
  int display_num;

  display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	if(display_num == 0)
	{
		g_printf("cliprdr[cliprdr_init]: Display must be different of 0\n");
		return ERROR;
	}
	l_config = g_malloc(sizeof(struct log_config), 1);
	l_config->program_name = "cliprdr";
	l_config->log_file = 0;
	l_config->fd = 0;
	l_config->log_level = LOG_LEVEL_DEBUG;
	l_config->enable_syslog = 0;
	l_config->syslog_level = LOG_LEVEL_DEBUG;

  names = list_create();
  names->auto_free = 1;
  values = list_create();
  values->auto_free = 1;
  g_snprintf(filename, 255, "%s/cliprdr.conf", XRDP_CFG_PATH);
  if (file_by_name_read_section(filename, CLIPRDR_CFG_GLOBAL, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, CLIPRDR_CFG_NAME))
      {
        if( g_strlen(value) > 1)
        {
        	l_config->program_name = (char*)g_strdup(value);
        }
      }
    }
  }
  if (file_by_name_read_section(filename, CLIPRDR_CFG_LOGGING, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, CLIPRDR_CFG_LOG_LEVEL))
      {
      	l_config->log_level = log_text2level(value);
      }
    }
  }
  list_delete(names);
  list_delete(values);

	if(log_start(l_config) != LOG_STARTUP_OK)
	{
		g_printf("vchannel[vchannel_init]: Unable to start log system\n");
		return ERROR;
	}
  else
  {
  	return LOG_STARTUP_OK;
  }
  return 0;
}

/*****************************************************************************/
int main(int argc, char** argv, char** environ)
{
	pthread_t Xevent_thread, Vchannel_thread;
	void *ret;
	l_config = g_malloc(sizeof(struct log_config), 1);
	if (cliprdr_init() != LOG_STARTUP_OK)
	{
		g_printf("cliprdr[main]: Unable to init log system\n");
		g_free(l_config);
		return 1;
	}
	if (vchannel_init() == ERROR)
	{
		g_printf("cliprdr[main]: Unable to init channel system\n");
		g_free(l_config);
		return 1;
	}

	pthread_cond_init(&reply_cond, NULL);
	pthread_mutex_init(&mutex, NULL);

	cliprdr_channel = vchannel_open("cliprdr");
	if( cliprdr_channel == ERROR)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "cliprdr[main]: "
				"Error while connecting to vchannel provider");
		g_free(l_config);
		return 1;
	}

	XInitThreads();
	log_message(l_config, LOG_LEVEL_DEBUG, "cliprdr[main]: "
			"Opening the default display : %s",getenv("DISPLAY"));

	if ((display = XOpenDisplay(0))== 0){
		log_message(l_config, LOG_LEVEL_ERROR, "cliprdr[main]: "
				"Unable to open the default display : %s ",getenv("DISPLAY"));
		g_free(l_config);
		return 1;
	}
	XSynchronize(display, 1);
	XSetErrorHandler(error_handler);

	running = 1;

	if (pthread_create (&Xevent_thread, NULL, thread_Xvent_process, (void*)0) < 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "cliprdr[main]: "
				"Pthread_create error for thread : Xevent_thread");
		g_free(l_config);
		return 1;
	}
	if (pthread_create (&Vchannel_thread, NULL, thread_vchannel_process, (void*)0) < 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "cliprdr[main]: "
				"Pthread_create error for thread : Vchannel_thread");
		g_free(l_config);
		return 1;
	}

	(void)pthread_join (Xevent_thread, &ret);
	//(void)pthread_join (Vchannel_thread, &ret);
	pthread_mutex_destroy(&mutex);
	XCloseDisplay(display);
	g_free(l_config);
	return 0;
}
