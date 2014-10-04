/**
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

#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/utils/event.h>
#include <freerdp/utils/memory.h>

#include <guacamole/client.h>

#include "client.h"
#include <math.h>

void guac_rdp_process_seamrdp_event(guac_client* client, RDP_EVENT* event) {
	char *input;
	int length_raw, length_base64;

	input = (char*)(event->user_data);
	length_raw = strlen(input);
	length_base64 = (length_raw+2) / 3 * 4;

	guac_socket_write_string(client->socket, "7.seamrdp,");
	guac_socket_write_int   (client->socket, length_base64);
	guac_socket_write_string(client->socket, ".");
	guac_socket_write_base64(client->socket, input, length_raw);
	guac_socket_flush_base64(client->socket);
	guac_socket_write_string(client->socket, ";");
}
