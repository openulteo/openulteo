/*
 * Copyright (C) 2012 Ulteo SAS
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

#ifndef UPDATE_ORDER_H_
#define UPDATE_ORDER_H_


static const char* const USERCHANNEL_UPDATE_LIST[] =
{
  "begin_update",
  "end_update",
  "reset",
  "reset_clip",
  "fill_rect",
  "paint_rect",
  "screen_blt",
  "set_cursor",
  "set_clip",
  "set_fgcolor",
  "set_bgcolor",
  "set_opcode",
  "set_mixmode",
  "set_brush",
  "set_pen",
  "draw_line",
  "add_char",
  "send_to_channel",
  "paint_update"
};

typedef enum {
  begin_update,
  end_update,
  reset,
  reset_clip,
  fill_rect,
  paint_rect,
  screen_blt,
  set_cursor,
  set_clip,
  set_fgcolor,
  set_bgcolor,
  set_opcode,
  set_mixmode,
  set_brush,
  set_pen,
  draw_line,
  add_char,
  send_to_channel,
  paint_update
} order_type;

typedef struct _update {
  order_type order_type;
  char* data;
  char* mask;
  unsigned int data_len;
  unsigned int mask_len;
  unsigned int color;
  unsigned int opcode;
  unsigned int mixmode;
  unsigned int x;
  unsigned int y;
  unsigned int cx;
  unsigned int cy;
  unsigned int srcx;
  unsigned int srcy;
  unsigned int width;
  unsigned int height;
  unsigned int bpp;
  unsigned int channel_id;
  unsigned int total_data_len;
  unsigned int flags;
  unsigned int quality;
} update;



#endif /* UPDATE_ORDER_H_ */
