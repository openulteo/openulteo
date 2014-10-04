/**
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author roullier <vincent.roullier@ulteo.com> 2013
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
#ifndef _PROGRESSIVE_DISPLAY_H
#define _PROGRESSIVE_DISPLAY_H

#include "list.h"
#include "fifo.h"
#include "libxrdpinc.h"
#include "xrdp_screen.h"

void progressive_display_add_rect(struct xrdp_screen* self);
void progressive_display_add_update_order(struct xrdp_screen* self, struct list* l, struct list* update);
void progressive_display_update_level(struct xrdp_screen* self, long *t0);
void progressive_display_rect_union(struct update_rect* pin1, struct update_rect* pin2, struct list* out);
void progressive_display_split_and_merge(struct list* self);
void progressive_display_rect_split_v(struct list* self, struct update_rect* f, struct update_rect* s, bool* remove);
void progressive_display_rect_split_h(struct list* self, struct update_rect* f, struct update_rect* s, bool* remove);
int list_add_progressive_display_rect(struct list* l, int left, int top, int right, int bottom, int level, int send);
void list_rect_progressive_display(struct list* self);
void fifo_rect_progressive_display(struct fifo* self);

#endif // _PROGRESSIVE_DISPLAY_H
