/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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
 */

#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/utils/event.h>
#include <freerdp/plugins/cliprdr.h>
#include <freerdp/utils/unicode.h>
#include <freerdp/utils/memory.h>
#include <freerdp/plugins/cliprdr.h>

#include "urdp.h"
#include "urdp_cliprdr.h"

void urdp_cliprdr_init(urdp_context* context) {
	/* Load clipboard plugin */
	int ret = freerdp_channels_load_plugin(context->context.channels, context->context.instance->settings, "cliprdr", NULL);
	if (ret != 0) {
		log_error("failed loading cliprdr plugin");
		return;
	}

	context->clipboard.num_formats = 2;
	context->clipboard.formats = (uint32*) xmalloc(sizeof(uint32) * context->clipboard.num_formats);
	context->clipboard.formats[0] = CB_FORMAT_UNICODETEXT;
	context->clipboard.formats[1] = CB_FORMAT_TEXT;
}

void urdp_send_format_list(urdp_context* context) {
	RDP_CB_FORMAT_LIST_EVENT* format_list = (RDP_CB_FORMAT_LIST_EVENT*) freerdp_event_new(RDP_EVENT_CLASS_CLIPRDR, RDP_EVENT_TYPE_CB_FORMAT_LIST, NULL, NULL);
	format_list->num_formats = context->clipboard.num_formats;
	format_list->formats = (uint32*) xmalloc(sizeof(uint32) * format_list->num_formats);
	memcpy(format_list->formats, context->clipboard.formats, sizeof(uint32) * format_list->num_formats);
	freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) format_list);
}

void urdp_process_cb_monitor_ready(urdp_context* context, RDP_EVENT* event) {
	/* Received notification of clipboard support. */
	RDP_CB_DATA_REQUEST_EVENT* request;
	
	request = (RDP_CB_DATA_REQUEST_EVENT*) freerdp_event_new(RDP_EVENT_CLASS_CLIPRDR, RDP_EVENT_TYPE_CB_DATA_REQUEST, NULL, NULL);
	request->format = CB_FORMAT_UNICODETEXT;
	freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) request);	
	
	urdp_send_format_list(context);
}

void urdp_process_cb_format_list(urdp_context* context, RDP_CB_FORMAT_LIST_EVENT* event) {
	/* Received notification of available data */
	int i, j;
	for (j = 0; j < context->clipboard.num_formats; j++) {
		for (i = 0; i < event->num_formats; i++) {
			/* If format available, request it */
			log_debug("check format=%x == %x", event->formats[i], context->clipboard.formats[j]);

			if (event->formats[i] == context->clipboard.formats[j]) {
				RDP_CB_DATA_REQUEST_EVENT* data_request = (RDP_CB_DATA_REQUEST_EVENT*) freerdp_event_new(RDP_EVENT_CLASS_CLIPRDR, RDP_EVENT_TYPE_CB_DATA_REQUEST, NULL, NULL);
				data_request->format = event->formats[i];
				context->clipboard.data_format = data_request->format;
				freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) data_request);
				log_debug("request clipboard format : %x", data_request->format);
				return;
			}
		}
	}
	log_debug("Ignoring unsupported clipboard data");
	context->clipboard.data_format = -1;
}

void urdp_process_cb_data_request(urdp_context* context, RDP_CB_DATA_REQUEST_EVENT* event) {
	/* If text requested, send clipboard text contents */
	UNICONV* uniconv;
	RDP_CB_DATA_RESPONSE_EVENT* response;
	size_t out_size;

	response = (RDP_CB_DATA_RESPONSE_EVENT*) freerdp_event_new(RDP_EVENT_CLASS_CLIPRDR, RDP_EVENT_TYPE_CB_DATA_RESPONSE, NULL, NULL);
	response->data = NULL;
	response->size = 0;
	
	if (context->clipboard.data != NULL) {
		switch (event->format) {
		case CB_FORMAT_UNICODETEXT:
			uniconv = freerdp_uniconv_new();
			response->data = (uint8*)freerdp_uniconv_out(uniconv, (char*)context->clipboard.data, &out_size);
			response->size = out_size + 2;
			freerdp_uniconv_free(uniconv);
			log_debug("clipboard (unicode) %s", context->clipboard.data);
			break;
		case CB_FORMAT_TEXT:
			log_debug("clipboard (text) : %s", context->clipboard.data);
			
			break;
		case CB_FORMAT_DIB:
			log_debug("clipboard (dib) %d", response->size);
			
			break;
		default:
			log_debug("Server requested unsupported clipboard data type (%x)", event->format);
			break;
		}
	} else {
		log_debug("Send empty clipboard");
	}
	
	freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) response);
	//urdp_send_format_list(context);
}

void urdp_process_cb_data_response(urdp_context* context, RDP_CB_DATA_RESPONSE_EVENT* event) {
	/* Received clipboard data */
	UNICONV* uniconv;
	char* clipboard;
	log_debug("Received clipboard data size=%d data=%s format=%x", event->size, event->data, context->clipboard.data_format);
	if (event->size > 0 && event->data != NULL && event->data[event->size - 1] == '\0') {
		switch (context->clipboard.data_format) {
		case CB_FORMAT_UNICODETEXT:
			uniconv = freerdp_uniconv_new();
			clipboard = (char*) freerdp_uniconv_in(uniconv, event->data, event->size);
			freerdp_uniconv_free(uniconv);
			log_debug("clipboard (unicode) %s", clipboard);
			IFCALL(context->urdp_process_clipboard, context, context->clipboard.data_format, (uint8*)clipboard, strlen(clipboard));
			xfree(clipboard);
			break;
		case CB_FORMAT_TEXT:
			log_debug("clipboard (text) : %s", event->data);
			IFCALL(context->urdp_process_clipboard, context, context->clipboard.data_format, event->data, event->size);
			break;
		case CB_FORMAT_DIB:
			log_debug("clipboard (dib) %d", event->size);
			IFCALL(context->urdp_process_clipboard, context, context->clipboard.data_format, event->data, event->size);
			break;
		default:
			log_debug("Unsupported clipboard format %c", context->clipboard.data_format);
			break;
		}
	} else
		log_debug("Clipboard data missing null terminator");
}

void urdp_process_cliprdr_event(urdp_context* context, RDP_EVENT* event) {

	switch (event->event_type) {

	case RDP_EVENT_TYPE_CB_MONITOR_READY:
		urdp_process_cb_monitor_ready(context, event);
		break;

	case RDP_EVENT_TYPE_CB_FORMAT_LIST:
		urdp_process_cb_format_list(context, (RDP_CB_FORMAT_LIST_EVENT*) event);
		break;

	case RDP_EVENT_TYPE_CB_DATA_REQUEST:
		urdp_process_cb_data_request(context, (RDP_CB_DATA_REQUEST_EVENT*) event);
		break;

	case RDP_EVENT_TYPE_CB_DATA_RESPONSE:
		urdp_process_cb_data_response(context, (RDP_CB_DATA_RESPONSE_EVENT*) event);
		break;

	default:
		log_debug("Unknown cliprdr event type: 0x%x", event->event_type);
		break;
	}

}

void urdp_cliprdr_send_unicode(urdp_context* context, const char* data) {
	context->clipboard.data = (uint8*)strdup(data);
	urdp_send_format_list(context);
}
