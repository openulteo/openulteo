/*
 * Copyright (C) 2012-2013 userChannel SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012,2013
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
 */

#ifndef userChannel_MODULE_H
#define userChannel_MODULE_H

#include <common/os_calls.h>
#include <libxrdp/libxrdp.h>
#include <libxrdp/libxrdpinc.h>
#include "update_order.h"
#include "xrdp_screen.h"


#define CURRENT_MOD_VER 1


struct userChannel
{
  int size; /* size of this struct */
  int version; /* internal version */
  /* client functions */
  int (*mod_start)(struct userChannel* v, int w, int h, int bpp);
  int (*mod_connect)(struct userChannel* v);
  int (*mod_event)(struct userChannel* v, int msg, long param1, long param2,
                   long param3, long param4);
  int (*mod_signal)(struct userChannel* v);
  int (*mod_end)(struct userChannel* v);
  int (*mod_set_param)(struct userChannel* v, char* name, char* value);
  int (*mod_session_change)(struct userChannel* v, int, int);
  int (*mod_get_wait_objs)(struct userChannel* v, tbus* read_objs, int* rcount,
                           tbus* write_objs, int* wcount, int* timeout);
  int (*mod_check_wait_objs)(struct userChannel* v);
  long mod_dumby[100 - 9]; /* align, 100 minus the number of mod
                              functions above */
  /* server functions */
  int (*server_begin_update)(struct userChannel* v);
  int (*server_end_update)(struct userChannel* v);
  int (*server_fill_rect)(struct userChannel* v, int x, int y, int cx, int cy);
  int (*server_screen_blt)(struct userChannel* v, int x, int y, int cx, int cy,
                           int srcx, int srcy);
  int (*server_paint_rect)(struct userChannel* v, int x, int y, int cx, int cy,
                           char* data, int width, int height, int srcx, int srcy, int quality);
  int (*server_set_cursor)(struct userChannel* v, int x, int y, char* data, char* mask);
  int (*server_palette)(struct userChannel* v, int* palette);
  int (*server_msg)(struct userChannel* v, char* msg, int code);
  int (*server_is_term)(struct userChannel* v);
  int (*server_set_clip)(struct userChannel* v, int x, int y, int cx, int cy);
  int (*server_reset_clip)(struct userChannel* v);
  int (*server_set_fgcolor)(struct userChannel* v, int fgcolor);
  int (*server_set_bgcolor)(struct userChannel* v, int bgcolor);
  int (*server_set_opcode)(struct userChannel* v, int opcode);
  int (*server_set_mixmode)(struct userChannel* v, int mixmode);
  int (*server_set_brush)(struct userChannel* v, int x_orgin, int y_orgin,
                          int style, char* pattern);
  int (*server_set_pen)(struct userChannel* v, int style,
                        int width);
  int (*server_draw_line)(struct userChannel* v, int x1, int y1, int x2, int y2);
  int (*server_add_char)(struct userChannel* v, int font, int charactor,
                         int offset, int baseline,
                         int width, int height, char* data);
  int (*server_draw_text)(struct userChannel* v, int font,
                          int flags, int mixmode, int clip_left, int clip_top,
                          int clip_right, int clip_bottom,
                          int box_left, int box_top,
                          int box_right, int box_bottom,
                          int x, int y, char* data, int data_len);
  int (*server_reset)(struct userChannel* v, int width, int height, int bpp);
  int (*server_query_channel)(struct userChannel* v, int index,
                              char* channel_name,
                              int* channel_flags);
  int (*server_get_channel_id)(struct userChannel* v, char* name);
  int (*server_send_to_channel)(struct userChannel* v, int channel_id,
                                char* data, int data_len,
                                int total_data_len, int flags);
  long server_dumby[100 - 24]; /* align, 100 minus the number of server
                                  functions above */
  /* common */
  long handle; /* pointer to self as long */
  long wm;
  long painter;
  int sck;
  /* mod data */
  int server_width;
  int server_height;
  int server_bpp;
  int mod_width;
  int mod_height;
  int mod_bpp;
  char mod_name[256];
  int mod_mouse_state;
  int palette[256];
  int userChannel_desktop;
  char username[256];
  char password[256];
  char ip[256];
  char port[256];
  int sck_closed;
  int shift_state; /* 0 up, 1 down */
  int keylayout;
  int clip_chanid;
  char* clip_data;
  int clip_data_size;
  tbus sck_obj;
  /* mod userChannel */
  long mod_handle; /* returned from g_load_library */
  struct xrdp_mod* (*mod_lib_init)(void);
  int (*mod_lib_exit)(struct xrdp_mod*);
  struct xrdp_mod* mod;
  int efd;
  pthread_t thread;
  int terminate;
  struct list* current_update_list;
  tbus mod_mutex;
  bool need_request;
  long bandwidth;
  int rtt;
  int framerate;
  unsigned last_update_time;
  long q_params;
  struct xrdp_screen* desktop;
};


void* lib_ulteo_thread_run(void *arg);

struct userChannel* APP_CC
lib_userChannel_init(void);
int APP_CC
lib_userChannel_load_library(struct userChannel* self, char* lib);
int APP_CC
lib_userChannel_cleanup(struct userChannel* self);
int DEFAULT_CC
lib_userChannel_mod_signal(struct userChannel* u);
int DEFAULT_CC
lib_userChannel_mod_start(struct userChannel* u, int w, int h, int bpp);
int DEFAULT_CC
lib_userChannel_mod_end(struct userChannel* u);
int DEFAULT_CC
lib_userChannel_mod_connect(struct userChannel* u);
void DEFAULT_CC
lib_userChannel_set_network_stat(struct userChannel* u, long bandwidth, int rtt);
void DEFAULT_CC
lib_userChannel_set_static_framerate(struct userChannel* u, int framerate);
int DEFAULT_CC
lib_userChannel_update_screen(struct userChannel* u);
int DEFAULT_CC
lib_userChannel_update(struct userChannel* u, long *t);
int DEFAULT_CC
lib_userChannel_get_version(struct userChannel* u);




#endif
