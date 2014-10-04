/**
 * Copyright (C) 2011-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011, 2012
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

#ifndef XRDP_BITMAP_H
#define XRDP_BITMAP_H

#include "userChannel.h"


struct xrdp_painter;


/* header for bmp file */
struct xrdp_bmp_header
{
  int size;
  int image_width;
  int image_height;
  short planes;
  short bit_count;
  int compression;
  int image_size;
  int x_pels_per_meter;
  int y_pels_per_meter;
  int clr_used;
  int clr_important;
};

/* window or bitmap */
struct xrdp_bitmap
{
  /* 0 = bitmap 1 = window 2 = screen 3 = button 4 = image 5 = edit
     6 = label 7 = combo 8 = special */
  int type;
  int width;
  int height;
  struct xrdp_wm* wm;
  /* msg 1 = click 2 = mouse move 3 = paint 100 = modal result */
  /* see messages in constants.h */
  int (*notify)(struct xrdp_bitmap* wnd, struct xrdp_bitmap* sender,
                int msg, long param1, long param2);
  /* for bitmap */
  int bpp;
  int line_size; /* in bytes */
  int do_not_free_data;
  char* data;
  /* for all but bitmap */
  int left;
  int top;
  int pointer;
  int bg_color;
  int tab_stop;
  int id;
  char* caption1;
  /* for window or screen */
  struct xrdp_bitmap* modal_dialog;
  struct xrdp_bitmap* focused_control;
  struct xrdp_bitmap* owner; /* window that created us */
  struct xrdp_bitmap* parent; /* window contained in */
  /* for modal dialog */
  struct xrdp_bitmap* default_button; /* button when enter is pressed */
  struct xrdp_bitmap* esc_button; /* button when esc is pressed */
  /* list of child windows */
  struct list* child_list;
  /* for edit */
  int edit_pos;
  twchar password_char;
  /* for button or combo */
  int state; /* for button 0 = normal 1 = down */
  /* for combo */
  struct list* string_list;
  struct list* data_list;
  /* for combo or popup */
  int item_index;
  /* for popup */
  struct xrdp_bitmap* popped_from;
  int item_height;
  /* crc */
  int crc;
  /* coords */
  long *coords;
  int srcX, srcY;
};


struct xrdp_bitmap* APP_CC
xrdp_bitmap_create(int width, int height, int bpp, int type, struct xrdp_wm* wm);
struct xrdp_bitmap* APP_CC
xrdp_bitmap_create_with_data(int width, int height, int bpp, char* data, struct xrdp_wm* wm);
void APP_CC
xrdp_bitmap_delete(struct xrdp_bitmap* self);
struct xrdp_bitmap* APP_CC
xrdp_bitmap_get_child_by_id(struct xrdp_bitmap* self, int id);
int APP_CC
xrdp_bitmap_set_focus(struct xrdp_bitmap* self, int focused);
int APP_CC
xrdp_bitmap_resize(struct xrdp_bitmap* self, int width, int height);
int APP_CC
xrdp_bitmap_load(struct xrdp_bitmap* self, const char* filename, int* palette);
int APP_CC
xrdp_bitmap_get_pixel(struct xrdp_bitmap* self, int x, int y);
int APP_CC
xrdp_bitmap_set_pixel(struct xrdp_bitmap* self, int x, int y, int pixel);
int APP_CC
xrdp_bitmap_copy_box(struct xrdp_bitmap* self, struct xrdp_bitmap* dest, int x, int y, int cx, int cy);
int APP_CC
xrdp_bitmap_copy_box_with_crc(struct xrdp_bitmap* self, struct xrdp_bitmap* dest, int x, int y, int cx, int cy);
int APP_CC
xrdp_bitmap_compare(struct xrdp_bitmap* self, struct xrdp_bitmap* b);
int APP_CC
xrdp_bitmap_compare_with_crc(struct xrdp_bitmap* self, struct xrdp_bitmap* b);
int APP_CC
xrdp_bitmap_compare_subtile_with_crc(struct xrdp_bitmap* self, struct xrdp_bitmap* b);
int APP_CC
xrdp_bitmap_invalidate(struct xrdp_bitmap* self, struct xrdp_rect* rect);
int APP_CC
xrdp_bitmap_def_proc(struct xrdp_bitmap* self, int msg, int param1, int param2);
int APP_CC
xrdp_bitmap_to_screenx(struct xrdp_bitmap* self, int x);
int APP_CC
xrdp_bitmap_to_screeny(struct xrdp_bitmap* self, int y);
int APP_CC
xrdp_bitmap_from_screenx(struct xrdp_bitmap* self, int x);
int APP_CC
xrdp_bitmap_from_screeny(struct xrdp_bitmap* self, int y);
int APP_CC
xrdp_bitmap_get_screen_clip(struct xrdp_bitmap* self,
                            struct xrdp_painter* painter,
                            struct xrdp_rect* rect,
                            int* dx, int* dy);
int APP_CC
xrdp_bitmap_add_coords(struct xrdp_bitmap* self, int x, int y);
int APP_CC
xrdp_bitmap_is_contained_by(struct xrdp_bitmap* self, struct xrdp_bitmap* other, int* x, int* y);
int APP_CC
xrdp_bitmap_compare_sub_tile(struct xrdp_bitmap* self, struct xrdp_bitmap* other, int x, int y);

#endif // XRDP_BITMAP_H
