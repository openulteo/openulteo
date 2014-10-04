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

#ifndef ABSTRACT_H_
#define ABSTRACT_H_

#include "arch.h"
#include "userChannel.h"


int DEFAULT_CC
lib_userChannel_server_begin_update (struct xrdp_mod* mod);
int DEFAULT_CC
lib_userChannel_server_end_update(struct xrdp_mod* mod);
int DEFAULT_CC
lib_userChannel_server_reset(struct xrdp_mod* mod, int width, int height, int bpp);
int DEFAULT_CC
lib_userChannel_server_fill_rect(struct xrdp_mod* mod, int x, int y, int cx, int cy);
int DEFAULT_CC
lib_userChannel_server_set_fgcolor(struct xrdp_mod* mod, int fgcolor);
int DEFAULT_CC
lib_userChannel_server_paint_rect(struct xrdp_mod* mod, int x, int y, int cx, int cy, char* data, int width, int height, int srcx, int srcy);
int DEFAULT_CC
lib_userChannel_server_screen_blt(struct xrdp_mod* mod, int x, int y, int cx, int cy, int srcx, int srcy);
int DEFAULT_CC
lib_userChannel_server_set_pointer(struct xrdp_mod* mod, int x, int y, char* data, char* mask);
int DEFAULT_CC
lib_userChannel_server_msg(struct xrdp_mod* mod, char* msg, int code);
int DEFAULT_CC
lib_userChannel_server_is_term(struct xrdp_mod* mod);
int DEFAULT_CC
lib_userChannel_server_palette(struct xrdp_mod* mod, int* palette);
int DEFAULT_CC
lib_userChannel_server_set_clip(struct xrdp_mod* mod, int x, int y, int cx, int cy);
int DEFAULT_CC
lib_userChannel_server_reset_clip(struct xrdp_mod* mod);
int DEFAULT_CC
lib_userChannel_server_set_bgcolor(struct xrdp_mod* mod, int bgcolor);
int DEFAULT_CC
lib_userChannel_server_set_opcode(struct xrdp_mod* mod, int opcode);
int DEFAULT_CC
lib_userChannel_server_set_mixmode(struct xrdp_mod* mod, int mixmode);
int DEFAULT_CC
lib_userChannel_server_set_brush(struct xrdp_mod* mod, int x_orgin, int y_orgin, int style, char* pattern);
int DEFAULT_CC
lib_userChannel_server_set_pen(struct xrdp_mod* mod, int style, int width);
int DEFAULT_CC
lib_userChannel_server_draw_line(struct xrdp_mod* mod, int x1, int y1, int x2, int y2);
int DEFAULT_CC
lib_userChannel_server_add_char(struct xrdp_mod* mod, int font, int charactor, int offset, int baseline, int width, int height, char* data);
int DEFAULT_CC
lib_userChannel_server_draw_text(struct xrdp_mod* mod, int font, int flags, int mixmode, int clip_left, int clip_top, int clip_right, int clip_bottom, int box_left, int box_top, int box_right, int box_bottom, int x, int y, char* data, int data_len);
int DEFAULT_CC
lib_userChannel_server_query_channel(struct xrdp_mod* mod, int index, char* channel_name, int* channel_flags);
int DEFAULT_CC
lib_userChannel_server_get_channel_id(struct xrdp_mod* mod, char* name);
int DEFAULT_CC
lib_userChannel_server_send_to_channel(struct xrdp_mod* mod, int channel_id, char* data, int data_len, int total_data_len, int flags);

#endif // ABSTRACT_H_
