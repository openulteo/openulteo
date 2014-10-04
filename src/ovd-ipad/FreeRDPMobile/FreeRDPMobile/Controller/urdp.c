/*
 * Copyright (C) 2013-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

#include <errno.h>
#include <locale.h>
#include <string.h>
#include <sys/select.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/semaphore.h>
#include <freerdp/utils/event.h>
#include <freerdp/constants.h>
#include <freerdp/plugins/cliprdr.h>
#include <freerdp/kbd/vkcodes.h>

#include "urdp.h"
#include "urdp_rdpdr.h"
#include "urdp_event.h"
#include "urdp_pointer.h"
#include "urdp_jpeg.h"

void urdp_context_new(freerdp* instance, urdp_context* context) {
	context->context.channels = freerdp_channels_new();
	pthread_mutex_init(&context->draw_mutex, NULL);
	pthread_mutex_init(&context->event_mutex, NULL);
}

void urdp_context_free(freerdp* instance, urdp_context* context) {
	IFCALL(context->urdp_context_free, context);
	pthread_mutex_destroy(&context->draw_mutex);
	pthread_mutex_destroy(&context->event_mutex);
	log_info("Connection closed");
}

void urdp_begin_paint(urdp_context* context) {
	pthread_mutex_lock(&context->draw_mutex);
	rdpGdi* gdi = context->context.gdi;
	gdi->primary->hdc->hwnd->invalid->null = 1;
	if (context->urdp_begin_draw != NULL)
		context->urdp_begin_draw(context, &context->context.gdi->primary->bitmap->data);
}

void urdp_end_paint(urdp_context* context) {
	rdpGdi* gdi = context->context.gdi;
	if (context->urdp_draw != NULL) {
		if (gdi->primary->hdc->hwnd->invalid->null) {
			context->urdp_draw(context, 0, 0, gdi->width, gdi->height);
		} else {
			context->urdp_draw(context, gdi->primary->hdc->hwnd->invalid->x, gdi->primary->hdc->hwnd->invalid->y, gdi->primary->hdc->hwnd->invalid->w,
					gdi->primary->hdc->hwnd->invalid->h);
		}
	}
	pthread_mutex_unlock(&context->draw_mutex);
}

void urdp_desktop_resize(urdp_context* context) {
	rdpSettings* settings = context->context.instance->settings;
	rdpGdi* gdi = context->context.gdi;
	HGDI_BITMAP bitmap = gdi->primary->bitmap;
    void* data = NULL;

    if (context->urdp_desktop_resize)
	  data = context->urdp_desktop_resize(context, settings->width, settings->height, gdi->dstBpp);

	if (data != NULL) {
		if (!context->gdi_initialized) {
			free(gdi->primary->bitmap->data);
		}
		bitmap->data = data;
		bitmap->width = settings->width;
		bitmap->height = settings->height;
		bitmap->scanline = bitmap->width * bitmap->bytesPerPixel;
		gdi->width = settings->width;
		gdi->height = settings->height;
	} else {
		gdi_resize(gdi, settings->width, settings->height);
	}
    
    if (context->urdp_desktop_resized)
        context->urdp_desktop_resized(context, settings->width, settings->height, gdi->dstBpp, gdi->primary->bitmap->data);
    
	context->gdi_initialized = true;
}

boolean urdp_pre_connect(freerdp* instance) {
	boolean bitmap_cache;
	rdpSettings* settings;
	urdp_context* context;

	settings = instance->settings;
	bitmap_cache = settings->bitmap_cache;

	settings->order_support[NEG_DSTBLT_INDEX] = true;
	settings->order_support[NEG_PATBLT_INDEX] = false;
	settings->order_support[NEG_SCRBLT_INDEX] = true;
	settings->order_support[NEG_OPAQUE_RECT_INDEX] = true;
	settings->order_support[NEG_DRAWNINEGRID_INDEX] = false;
	settings->order_support[NEG_MULTIDSTBLT_INDEX] = false;
	settings->order_support[NEG_MULTIPATBLT_INDEX] = false;
	settings->order_support[NEG_MULTISCRBLT_INDEX] = false;
	settings->order_support[NEG_MULTIOPAQUERECT_INDEX] = true;
	settings->order_support[NEG_MULTI_DRAWNINEGRID_INDEX] = false;
	settings->order_support[NEG_LINETO_INDEX] = true;
	settings->order_support[NEG_POLYLINE_INDEX] = false;
	settings->order_support[NEG_MEMBLT_INDEX] = bitmap_cache;
	settings->order_support[NEG_MEM3BLT_INDEX] = false;
	settings->order_support[NEG_MEMBLT_V2_INDEX] = bitmap_cache;
	settings->order_support[NEG_MEM3BLT_V2_INDEX] = false;
	settings->order_support[NEG_SAVEBITMAP_INDEX] = false;
	settings->order_support[NEG_GLYPH_INDEX_INDEX] = true;
	settings->order_support[NEG_FAST_INDEX_INDEX] = true;
	settings->order_support[NEG_FAST_GLYPH_INDEX] = true;
	settings->order_support[NEG_POLYGON_SC_INDEX] = false;
	settings->order_support[NEG_POLYGON_CB_INDEX] = false;
	settings->order_support[NEG_ELLIPSE_SC_INDEX] = false;
	settings->order_support[NEG_ELLIPSE_CB_INDEX] = false;

	settings->frame_acknowledge =  10;

	char *at = strchr(settings->username, '@');
	if (at != NULL) {
		settings->domain = strdup(at + 1);
		at[0] = 0;
	}

	context = ((urdp_context*) instance->context);

	urdp_cliprdr_init(context);
	urdp_rdpdr_init(context);
	urdp_ukbrdr_init(context);

	if (context->urdp_pre_connect) {
		AND_EXIT_CODE ret;
		if ((ret = context->urdp_pre_connect(context)) != SUCCESS)
			return ret;
	}

	freerdp_channels_pre_connect(context->context.channels, instance);

	instance->context->cache = cache_new(instance->settings);
	return true;
}

boolean urdp_post_connect(freerdp* instance) {
	rdpGdi* gdi;
	urdp_context* context;

	context = ((urdp_context*) instance->context);

	uint32 flags = context->gdi_flags;
	if (instance->settings->color_depth > 16)
		flags |= CLRBUF_32BPP;
	else
		flags |= CLRBUF_16BPP;

	gdi_init(instance, flags, NULL);
	gdi = instance->context->gdi;
	context->gdi_initialized = false;
	urdp_desktop_resize(context);

	instance->update->BeginPaint = (pBeginPaint) urdp_begin_paint;
	instance->update->EndPaint = (pEndPaint) urdp_end_paint;
	instance->update->DesktopResize = (pDesktopResize) urdp_desktop_resize;
	jpeg_cache_register_callbacks(instance->update);

	log_debug("gdi->dstBpp=%d gdi->srcBpp=%d\n", gdi->dstBpp, gdi->srcBpp);

	pointer_cache_register_callbacks(instance->update);
	urdp_register_pointer(context);

	freerdp_channels_post_connect(instance->context->channels, instance);

	if (context->urdp_post_connect != NULL)
		return context->urdp_post_connect(context) == SUCCESS;
	return true;
}

boolean urdp_verify_certificate(freerdp* instance, char* subject, char* issuer, char* fingerprint) {
	log_info("Certificate details:");
	log_info("  Subject: %s", subject);
	log_info("  Issuer: %s", issuer);
	log_info("  Thumbprint: %s", fingerprint);
	return true;
}

static int urdp_receive_channel_data(freerdp* instance, int channelId, uint8* data, int size, int flags, int total_size) {
	return freerdp_channels_data(instance, channelId, data, size, flags, total_size);
}

urdp_context* urdp_init(int context_size, p_urdp_context_new ContextNew) {
	freerdp* instance;
	//rdpChannels* channels;
	urdp_context* context;

	freerdp_channels_global_init();
	instance = freerdp_new();

	instance->PreConnect = urdp_pre_connect;
	instance->PostConnect = urdp_post_connect;
	instance->VerifyCertificate = urdp_verify_certificate;
	instance->ReceiveChannelData = urdp_receive_channel_data;

	instance->context_size = context_size;
	instance->ContextNew = (pContextNew) urdp_context_new;
	instance->ContextFree = (pContextFree) urdp_context_free;
	freerdp_context_new(instance);
	context = (urdp_context*) instance->context;
	//channels = context->context.channels;

	instance->settings->port = 3389;
	instance->settings->kbd_layout = KBD_US;
	instance->settings->color_depth = 16;
	instance->settings->bitmap_cache = true;
	instance->settings->rdp_security = true;
	instance->settings->tls_security = true;
	instance->settings->nla_security = false;
	instance->settings->encryption = true;
	instance->settings->encryption_method = ENCRYPTION_METHOD_40BIT | ENCRYPTION_METHOD_128BIT | ENCRYPTION_METHOD_FIPS;
	instance->settings->encryption_level = ENCRYPTION_LEVEL_CLIENT_COMPATIBLE;

	context->gdi_flags = CLRCONV_ALPHA;

	urdp_event_init(context);

	IFCALL(ContextNew, context);

	return context;
}

void urdp_process_printer_event(urdp_context* context, RDP_EVENT* event) {
	log_info("Printed file : %s", (char*) event->user_data);
	if (context->urdp_process_printjob)
		context->urdp_process_printjob(context, (char*) event->user_data);
}

static void urdp_process_channel_event(rdpChannels* channels, freerdp* instance) {
	RDP_EVENT* event;

	event = freerdp_channels_pop_event(channels);

	if (event) {
		switch (event->event_class) {
		case RDP_EVENT_CLASS_CLIPRDR:
			urdp_process_cliprdr_event((urdp_context*) instance->context, event);
			break;
		case 5000:
			urdp_process_printer_event((urdp_context*) instance->context, event);
			break;
		case RDP_EVENT_CLASS_UKBRDR:
			urdp_process_ukbrdr_event((urdp_context*) instance->context, event);
		default:
			log_error("urdp_process_channel_event: unknown event type %d/%d", event->event_class, event->event_type);
			break;
		}

		freerdp_event_free(event);
	}
}

AND_EXIT_CODE urdp_connect(urdp_context* context) {
	int i;
	int fds;
	int max_fds;
	int rcount;
	int wcount;
	void* rfds[32];
	void* wfds[32];
	fd_set rfds_set;
	fd_set wfds_set;
	rdpChannels* channels;
	struct timeval timeout;
	int select_status;
	AND_EXIT_CODE exit_code = SUCCESS;
	freerdp* instance = context->context.instance;

	memset(rfds, 0, sizeof(rfds));
	memset(wfds, 0, sizeof(wfds));
	memset(&timeout, 0, sizeof(struct timeval));

	if (!freerdp_connect(instance)) {
		context->rdp_run = false;
		return ERROR_RDP_CONNECT;
	}

	context->rdp_run = true;

	channels = context->context.channels;

	while (context->rdp_run) {
		rcount = 0;
		wcount = 0;

		if (freerdp_get_fds(instance, rfds, &rcount, wfds, &wcount) != true) {
			log_error("Failed to get FreeRDP file descriptor");
			break;
		}
		if (freerdp_channels_get_fds(channels, instance, rfds, &rcount, wfds, &wcount) != true) {
			log_error("Failed to get channel manager file descriptor");
			break;
		}

		max_fds = 0;
		FD_ZERO(&rfds_set);
		FD_ZERO(&wfds_set);

		for (i = 0; i < rcount; i++) {
			fds = (int) (long) (rfds[i]);

			if (fds > max_fds)
				max_fds = fds;

			FD_SET(fds, &rfds_set);
		}

		if (max_fds == 0)
			break;

		timeout.tv_sec = 2;
		select_status = select(max_fds + 1, &rfds_set, &wfds_set, NULL, &timeout);

		if (select_status == 0) {
			//freerdp_send_keep_alive(instance);
			continue;
		} else if (select_status == -1) { // these are not really errors
			if (!((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINPROGRESS) || (errno == EINTR))) { // signal occurred
				log_error("xfreerdp_run: select failed");
				exit_code = ERROR_RDP_SOCKET;
				break;
			}
		}

		pthread_mutex_lock(&context->event_mutex);
		if (freerdp_check_fds(instance) != true) {
			log_error("Failed to check FreeRDP file descriptor");
			exit_code = ERROR_RDP_CHECKFD;
			break;
		}

		if (freerdp_channels_check_fds(channels, instance) != true) {
			log_error("Failed to check channel manager file descriptor");
			exit_code = ERROR_RDP_CHECKFD;
			break;
		}

		urdp_process_channel_event(channels, instance);
		pthread_mutex_unlock(&context->event_mutex);
	}

	context->rdp_run = false;

	freerdp_channels_close(channels, instance);
	freerdp_channels_free(channels);
	context->context.gdi->primary->bitmap->data = NULL;
	gdi_free(instance);
	freerdp_disconnect(instance);
	freerdp_context_free(instance);
	freerdp_free(instance);
	freerdp_channels_global_uninit();

	return exit_code;
}
