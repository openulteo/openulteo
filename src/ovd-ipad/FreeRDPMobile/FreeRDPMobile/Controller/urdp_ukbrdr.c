/*
 * Copyright (C) 2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2014
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
#include <freerdp/utils/unicode.h>
#include <freerdp/utils/memory.h>

#include "urdp.h"
#include "urdp_ukbrdr.h"

void urdp_ukbrdr_init(urdp_context* context) {
	/* Load ukbrdr plugin */
	int ret = freerdp_channels_load_plugin(context->context.channels, context->context.instance->settings, "ukbrdr", NULL);
	if (ret != 0) {
		log_error("failed loading ukbrdr plugin");
		return;
	}
}

void urdp_process_ukbrdr_event(urdp_context* context, RDP_EVENT* event) {
	log_debug("Got ukbrdr event !");
}

void urdp_ukbrdr_send_ime_preedit_string(urdp_context* context, const char* data, const long size) {
	RDP_EVENT *event = malloc(sizeof(RDP_EVENT));
	ukbrdrEvent *ukbrdr_ev = malloc(sizeof(ukbrdrEvent));
	struct ukb_header *ukbrdr_msg = malloc(sizeof(struct ukb_header) + size + 2);
	char *string = (char*) (ukbrdr_msg+1); /* Points to the byte following the structure */

	/* Generate Protocol message */
	memset(ukbrdr_msg, 0, sizeof(struct ukb_header) + size + 2);
	ukbrdr_msg->type = UKB_PUSH_COMPOSITION;
	ukbrdr_msg->flags = 0;
	ukbrdr_msg->len = size + 2;
	memcpy(string, data, size);

	/* Put the message in an ukbrdr event structure */
	ukbrdr_ev->size = sizeof(struct ukb_header) + size + 2;
	ukbrdr_ev->data = (char*) ukbrdr_msg;

	/* Put the ukbrdr event in an RDP event structure */
	event->event_class = RDP_EVENT_CLASS_UKBRDR;
	event->event_type = 0;
	event->on_event_free_callback = NULL;
	event->user_data = ukbrdr_ev;

	freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) event);
}

void urdp_ukbrdr_send_ime_preedit_string_stop(urdp_context* context) {
	RDP_EVENT *event = malloc(sizeof(RDP_EVENT));
	ukbrdrEvent *ukbrdr_ev = malloc(sizeof(ukbrdrEvent));
	struct ukb_header *ukbrdr_msg = malloc(sizeof(struct ukb_header));
	
	/* Generate Protocol message */
	memset(ukbrdr_msg, 0, sizeof(struct ukb_header));
	ukbrdr_msg->type = UKB_STOP_COMPOSITION;
	ukbrdr_msg->flags = 0;
	ukbrdr_msg->len = 0;
	
	/* Put the message in an ukbrdr event structure */
	ukbrdr_ev->size = sizeof(struct ukb_header);
	ukbrdr_ev->data = (char*) ukbrdr_msg;
	
	/* Put the ukbrdr event in an RDP event structure */
	event->event_class = RDP_EVENT_CLASS_UKBRDR;
	event->event_type = 0;
	event->on_event_free_callback = NULL;
	event->user_data = ukbrdr_ev;
	
	freerdp_channels_send_event(context->context.channels, (RDP_EVENT*) event);
}