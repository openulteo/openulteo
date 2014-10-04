/**
 * FreeRDP: A Remote Desktop Protocol client.
 * Ukbrdr : Ulteo keyboard extended redirection
 *
 * Copyright 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freerdp/types.h>
#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/svc_plugin.h>

typedef struct ukbrdr_plugin
{
	rdpSvcPlugin plugin;
} ukbrdrPlugin;

typedef struct ukbrdr_event
{
	unsigned int size;
	char *data;
} ukbrdrEvent;

static void ukbrdr_process_connect(rdpSvcPlugin* plugin)
{
	/* Vchannel connection callback */
	printf("Ukbrdr connect\n");
}

static void ukbrdr_process_receive(rdpSvcPlugin* plugin, STREAM* data_in)
{
	/* Vchannel receive proc (receive data FROM server) */
	/* - Get data as a stream
		 - Parse it
		 - Generate an event
		 - Send it to the main process "with svc_plugin_send_event"
		 - Free stream
	*/

	/* stream_get_size gives the data size in bytes */
	int len = stream_get_size(data_in);
	RDP_EVENT *event = malloc(sizeof(RDP_EVENT));
	ukbrdrEvent *ukbrdr_ev = malloc(sizeof(ukbrdrEvent));

	/* Create a new event and copy data from stream */
	/* Be carefull ! There is no guarantee that the stream is null-terminated */
	ukbrdr_ev->size = len;
	ukbrdr_ev->data = malloc(len);
	memcpy(ukbrdr_ev->data, (const char*) stream_get_data(data_in), ukbrdr_ev->size);

	event->event_class = RDP_EVENT_CLASS_UKBRDR;
	event->event_type = 0;
	event->on_event_free_callback = NULL;
	event->user_data = ukbrdr_ev;

	/* Send the event to the main program */
	svc_plugin_send_event(plugin, event);

	stream_free(data_in);
}

static void ukbrdr_process_event(rdpSvcPlugin* plugin, RDP_EVENT* event)
{
	/* Vchannel send proc (send data to server) */
	/* - Get data as an event
		 - Convert-it to a stream
		 - Send it to the server with "svc_plugin_send"
		 - Free event
	*/

	ukbrdrEvent *ukbrdr_ev = event->user_data;
	unsigned int len = ukbrdr_ev->size;
	char *data = ukbrdr_ev->data;
	STREAM *stream = stream_new(len);

	/* Copy event data to stream */
	stream_write(stream, data, len);

	/* Send the stream to the server */
	svc_plugin_send(plugin, stream);

	free(ukbrdr_ev->data);
	free(event->user_data);
	freerdp_event_free(event);
}

static void ukbrdr_process_terminate(rdpSvcPlugin* plugin)
{
	/* Vchannel close callback */
	printf("Ukbrdr terminate\n");
}

DEFINE_SVC_PLUGIN(ukbrdr, "ukbrdr", CHANNEL_OPTION_INITIALIZED | CHANNEL_OPTION_ENCRYPT_RDP)
