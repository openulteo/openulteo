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

#include "userChannel.h"
#include "xrdp_painter.h"
#include "xrdp_wm.h"


/*****************************************************************************/
int DEFAULT_CC
graphics_begin_update(struct userChannel* u)
{
  struct xrdp_wm* wm;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(u->wm);
  p = xrdp_painter_create(wm, wm->session);
  xrdp_painter_begin_update(p);
  u->painter = (long)p;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
graphics_end_update(struct userChannel* u)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(u->painter);
  xrdp_painter_end_update(p);
  xrdp_painter_delete(p);
  u->painter = 0;
  return 0;
}


