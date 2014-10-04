/**
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
 **/

#ifndef XRDP_FONT_H
#define XRDP_FONT_H

#include "userChannel.h"

#define NUM_FONTS 0x4e00
#define DEFAULT_FONT_NAME "sans-10.fv1"

/* font */
struct xrdp_font
{
  struct xrdp_wm* wm;
  struct xrdp_font_char font_items[NUM_FONTS];
  char name[32];
  int size;
  int style;
};


struct xrdp_font* APP_CC
xrdp_font_create(struct xrdp_wm* wm);
void APP_CC
xrdp_font_delete(struct xrdp_font* self);
int APP_CC
xrdp_font_item_compare(struct xrdp_font_char* font1, struct xrdp_font_char* font2);

#endif //XRDP_FONT_H
