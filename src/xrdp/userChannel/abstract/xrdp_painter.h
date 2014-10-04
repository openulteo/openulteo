/*
 * Copyright (C) 2012 userChannel SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012
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

#ifndef XRDP_PAINTER_H
#define XRDP_PAINTER_H

#include "userChannel.h"
#include "xrdp_bitmap.h"


/* painter */
struct xrdp_painter
{
  int rop;
  struct xrdp_rect* use_clip; /* nil if not using clip */
  struct xrdp_rect clip;
  int clip_children;
  int bg_color;
  int fg_color;
  int mix_mode;
  struct xrdp_brush brush;
  struct xrdp_pen pen;
  struct xrdp_session* session;
  struct xrdp_wm* wm; /* owner */
  struct xrdp_font* font;
};

struct xrdp_palette_item
{
  int stamp;
  int palette[256];
};

struct xrdp_bitmap_item
{
  int stamp;
  struct xrdp_bitmap* bitmap;
  int quality;
};

struct xrdp_char_item
{
  int stamp;
  struct xrdp_font_char font_item;
};

struct xrdp_pointer_item
{
  int stamp;
  int x; /* hotspot */
  int y;
  char data[32 * 32 * 3];
  char mask[32 * 32 / 8];
};

struct xrdp_brush_item
{
  int stamp;
  /* expand this to a structure to handle more complicated brushes
     for now its 8x8 1bpp brushes only */
  char pattern[8];
};



struct xrdp_painter* APP_CC
xrdp_painter_create(struct xrdp_wm* wm, struct xrdp_session* session);
void APP_CC
xrdp_painter_delete(struct xrdp_painter* self);
int APP_CC
xrdp_painter_begin_update(struct xrdp_painter* self);
int APP_CC
xrdp_painter_end_update(struct xrdp_painter* self);
int APP_CC
xrdp_painter_font_needed(struct xrdp_painter* self);
int APP_CC
xrdp_painter_set_clip(struct xrdp_painter* self, int x, int y, int cx, int cy);
int APP_CC
xrdp_painter_clr_clip(struct xrdp_painter* self);
int APP_CC
xrdp_painter_text_width(struct xrdp_painter* self, char* text);
int APP_CC
xrdp_painter_text_height(struct xrdp_painter* self, char* text);
int APP_CC
xrdp_painter_fill_rect(struct xrdp_painter* self, struct xrdp_bitmap* bitmap, int x, int y, int cx, int cy);
int APP_CC
xrdp_painter_draw_text(struct xrdp_painter* self, struct xrdp_bitmap* bitmap, int x, int y, const char* text);
int APP_CC
xrdp_painter_draw_text2(struct xrdp_painter* self,
                        struct xrdp_bitmap* bitmap,
                        int font, int flags, int mixmode,
                        int clip_left, int clip_top,
                        int clip_right, int clip_bottom,
                        int box_left, int box_top,
                        int box_right, int box_bottom,
                        int x, int y, char* data, int data_len);
int APP_CC
xrdp_painter_copy(struct xrdp_painter* self,
                  struct xrdp_bitmap* src,
                  struct xrdp_bitmap* dst,
                  int x, int y, int cx, int cy,
                  int srcx, int srcy, int quality);
int APP_CC
xrdp_painter_line(struct xrdp_painter* self,
                  struct xrdp_bitmap* bitmap,
                  int x1, int y1, int x2, int y2);

#endif // XRDP_PAINTER_H
