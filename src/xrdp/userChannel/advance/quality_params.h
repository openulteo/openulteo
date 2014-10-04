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

#ifndef _QUALITY_PARAMS_H
#define _QUALITY_PARAMS_H

#include "arch.h"
#include "list.h"
#include "xrdp_screen.h"
#include "userChannel.h"


#define MAX_REQUEST_TIME 1000
#define MID_REQUEST_TIME 100
#define MIN_REQUEST_TIME 10

struct quality_params
{
  struct xrdp_client_info* client_info;
  bool is_progressive_display_enable;
  bool is_video_detection_enable;
  bool use_progressive_display;
  bool use_video_detection;
  int video_display_fps;
  int progressive_display_nb_level;
  int progressive_display_scale;
  int fps;
  int maxfps;
  int minfps;
  float coef_size;
};

struct quality_params* quality_params_create(struct xrdp_client_info* cl);
void quality_params_delete(struct quality_params* self);
float quality_params_estimate_size(struct quality_params* self, struct list* l1, struct list* l2);
float quality_params_estimate_progressive_display_size(struct quality_params* self, struct list* l);
float quality_params_estimate_video_regs_size(struct quality_params* self, struct list* l);
void quality_params_decrease_quality(struct quality_params* self, struct list* l1, struct list* l2);
void quality_params_increase_quality(struct quality_params* self, struct list* l1, struct list* l2);
void quality_params_decrease_quality_progressive_display(struct quality_params* self, struct list* l1);
void quality_params_increase_quality_progressive_display(struct quality_params* self, struct list* l1);
void quality_params_display(struct quality_params* self);
bool quality_params_is_max(struct quality_params* self, struct list* l1, struct list* l2);
bool quality_params_is_min(struct quality_params* self, struct list* l1);
float quality_params_estimate_size_rect(struct quality_params* self, struct update_rect* urect, int q);
void quality_params_prepare_data(struct quality_params* self, struct xrdp_screen* screen, struct userChannel* u);
void quality_params_prepare_data2(struct quality_params* self, struct xrdp_screen* screen, struct userChannel* u);
void quality_params_add_update_order(struct xrdp_screen* self, struct list* p_display, struct list* update_list);

void quality_params_prepare_data3(struct quality_params* self, struct xrdp_screen* desktop, struct userChannel* u);
float quality_params_select_regions(struct quality_params* self, struct xrdp_screen* screen, float bw, struct list* in, struct list* out);

#endif // _QUALITY_PARAMS_H
