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

#ifndef _XRDP_SCREEN_H
#define _XRDP_SCREEN_H

#include "list.h"
#include "ip_image.h"
#include "libxrdpinc.h"

struct update_rect
{
  struct xrdp_rect rect;
  int quality;
  int quality_already_send;
};

struct xrdp_screen
{
  struct ip_image* screen;
  int width;
  int height;
  int bpp;
  struct list* update_rects;
  struct list* video_regs;
  struct list* candidate_video_regs;
  struct xrdp_client_info* client_info;
  struct fifo* candidate_update_rects;
};

struct xrdp_screen* xrdp_screen_create(int w, int h, int bpp, struct xrdp_client_info* c);
void xrdp_screen_delete(struct xrdp_screen* self);
void xrdp_screen_update_desktop(struct xrdp_screen* self);
int list_add_update_rect(struct list* self, int left, int top, int right, int bottom, int quality, int send);
int update_rect_union(struct update_rect* f1, struct update_rect* f2, struct list* out);
void xrdp_screen_add_update_orders(struct xrdp_screen* desktop, struct list* l);
bool xrdp_screen_reduce_update_list(struct xrdp_screen* self, struct list* l);
bool xrdp_screen_reduce_regions(struct xrdp_screen* self, struct list* l);
bool xrdp_screen_reduce_rect(struct xrdp_screen* self, struct update_rect* urect, struct list* l);
void xrdp_screen_update_screen(struct xrdp_screen* self, int x, int y, int cx, int cy, char* data, int w, int h, int srcx, int srcy);

#endif //_XRDP_SCREEN_H
