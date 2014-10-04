
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libguac-client-rdp.
 *
 * The Initial Developer of the Original Code is
 * Michael Jumper.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  David PHAM-VAN <d.pham-van@ulteo.com> Ulteo SAS - http://www.ulteo.com
 *  David LECHEVALIER <david@ulteo.com> Ulteo SAS - http://www.ulteo.com - 2012
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/utils/event.h>
#include <freerdp/utils/unicode.h>
#include <freerdp/utils/memory.h>

#include <guacamole/client.h>

#include "client.h"
#include "rdp_cliprdr.h"

void guac_rdp_process_cliprdr_init(rdp_guac_client_data* client_data) {
  
    client_data->clipboard_num_formats = 1;
    client_data->clipboard_formats = (uint32*) malloc(sizeof(uint32)*client_data->clipboard_num_formats);
    client_data->clipboard_formats[0] = CB_FORMAT_UNICODETEXT;
    client_data->clipboard = NULL;

}

void guac_rdp_process_cliprdr_event(guac_client* client, RDP_EVENT* event) {

        switch (event->event_type) {

            case RDP_EVENT_TYPE_CB_MONITOR_READY:
                guac_rdp_process_cb_monitor_ready(client, event);
                break;

            case RDP_EVENT_TYPE_CB_FORMAT_LIST:
                guac_rdp_process_cb_format_list(client,
                        (RDP_CB_FORMAT_LIST_EVENT*) event);
                break;

            case RDP_EVENT_TYPE_CB_DATA_REQUEST:
                guac_rdp_process_cb_data_request(client,
                        (RDP_CB_DATA_REQUEST_EVENT*) event);
                break;

            case RDP_EVENT_TYPE_CB_DATA_RESPONSE:
                guac_rdp_process_cb_data_response(client,
                        (RDP_CB_DATA_RESPONSE_EVENT*) event);
                break;

            default:
                guac_client_log_info(client,
                        "Unknown cliprdr event type: 0x%x",
                        event->event_type);
        }

}

void guac_rdp_process_cb_monitor_ready(guac_client* client, RDP_EVENT* event) {

    rdpChannels* channels = 
        ((rdp_guac_client_data*) client->data)->rdp_inst->context->channels;

    RDP_CB_FORMAT_LIST_EVENT* format_list =
        (RDP_CB_FORMAT_LIST_EVENT*) freerdp_event_new(
            RDP_EVENT_CLASS_CLIPRDR,
            RDP_EVENT_TYPE_CB_FORMAT_LIST,
            NULL, NULL);

    /* Received notification of clipboard support. */

    /* Respond with supported format list */
    rdp_guac_client_data *client_data = (rdp_guac_client_data*) client->data;
    format_list->num_formats = client_data->clipboard_num_formats;
    guac_client_log_info(client, "guac_rdp_process_cb_monitor_ready num formats = %d", format_list->num_formats);
    format_list->formats = (uint32*) malloc(sizeof(uint32)*format_list->num_formats);    
    memcpy(format_list->formats, client_data->clipboard_formats, sizeof(uint32) * format_list->num_formats);

    freerdp_channels_send_event(channels, (RDP_EVENT*) format_list);

}

void guac_rdp_process_cb_format_list(guac_client* client,
        RDP_CB_FORMAT_LIST_EVENT* event) {

    rdpChannels* channels = 
        ((rdp_guac_client_data*) client->data)->rdp_inst->context->channels;

    rdp_guac_client_data *client_data = (rdp_guac_client_data*) client->data;

    /* Received notification of available data */
    
    int i, j;
    for (j=0; j<client_data->clipboard_num_formats; j++) {
        for (i=0; i<event->num_formats; i++) {
            /* If format available, request it */
            if (event->formats[i] == client_data->clipboard_formats[j]) {

                /* Create new data request */
                RDP_CB_DATA_REQUEST_EVENT* data_request =
                    (RDP_CB_DATA_REQUEST_EVENT*) freerdp_event_new(
                            RDP_EVENT_CLASS_CLIPRDR,
                            RDP_EVENT_TYPE_CB_DATA_REQUEST,
                            NULL, NULL);

                /* We want plain text */
                data_request->format = event->formats[i];
                client_data->clipboard_format = data_request->format;

                /* Send request */
                freerdp_channels_send_event(channels, (RDP_EVENT*) data_request);
                return;

            }

        }

    }

    /* Otherwise, no supported data available */
    guac_client_log_info(client, "Ignoring unsupported clipboard data");

}

void guac_rdp_process_cb_data_request(guac_client* client,
        RDP_CB_DATA_REQUEST_EVENT* event) {

    rdp_guac_client_data *client_data = (rdp_guac_client_data*) client->data;
    UNICONV* uniconv;
    size_t out_size;
    char empty[] = "";
    char* clip_data = empty;

    rdpChannels* channels = client_data->rdp_inst->context->channels;

    /* If text requested, send clipboard text contents */
    if (event->format == CB_FORMAT_UNICODETEXT) {

        /* Create new data response */
        RDP_CB_DATA_RESPONSE_EVENT* data_response =
            (RDP_CB_DATA_RESPONSE_EVENT*) freerdp_event_new(
                    RDP_EVENT_CLASS_CLIPRDR,
                    RDP_EVENT_TYPE_CB_DATA_RESPONSE,
                    NULL, NULL);

        if (client_data->clipboard != NULL)
            clip_data = client_data->clipboard;

        /* Set data and length */
        uniconv = freerdp_uniconv_new();
        data_response->data = (unsigned char*)freerdp_uniconv_out(uniconv, clip_data, &out_size);
        data_response->size = out_size + 2;
        freerdp_uniconv_free(uniconv);

        /* Send response */
        freerdp_channels_send_event(channels, (RDP_EVENT*) data_response);

    }

    /* Otherwise ... failure */
    else
        guac_client_log_error(client, 
                "Server requested unsupported clipboard data type");

}

void guac_rdp_process_cb_data_response(guac_client* client,
        RDP_CB_DATA_RESPONSE_EVENT* event) {
          
    rdp_guac_client_data *client_data = (rdp_guac_client_data*) client->data;
    UNICONV* uniconv;

    if(! event->data || ! event->size)
        return;

    /* Received clipboard data */
    if (event->data[event->size - 1] == '\0') {

        /* Free existing data */
        free(client_data->clipboard);
        
        /* Store clipboard data */
        switch (client_data->clipboard_format) {
            case CB_FORMAT_UNICODETEXT:                
                uniconv = freerdp_uniconv_new();
                /* Store clipboard data */
                client_data->clipboard = (char*)freerdp_uniconv_in(uniconv, event->data, event->size);
                client_data->clipboard_length = strlen(client_data->clipboard);
                fprintf(stderr, "%s\n", client_data->clipboard);
                freerdp_uniconv_free(uniconv);
                break;
            case CB_FORMAT_TEXT:
                client_data->clipboard = strdup((char*) event->data);
                client_data->clipboard_length = event->size;
                break;
            default:
                guac_client_log_error(client,
                        "Clipboard unknown data format");
                return;
        }

        /* Send clipboard data */
        guac_protocol_send_clipboard(client->socket, client_data->clipboard, client_data->clipboard_length);
      
    }
    else
        guac_client_log_error(client,
                "Clipboard data missing null terminator");

}

