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

#ifndef __URDP_H
#define __URDP_H

#include "config.h"
#include <stdint.h>
#include <pthread.h>
#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/gdi/dc.h>
#include <freerdp/gdi/region.h>
#include <freerdp/rail/rail.h>
#include <freerdp/cache/cache.h>

typedef struct urdp_context_t urdp_context;

typedef enum {
	SUCCESS = 0,
	//memory
	ERROR_ALLOC = 1,

	//rdp
	ERROR_RDP_INIT = 101,
	ERROR_RDP_CONNECT = 102,
	ERROR_RDP_SOCKET = 103,
	ERROR_RDP_CHECKFD = 104,

	//jni
	ERROR_JNI_FINDCLASS = 201,
	ERROR_JNI_ATTACHTHREAD = 202,
	ERROR_JNI_DETACHTHREAD = 203,
} AND_EXIT_CODE;

#include "log.h"
#include "urdp_cliprdr.h"
#include "urdp_rdpdr.h"
#include "urdp_ukbrdr.h"

typedef AND_EXIT_CODE (*p_urdp_context_new)(urdp_context* context);
typedef AND_EXIT_CODE (*p_urdp_context_free)(urdp_context* context);
typedef AND_EXIT_CODE (*p_urdp_begin_draw)(urdp_context* context, uint8 ** data);
typedef AND_EXIT_CODE (*p_urdp_draw)(urdp_context* context, int x, int y, int w, int h);
typedef AND_EXIT_CODE (*p_urdp_pre_connect)(urdp_context* context);
typedef AND_EXIT_CODE (*p_urdp_post_connect)(urdp_context* context);
typedef AND_EXIT_CODE (*p_urdp_process_channel_event)(urdp_context* context);
typedef void* (*p_urdp_desktop_resize)(urdp_context* context, int width, int height, int bpp);
typedef AND_EXIT_CODE (*p_urdp_desktop_resized)(urdp_context* context, int width, int height, int bpp, void *buffer);
typedef char* (*p_urdp_get_client_name)(urdp_context* context);
typedef urdp_rdpdr_disk* (*p_urdp_get_disks)(urdp_context* context);
typedef void (*p_urdp_process_clipboard)(urdp_context* context, uint32 format, const uint8* data, uint32 size);
typedef void (*p_urdp_process_printjob)(urdp_context* context, const char* filename);
typedef void (*p_urdp_set_pointer)(urdp_context* context, const uint8* rgba, uint32 width, uint32 height, uint32 xhot, uint32 yhot);

struct urdp_context_t {
	rdpContext context;
	pthread_mutex_t draw_mutex;
	pthread_mutex_t event_mutex;
	clipboard_context clipboard;
	boolean gdi_initialized;
	boolean rdp_run;
	uint32 gdi_flags;

	p_urdp_context_free urdp_context_free;
	p_urdp_begin_draw urdp_begin_draw;
	p_urdp_draw urdp_draw;
	p_urdp_pre_connect urdp_pre_connect;
	p_urdp_post_connect urdp_post_connect;
	p_urdp_process_channel_event urdp_process_channel_event;
	p_urdp_desktop_resize urdp_desktop_resize;
	p_urdp_desktop_resized urdp_desktop_resized;
	p_urdp_get_client_name urdp_get_client_name;
	p_urdp_get_disks urdp_get_disks;
	p_urdp_process_clipboard urdp_process_clipboard;
	p_urdp_process_printjob urdp_process_printjob;
	p_urdp_set_pointer urdp_set_pointer;
};

urdp_context* urdp_init(int context_size, p_urdp_context_new ContextNew);
AND_EXIT_CODE urdp_connect(urdp_context* context);
AND_EXIT_CODE urdp_stop(urdp_context* context);

#endif // __URDP_H
