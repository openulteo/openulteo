/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011, 2013
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

#ifndef XRDP_MM_H
#define XRDP_MM_H

#include "userChannel.h"


struct xrdp_mod
{
  int size; /* size of this struct */
  int version; /* internal version */
  /* client functions */
  int (*mod_start)(struct xrdp_mod* v, int w, int h, int bpp);
  int (*mod_connect)(struct xrdp_mod* v);
  int (*mod_event)(struct xrdp_mod* v, int msg, long param1, long param2,
                   long param3, long param4);
  int (*mod_signal)(struct xrdp_mod* v);
  int (*mod_end)(struct xrdp_mod* v);
  int (*mod_set_param)(struct xrdp_mod* v, char* name, char* value);
  int (*mod_session_change)(struct xrdp_mod* v, int, int);
  int (*mod_get_wait_objs)(struct xrdp_mod* v, tbus* read_objs, int* rcount,
                           tbus* write_objs, int* wcount, int* timeout);
  int (*mod_check_wait_objs)(struct xrdp_mod* v);
  long mod_dumby[100 - 9]; /* align, 100 minus the number of mod
                              functions above */
  /* server functions */
  int (*server_begin_update)(struct xrdp_mod* v);
  int (*server_end_update)(struct xrdp_mod* v);
  int (*server_fill_rect)(struct xrdp_mod* v, int x, int y, int cx, int cy);
  int (*server_screen_blt)(struct xrdp_mod* v, int x, int y, int cx, int cy,
                           int srcx, int srcy);
  int (*server_paint_rect)(struct xrdp_mod* v, int x, int y, int cx, int cy,
                           char* data, int width, int height, int srcx, int srcy);
  int (*server_set_pointer)(struct xrdp_mod* v, int x, int y, char* data, char* mask);
  int (*server_palette)(struct xrdp_mod* v, int* palette);
  int (*server_msg)(struct xrdp_mod* v, char* msg, int code);
  int (*server_is_term)(struct xrdp_mod* v);
  int (*server_set_clip)(struct xrdp_mod* v, int x, int y, int cx, int cy);
  int (*server_reset_clip)(struct xrdp_mod* v);
  int (*server_set_fgcolor)(struct xrdp_mod* v, int fgcolor);
  int (*server_set_bgcolor)(struct xrdp_mod* v, int bgcolor);
  int (*server_set_opcode)(struct xrdp_mod* v, int opcode);
  int (*server_set_mixmode)(struct xrdp_mod* v, int mixmode);
  int (*server_set_brush)(struct xrdp_mod* v, int x_orgin, int y_orgin,
                          int style, char* pattern);
  int (*server_set_pen)(struct xrdp_mod* v, int style,
                        int width);
  int (*server_draw_line)(struct xrdp_mod* v, int x1, int y1, int x2, int y2);
  int (*server_add_char)(struct xrdp_mod* v, int font, int charactor,
                         int offset, int baseline,
                         int width, int height, char* data);
  int (*server_draw_text)(struct xrdp_mod* v, int font,
                          int flags, int mixmode, int clip_left, int clip_top,
                          int clip_right, int clip_bottom,
                          int box_left, int box_top,
                          int box_right, int box_bottom,
                          int x, int y, char* data, int data_len);
  int (*server_reset)(struct xrdp_mod* v, int width, int height, int bpp);
  int (*server_query_channel)(struct xrdp_mod* v, int index,
                              char* channel_name,
                              int* channel_flags);
  int (*server_get_channel_id)(struct xrdp_mod* v, char* name);
  int (*server_send_to_channel)(struct xrdp_mod* v, int channel_id,
                                char* data, int data_len,
                                int total_data_len, int flags);
  long server_dumby[100 - 24]; /* align, 100 minus the number of server
                                  functions above */
  /* common */
  long handle; /* pointer to self as int */
  long wm; /* struct xrdp_wm* */
  long painter;
  int sck;
};


struct xrdp_mm
{
  struct xrdp_wm* wm; /* owner */
  int connected_state; /* true if connected to sesman else false */
  struct trans* sesman_trans; /* connection to sesman */
  int sesman_trans_up; /* true once connected to sesman */
  int delete_sesman_trans; /* boolean set when done with sesman connection */
  struct list* login_names;
  struct list* login_values;
  /* mod vars */
  long mod_handle; /* returned from g_load_library */
  struct xrdp_mod* (*mod_init)(void);
  int (*mod_exit)(struct xrdp_mod*);
  struct xrdp_mod* mod; /* module interface */
  bool connected;
  int display; /* 10 for :10.0, 11 for :11.0, etc */
  int code; /* 0 Xvnc session 10 X11rdp session */
  int sesman_controlled; /* true if this is a sesman session */
  int delete_chan_trans; /* boolean set when done with channel connection */
  struct trans* scim_trans; /* connection to scim-panel */
  int scim_trans_up; /* true once connected to scim-panel */
  int delete_scim_trans; /* boolean set when done with scim-panel connection */
};


/* module */
struct xrdp_mod_data
{
  struct list* names;
  struct list* values;
};


struct xrdp_mm* APP_CC
xrdp_mm_create(struct xrdp_wm* owner);
void APP_CC
xrdp_mm_delete(struct xrdp_mm* self);
int APP_CC
xrdp_mm_load_userchannel(struct xrdp_mm* self, const char* lib);
int APP_CC
xrdp_mm_process_channel_data(struct xrdp_mm* self, tbus param1, tbus param2, tbus param3, tbus param4);
int APP_CC
xrdp_mm_connect(struct xrdp_mm* self);
int APP_CC
xrdp_mm_send_disconnect(struct xrdp_mm* self);
int APP_CC
xrdp_mm_end(struct xrdp_mm* self);
void APP_CC
xrdp_mm_set_network_stat(struct xrdp_mm* self, long bandwidth, int rtt);
void APP_CC
xrdp_mm_set_static_framerate(struct xrdp_mm* self, int framerate);
int APP_CC
xrdp_mm_get_wait_objs(struct xrdp_mm* self, tbus* read_objs, int* rcount, tbus* write_objs, int* wcount, int* timeout);
int APP_CC
xrdp_mm_check_wait_objs(struct xrdp_mm* self);
int DEFAULT_CC
server_fill_rect(struct userChannel* mod, int x, int y, int cx, int cy);
int DEFAULT_CC
server_screen_blt(struct userChannel* mod, int x, int y, int cx, int cy, int srcx, int srcy);
int DEFAULT_CC
server_paint_update(struct userChannel* mod, int x, int y, int cx, int cy, char* data);
int DEFAULT_CC
server_paint_rect(struct userChannel* mod, int x, int y, int cx, int cy, char* data, int width, int height, int srcx, int srcy, int quality);
int DEFAULT_CC
server_set_pointer(struct userChannel* mod, int x, int y, char* data, char* mask);
int DEFAULT_CC
server_palette(struct userChannel* mod, int* palette);
int DEFAULT_CC
server_msg(struct userChannel* mod, char* msg, int code);
int DEFAULT_CC
server_is_term(struct userChannel* mod);
int DEFAULT_CC
server_set_clip(struct userChannel* mod, int x, int y, int cx, int cy);
int DEFAULT_CC
server_reset_clip(struct userChannel* mod);
int DEFAULT_CC
server_set_fgcolor(struct userChannel* mod, int fgcolor);
int DEFAULT_CC
server_set_bgcolor(struct userChannel* mod, int bgcolor);
int DEFAULT_CC
server_set_opcode(struct userChannel* mod, int opcode);
int DEFAULT_CC
server_set_mixmode(struct userChannel* mod, int mixmode);
int DEFAULT_CC
server_set_brush(struct userChannel* mod, int x_orgin, int y_orgin, int style, char* pattern);
int DEFAULT_CC
server_set_pen(struct userChannel* mod, int style, int width);
int DEFAULT_CC
server_draw_line(struct userChannel* mod, int x1, int y1, int x2, int y2);
int DEFAULT_CC
server_add_char(struct userChannel* mod, int font, int charactor, int offset, int baseline, int width, int height, char* data);
int DEFAULT_CC
server_draw_text(struct userChannel* mod, int font, int flags, int mixmode, int clip_left, int clip_top, int clip_right, int clip_bottom, int box_left, int box_top, int box_right, int box_bottom, int x, int y, char* data, int data_len);
int DEFAULT_CC
server_reset(struct userChannel* mod, int width, int height, int bpp);
int DEFAULT_CC
server_query_channel(struct userChannel* mod, int index, char* channel_name, int* channel_flags);
int DEFAULT_CC
server_get_channel_id(struct userChannel* mod, char* name);
int DEFAULT_CC
server_send_to_channel(struct userChannel* mod, int channel_id, char* data, int data_len, int total_data_len, int flags);
int APP_CC
xrdp_mm_scim_send_unicode(struct xrdp_mm* self, unsigned int unicode_key);


#endif // XRDP_MM_H

