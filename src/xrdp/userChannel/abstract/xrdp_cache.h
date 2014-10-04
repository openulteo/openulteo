/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012
 * Author Vincent ROULLIER <vincent.roullier@ulteo.com> 2013
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

#ifndef XRDP_CACHE_H
#define XRDP_CACHE_H

#include "userChannel.h"
#include "xrdp_painter.h"


/* differnce caches */
struct xrdp_cache
{
  struct xrdp_wm* wm; /* owner */
  struct xrdp_session* session;
  /* palette */
  int palette_stamp;
  struct xrdp_palette_item palette_items[6];
  /* bitmap */
  int bitmap_stamp;
  struct xrdp_bitmap_item bitmap_items[3][2000];
  int use_bitmap_comp;
  int use_jpeg_comp;
  int jpeg_quality;
  int cache1_entries;
  int cache1_size;
  int cache2_entries;
  int cache2_size;
  int cache3_entries;
  int cache3_size;
  int bitmap_cache_persist_enable;
  int bitmap_cache_version;
  /* font */
  int char_stamp;
  struct xrdp_char_item char_items[12][256];
  /* pointer */
  int pointer_stamp;
  struct xrdp_pointer_item pointer_items[32];
  int pointer_cache_entries;
  int brush_stamp;
  struct xrdp_brush_item brush_items[64];
};

int APP_CC (*xrdp_bitmap_compare_ptr)(struct xrdp_bitmap*, struct xrdp_bitmap*);

struct xrdp_cache* APP_CC
xrdp_cache_create(struct xrdp_wm* owner, struct xrdp_session* session, struct xrdp_client_info* client_info);
void APP_CC
xrdp_cache_delete(struct xrdp_cache* self);
int APP_CC
xrdp_cache_reset(struct xrdp_cache* self, struct xrdp_client_info* client_info);
int APP_CC
xrdp_cache_add_bitmap(struct xrdp_cache* self, struct xrdp_bitmap* bitmap, int quality);
int APP_CC
xrdp_cache_add_palette(struct xrdp_cache* self, int* palette);
int APP_CC
xrdp_cache_add_char(struct xrdp_cache* self, struct xrdp_font_char* font_item);
int APP_CC
xrdp_cache_add_pointer(struct xrdp_cache* self, struct xrdp_pointer_item* pointer_item);
int APP_CC
xrdp_cache_add_pointer_static(struct xrdp_cache* self, struct xrdp_pointer_item* pointer_item, int index);
int APP_CC
xrdp_cache_add_brush(struct xrdp_cache* self, char* brush_item_data);


#endif // XRDP_CACHE_H
