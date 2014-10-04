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

#ifndef __URDP__CLIPRDR_H
#define __URDP__CLIPRDR_H

#include <freerdp/plugins/cliprdr.h>

#include "urdp.h"

typedef struct clipboard_context_t clipboard_context;
struct clipboard_context_t {
	uint32* formats;
	int num_formats;
	uint8* data;
	uint32 data_format;
	uint32 data_alt_format;
	int data_length;
};

void urdp_cliprdr_init(urdp_context* context);
void urdp_process_cliprdr_event(urdp_context* context, RDP_EVENT* event);
void urdp_cliprdr_send_unicode(urdp_context* context, const char* data);

#endif // __URDP_CLIPRDR_H
