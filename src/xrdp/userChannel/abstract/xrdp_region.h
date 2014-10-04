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

#ifndef XRDP_REGION_H
#define XRDP_REGION_H

#include "userChannel.h"


/* region */
struct xrdp_region
{
  struct xrdp_wm* wm; /* owner */
  struct list* rects;
};


struct xrdp_region* APP_CC
xrdp_region_create(struct xrdp_wm* wm);
void APP_CC
xrdp_region_delete(struct xrdp_region* self);
int APP_CC
xrdp_region_add_rect(struct xrdp_region* self, struct xrdp_rect* rect);
int APP_CC
xrdp_region_insert_rect(struct xrdp_region* self, int i, int left, int top, int right, int bottom);
int APP_CC
xrdp_region_subtract_rect(struct xrdp_region* self, struct xrdp_rect* rect);
int APP_CC
xrdp_region_get_rect(struct xrdp_region* self, int index, struct xrdp_rect* rect);

#endif // XRDP_REGION_H
