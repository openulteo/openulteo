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
#include <freerdp/utils/memory.h>
#include <freerdp/channels/channels.h>

#include "urdp_rdpdr.h"

RDP_PLUGIN_DATA *urdp_rdpdr_add_disks(urdp_context* context, RDP_PLUGIN_DATA *data) {
	int nb, i, dk;

	if (context->urdp_get_disks == NULL)
		return data;

	urdp_rdpdr_disk* disks = context->urdp_get_disks(context);

	for (dk = 0; disks[dk].name != NULL; dk++) {
	}

	for (nb = 0; data[nb].size != 0; nb++) {
	}

	data = realloc(data, (nb + dk + 1) * sizeof(RDP_PLUGIN_DATA));

	for (i = 0; i < dk; i++) {
		data[nb].size = sizeof(RDP_PLUGIN_DATA);
		data[nb].data[0] = "disk";
		data[nb].data[1] = disks[i].name;
		data[nb].data[2] = disks[i].path;
		data[nb].data[3] = NULL;
		nb++;
	}
	data[nb].size = 0;

	free(disks);

	return data;
}


RDP_PLUGIN_DATA *urdp_rdpdr_add_printers(urdp_context* context, RDP_PLUGIN_DATA *data) {
	int nb;

	for (nb = 0; data[nb].size != 0; nb++) {
	}

	data = realloc(data, (nb + 2) * sizeof(RDP_PLUGIN_DATA));

	data[nb].size = sizeof(RDP_PLUGIN_DATA);
	data[nb].data[0] = "rdpdr_pdf";
	data[nb].data[1] = context->urdp_get_client_name(context);
	data[nb].data[2] = NULL;
	data[nb].data[3] = NULL;
	nb++;
	data[nb].size = 0;

	return data;
}


AND_EXIT_CODE urdp_rdpdr_init(urdp_context* context) {
	RDP_PLUGIN_DATA* rdpdr_data;
	if (context->urdp_get_client_name != NULL) {
		rdpdr_data = (RDP_PLUGIN_DATA *) xmalloc(2 * sizeof(RDP_PLUGIN_DATA));
		rdpdr_data[0].size = sizeof(RDP_PLUGIN_DATA);
		rdpdr_data[0].data[0] = "clientname";
		rdpdr_data[0].data[1] = context->urdp_get_client_name(context);
		rdpdr_data[0].data[2] = NULL;
		rdpdr_data[0].data[3] = NULL;
		rdpdr_data[1].size = 0;
	} else {
		rdpdr_data = (RDP_PLUGIN_DATA *) xmalloc(sizeof(RDP_PLUGIN_DATA));
		rdpdr_data[0].size = 0;
	}

	rdpdr_data = urdp_rdpdr_add_disks(context, rdpdr_data);
	rdpdr_data = urdp_rdpdr_add_printers(context, rdpdr_data);

#ifdef DEBUG_RDPDR
	int i, j;
	for (i = 0; rdpdr_data[i].size != 0; i++) {
		for (j = 0; j <= 3; j++) {
			log_debug("rdpdr_data[%d].data[%d] = '%s'", i, j, (char*)rdpdr_data[i].data[j]);
		}
	}
#endif

	int ret = freerdp_channels_load_plugin(context->context.channels, context->context.instance->settings, "rdpdr", rdpdr_data);

	if (ret != 0)
		log_error("failed loading rdpdr plugin");

	return SUCCESS;
}
