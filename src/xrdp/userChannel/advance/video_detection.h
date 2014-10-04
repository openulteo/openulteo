/**
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author roullier <vincent.roullier@ulteo.com> 2013
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

#ifndef _VIDEO_DETECTION_H
#define _VIDEO_DETECTION_H

#include "xrdp_screen.h"
#include "libxrdpinc.h"
#include "list.h"
#include "userChannel.h"

#define DELTA_VIDEO 64

struct video_reg
{
  struct xrdp_rect rect;
  int nb_update;
  bool already_send;
  long timestamps;
};

void video_detection_update(struct xrdp_screen* self, int x, int y, int cx, int cy, int quality);
void update_video_regions(struct list* video, struct list* candidate, int fps);
void update_candidate_video_regions(struct list* self, struct xrdp_rect rect, int max_fps);
int video_regions_union(struct video_reg* v1, struct video_reg* v2, struct list* out, int maxfps);
void video_regions_merge(struct list* self);
void list_add_video_reg(struct list* self, int left, int top, int right, int bottom, int nb_update);
void video_regions_merge(struct list* video);
void video_detection_add_update_order(struct xrdp_screen* self, struct list* update_list, struct userChannel* u);
bool video_detection_check_video_regions(struct xrdp_screen* desktop);

#endif // _VIDEO_DETECTION_H
