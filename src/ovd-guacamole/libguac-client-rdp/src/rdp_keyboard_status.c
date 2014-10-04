/**
* Copyright (C) 2013 Ulteo SAS
* http://www.ulteo.com
* Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2013
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
#include <guacamole/client.h>
#include "client.h"
#include "rdp_keyboard_status.h"

void guac_rdp_keyboard_ime_state(rdpInput* input, uint32 imeState, uint32 imeConvMode) {
	rdpContext* context = input->context;
	guac_client* client = ((rdp_freerdp_context*) context)->client;
	guac_protocol_send_keyboard_ime_state(client->socket, imeState, imeConvMode);
}

