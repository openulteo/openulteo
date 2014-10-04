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

#include "quality_params.h"
#include "os_calls.h"
#include "xrdp_wm.h"
#include "fifo.h"
#include "funcs.h"
#include "progressive_display.h"
#include "video_detection.h"

struct quality_params* quality_params_create(struct xrdp_client_info* cl)
{
  struct quality_params* self;
  self = (struct quality_params*) g_malloc(sizeof(struct quality_params), 1);
  self->client_info = cl;
  if (cl->use_video_detection)
  {
    self->is_video_detection_enable = true;
    self->video_display_fps = cl->video_display_fps;
    self->use_video_detection = true;
  }
  else
  {
    self->is_video_detection_enable = false;
    self->use_video_detection = false;
  }
  if (cl->use_progressive_display)
  {
    self->is_progressive_display_enable = true;
    self->use_progressive_display = true;
    self->progressive_display_nb_level = cl->progressive_display_nb_level;
    self->progressive_display_scale = cl->progressive_display_scale;
  }
  else
  {
    self->is_progressive_display_enable = false;
    self->use_progressive_display = false;
  }
  self->maxfps = cl->progressive_display_maxfps;
  self->minfps = cl->progressive_display_minfps;
  self->fps = self->maxfps;

  self->coef_size = 1;
  return self;
}

void quality_params_delete(struct quality_params* self)
{
  g_free(self);
}

float quality_params_estimate_size(struct quality_params* self, struct list* lr, struct list* lv)
{
  float vregs_estimate_size = 0;
  float urect_estimate_size = 0;

  urect_estimate_size = quality_params_estimate_progressive_display_size(self, lr);

  if (self->is_video_detection_enable)
  {
    vregs_estimate_size = quality_params_estimate_video_regs_size(self, lv);
  }
  return (urect_estimate_size + vregs_estimate_size);
}

float quality_params_estimate_progressive_display_size(struct quality_params* self, struct list* l)
{
  float size = 0;
  if (l->count > 0)
  {
    int i;
    int w, h;
    int Bpp = (self->client_info->bpp + 7) / 8;
    int scale_factor;
    Bpp = (Bpp == 3) ? 4 : Bpp;
    for (i = 0; i < l->count; i++)
    {
      struct update_rect* cur = (struct update_rect*) list_get_item(l, i);
      if ((rect_width(&cur->rect) < 30 || rect_height(&cur->rect) < 30) && cur->quality_already_send == -1)
      {
        cur->quality = 0;
      }

      if (cur->quality == 0)
        scale_factor = 1;
      else
        scale_factor = self->progressive_display_scale << cur->quality;
      w = rect_width(&cur->rect);
      h = rect_height(&cur->rect);
      size += Bpp * w * h / (scale_factor * scale_factor);
    }
  }
  return size;
}

float quality_params_estimate_video_regs_size(struct quality_params* self, struct list* l)
{
  float size = 0;
  if (l->count > 0)
  {
    int i;
    float w, h;
    int Bpp = (self->client_info->bpp + 7) / 8;
    Bpp = (Bpp == 3) ? 4 : Bpp;
    for (i = 0; i < l->count; i++)
    {
      struct video_reg* cur = (struct video_reg*) list_get_item(l, i);
      w = (float) rect_width(&cur->rect);
      h = (float) rect_height(&cur->rect);
      size +=((float) Bpp) * w * h;
    }
  }
  return size;
}

void quality_params_decrease_quality_progressive_display(struct quality_params* self, struct list* l)
{
  int i;
  for (i = 0; i < l->count; i++)
  {
    struct update_rect* cur = (struct update_rect*) list_get_item(l, i);
    if ((rect_width(&cur->rect) < 30 || rect_height(&cur->rect) < 30) && cur->quality_already_send == -1)
    {
      cur->quality = 0;
    }
    else
    {
      if (cur->quality_already_send == -1)
      {
        if (cur->quality != self->progressive_display_nb_level - 1)
          cur->quality++;
      }
      else
      {
        if (cur->quality != cur->quality_already_send - 1)
        {
          cur->quality++;
        }

      }
    }
  }
}

void quality_params_increase_quality_progressive_display(struct quality_params* self, struct list* l)
{
  int i;
  for (i = 0; i < l->count; i++)
  {
    struct update_rect* cur = (struct update_rect*) list_get_item(l, i);
    if (cur->quality >= 1)
      cur->quality--;
    else
      cur->quality = 0;
//		}
//		else
//		{
//			cur->already_send = false;
//		}
  }
}

void quality_params_decrease_quality(struct quality_params* self, struct list* lr, struct list* lv)
{
  if (self->is_progressive_display_enable)
  {
    quality_params_decrease_quality_progressive_display(self, lr);
  }
}

void quality_params_increase_quality(struct quality_params* self, struct list* lr, struct list* lv)
{
  if (self->is_video_detection_enable)
  {
    self->use_video_detection = true;
    self->video_display_fps = 1;
  }

  if (self->is_progressive_display_enable)
  {
    quality_params_increase_quality_progressive_display(self, lr);
  }
}

void quality_params_display(struct quality_params* self)
{
  printf("Quality Params : \n");
  printf("  - Use progressive display : %i \n", self->use_progressive_display);
  printf("   -- Progressive display nb level : %i \n", self->progressive_display_nb_level);
  printf("   -- Progressive display scale : %i \n", self->progressive_display_scale);
  printf("  - Use Video detection : %i \n", self->use_video_detection);
  printf("   -- Video Detection fps : %i \n", self->video_display_fps);
}

bool quality_params_is_max(struct quality_params* self, struct list* lr, struct list* lv)
{
  bool pd = true;
  bool vd = true;
  int i;
  for (i = 0; i < lr->count; i++)
  {
    struct update_rect* cur = (struct update_rect*) list_get_item(lr, i);
    if (cur->quality != 0)
    {
      pd = false;
      break;
    }
  }
  if (self->is_video_detection_enable)
    if (self->video_display_fps != 1)
      vd = false;
  return pd && vd;
}

bool quality_params_is_min(struct quality_params* self, struct list* lr)
{
  bool pd = true;
  int i;
  for (i = 0; i < lr->count; i++)
  {
    struct update_rect* cur = (struct update_rect*) list_get_item(lr, i);
    if ((cur->quality_already_send == -1 && cur->quality != self->progressive_display_nb_level - 1) ||
        (cur->quality_already_send != -1 && cur->quality != cur->quality_already_send - 1))
    {
      pd = false;
      break;
    }
  }
  return pd;
}

float quality_params_estimate_size_rect(struct quality_params* self, struct update_rect* urect, int q)
{
  int scale_factor;
  int Bpp = (self->client_info->bpp + 7) / 8;
  Bpp = (Bpp == 3) ? 4 : Bpp;
  if (q < urect->quality_already_send)
  {
    if (q == 0)
    {
      scale_factor = 1;
    }
    else
    {
      scale_factor = self->progressive_display_scale << q;
    }
    return rect_width(&urect->rect) * rect_height(&urect->rect) * Bpp / (scale_factor * scale_factor);
  }
  else
    return 0;
}

void quality_params_prepare_data(struct quality_params* self, struct xrdp_screen* desktop, struct userChannel* u)
{
  struct xrdp_wm* wm = (struct xrdp_wm*) u->wm;
  float estimate_size = 0;

  if (wm->session->next_request_time >= MAX_REQUEST_TIME)
  {
    self->coef_size = 3;
  }
  else if (wm->session->next_request_time >= MID_REQUEST_TIME && wm->session->next_request_time < MAX_REQUEST_TIME)
  {
    self->coef_size = (self->coef_size > 3) ? 3 : self->coef_size * 1.5;
  }
  else if (wm->session->next_request_time < MID_REQUEST_TIME && wm->session->next_request_time >= MIN_REQUEST_TIME)
  {
    self->coef_size =
        (self->coef_size * 0.2 < 0.05) ? 0.05 : self->coef_size * 0.2;
  }

  float estimate_size_coef = self->coef_size;
  struct list* list_rq = list_create();
  list_rq->auto_free = true;
  unsigned int bandwidth =
      (u->bandwidth) ? (u->bandwidth) : wm->session->bandwidth;
  if (bandwidth < 10)
    self->fps = 5;
  float bw = ((float) (bandwidth)) / ((float) (self->fps));
  struct list* l_update = list_create();
  l_update->auto_free = true;
  bool q_params_min;
  int i;
  if (desktop->update_rects->count > 0)
  {
    l_update = desktop->update_rects;
  }
  if (desktop->candidate_update_rects->count > 0)
  {
    while (!fifo_is_empty(desktop->candidate_update_rects))
    {
      struct update_rect* cur = fifo_pop(desktop->candidate_update_rects);
      list_add_item(l_update, (tbus) cur);
    }
  }

  if (l_update->count > 0)
  {
    for (i = l_update->count - 1; i >= 0; i--)
    {
      struct update_rect * cur = (struct update_rect*) list_get_item(l_update, i);
      if (cur->quality_already_send == -1)
      {
        struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
        g_memcpy(tmp, cur, sizeof(struct update_rect));
        list_add_item(list_rq, (tbus) tmp);
        list_remove_item(l_update, i);
      }
    }
    if (list_rq->count > 0)
    {
      estimate_size = estimate_size_coef * quality_params_estimate_progressive_display_size(self, list_rq) / 1024;
      q_params_min = quality_params_is_min(self, list_rq);
      int cpt = list_rq->count * (self->progressive_display_nb_level - 1);
      while (estimate_size > bw && !q_params_min && cpt-- > 0)
      {
        quality_params_decrease_quality(self, list_rq, desktop->video_regs);
        estimate_size = estimate_size_coef * quality_params_estimate_progressive_display_size(self, list_rq) / 1024;
        q_params_min = quality_params_is_min(self, list_rq);
      }

      if (estimate_size <= bw)
      {
        if (self->is_video_detection_enable)
        {
          float vr_estimate_size = estimate_size_coef * quality_params_estimate_video_regs_size(self, desktop->video_regs) / 1024;
          if ((estimate_size + vr_estimate_size) <= bw)
          {
            self->use_video_detection = true;
            self->video_display_fps = 1;
            estimate_size += vr_estimate_size;
          }
          else
          {
            self->use_video_detection = true;
            self->video_display_fps = 0;
          }
        }
      }
      else if (q_params_min && estimate_size > bw)
      {
        bool r_list = xrdp_screen_reduce_update_list(desktop, list_rq);
        estimate_size = estimate_size_coef * quality_params_estimate_size(self, list_rq, desktop->video_regs) / 1024;
        while (estimate_size >= bw && r_list)
        {
          r_list = xrdp_screen_reduce_update_list(desktop, list_rq);
          estimate_size = estimate_size_coef * quality_params_estimate_size(self, list_rq, desktop->video_regs) / 1024;
        }
      }
    }
    else if (l_update->count > 0)
    {
      struct list* list_r = list_create();
      list_r->auto_free = true;
      int i = 0;
      int q = self->progressive_display_nb_level - 1;
      for (; q >= 0; q--)
      {
        for (i = l_update->count - 1; i >= 0; i--)
        {
          struct update_rect * cur = (struct update_rect*) list_get_item(l_update, i);
          if (cur->quality_already_send == q)
          {
            struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
            g_memcpy(tmp, cur, sizeof(struct update_rect));
            list_add_item(list_r, (tbus) tmp);
            list_remove_item(l_update, i);
          }
        }
      }
      if (list_r->count == 0 && list_rq->count == 0)
        list_clear(l_update);

      bool added = false;
      for (q = 0; q < self->progressive_display_nb_level - 1; q++)
      {
        for (i = list_r->count - 1; i >= 0; i--)
        {
          struct update_rect* cur = (struct update_rect*) list_get_item(list_r, i);
          if (q < cur->quality_already_send)
          {
            float es = estimate_size_coef * quality_params_estimate_size_rect(self, cur, q) / 1024;
            if ((estimate_size + es) < bw)
            {
              struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
              g_memcpy(tmp, cur, sizeof(struct update_rect));
              tmp->quality = q;
              list_add_item(list_rq, (tbus) tmp);
              list_remove_item(list_r, i);
              estimate_size += es;
              added = true;
            }
          }
        }
      }

      if (!added)
      {
        for (i = list_r->count - 1; i >= 0; i--)
        {
          struct update_rect* cur = (struct update_rect*) list_get_item(list_r, i);
          xrdp_screen_reduce_rect(desktop, cur, list_r);
          for (q = 0; q < cur->quality_already_send; q++)
          {
            float es = estimate_size_coef * quality_params_estimate_size_rect(self, cur, q) / 1024;
            if ((estimate_size + es) < bw)
            {
              struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
              g_memcpy(tmp, cur, sizeof(struct update_rect));
              tmp->quality = q;
              list_add_item(list_rq, (tbus) tmp);
              estimate_size += es;
              list_remove_item(list_r, i);
              break;
            }
          }
        }
      }

      if (list_r->count > 0)
      {
        for (i = list_r->count - 1; i >= 0; i--)
        {
          struct update_rect* cur = (struct update_rect*) list_get_item(list_r, i);
          struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
          g_memcpy(tmp, cur, sizeof(struct update_rect));
          tmp->quality = 0;
          fifo_push(desktop->candidate_update_rects, tmp);
          list_remove_item(list_r, i);
        }
      }
      list_delete(list_r);
      if (self->is_video_detection_enable && desktop->video_regs->count > 0)
      {
        float vr_estimate_size = estimate_size_coef * quality_params_estimate_video_regs_size(self, desktop->video_regs) / 1024;
        if (vr_estimate_size <= bw)
        {
          self->use_video_detection = true;
          self->video_display_fps = 1;
          estimate_size += vr_estimate_size;
        }
        else
        {
          self->use_video_detection = true;
          self->video_display_fps = 0;
        }
      }
    }
  }
  else
  {
    struct update_rect* fu_rect;
    struct update_rect* urect;
    struct xrdp_rect intersection;
    struct update_rect* ur;
    struct update_rect* tmp;
    bool no_inter = true;
    struct list* l_tmp = list_create();
    l_tmp->auto_free = 1;
    int i, j;
    while (!fifo_is_empty(desktop->candidate_update_rects))
    {
      fu_rect = (struct update_rect*) fifo_pop(desktop->candidate_update_rects);
      if (list_rq->count /*desktop->update_rects->count */> 0)
      {
        no_inter = true;
        for (i = list_rq->count /*desktop->update_rects->count*/- 1; i >= 0;
            i--)
        {
          urect = (struct update_rect*) list_get_item(list_rq /*desktop->update_rects*/, i);
          if (!rect_equal(&urect->rect, &fu_rect->rect))
          {
            if (rect_intersect(&urect->rect, &fu_rect->rect, &intersection))
            {
              no_inter = false;
              progressive_display_rect_union(fu_rect, urect, l_tmp);
              list_remove_item(list_rq /*desktop->update_rects*/, i);
              for (j = 0; j < l_tmp->count; j++)
              {
                ur = (struct update_rect*) list_get_item(l_tmp, j);
                tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
                g_memcpy(tmp, ur, sizeof(struct update_rect));
                fifo_push(desktop->candidate_update_rects, tmp);
              }
              list_clear(l_tmp);
              break;
            }
          }
          else
          {
            no_inter = false;
            urect->quality = fu_rect->quality;
            urect->quality_already_send = fu_rect->quality_already_send;
            break;
          }
        }
        if (no_inter)
        {
          list_add_progressive_display_rect(list_rq /*desktop->update_rects*/, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
        }
      }
      else
      {
        list_add_progressive_display_rect(list_rq/*desktop->update_rects*/, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
      }
    }
  }
  progressive_display_add_update_order(desktop, list_rq, u->current_update_list);
  if (self->is_video_detection_enable)
  {
    video_detection_add_update_order(desktop, u->current_update_list, u);
  }
  list_delete(list_rq);
}

void quality_params_prepare_data2(struct quality_params* self, struct xrdp_screen* desktop, struct userChannel* u)
{
  struct xrdp_wm* wm = (struct xrdp_wm*) u->wm;
  float estimate_size = 0;

  if (wm->session->next_request_time >= MAX_REQUEST_TIME)
  {
    self->coef_size = 3;
  }
  else if (wm->session->next_request_time >= MID_REQUEST_TIME && wm->session->next_request_time < MAX_REQUEST_TIME)
  {
    self->coef_size = (self->coef_size > 3) ? 3 : self->coef_size * 1.5;
  }
  else if (wm->session->next_request_time < MID_REQUEST_TIME && wm->session->next_request_time >= MIN_REQUEST_TIME)
  {
    self->coef_size =
        (self->coef_size * 0.2 < 0.05) ? 0.05 : self->coef_size * 0.2;
  }

  float estimate_size_coef = self->coef_size;
  struct list* list_rq = list_create();
  list_rq->auto_free = true;
  unsigned int bandwidth = (u->bandwidth) ? (u->bandwidth) : wm->session->bandwidth;

  if (bandwidth < 10)
  {
    self->fps = 5;
  }

  float bw = ((float) (bandwidth)) / ((float) (self->fps));
  struct list* l_update = list_create();
  l_update->auto_free = true;
  bool q_params_min;
  int i;
  if (desktop->update_rects->count > 0)
  {
    l_update = desktop->update_rects;
  }
  if (desktop->candidate_update_rects->count > 0)
  {
    while (!fifo_is_empty(desktop->candidate_update_rects))
    {
      struct update_rect* cur = fifo_pop(desktop->candidate_update_rects);
      list_add_item(l_update, (tbus) cur);
    }
  }

  if (l_update->count > 0)
  {
    for (i = l_update->count - 1; i >= 0; i--)
    {
      struct update_rect * cur = (struct update_rect*) list_get_item(l_update, i);
      struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
      g_memcpy(tmp, cur, sizeof(struct update_rect));
      tmp->quality = 0;
      tmp->quality_already_send = -1;
      list_add_item(list_rq, (tbus) tmp);
      list_remove_item(l_update, i);
    }
    if (list_rq->count > 0)
    {
      estimate_size = estimate_size_coef * quality_params_estimate_progressive_display_size(self, list_rq) / 1024;
      q_params_min = quality_params_is_min(self, list_rq);
      int cpt = list_rq->count * (self->progressive_display_nb_level - 1);
      while (estimate_size > bw && !q_params_min && cpt-- > 0)
      {
        quality_params_decrease_quality(self, list_rq, desktop->video_regs);
        estimate_size = estimate_size_coef * quality_params_estimate_progressive_display_size(self, list_rq) / 1024;
        q_params_min = quality_params_is_min(self, list_rq);
      }

      if (estimate_size >= bw)
      {
        bool r_list = xrdp_screen_reduce_update_list(desktop, list_rq);
        estimate_size = estimate_size_coef * quality_params_estimate_size(self, list_rq, desktop->video_regs) / 1024;
        while (estimate_size >= bw && r_list)
        {
          r_list = xrdp_screen_reduce_update_list(desktop, list_rq);
          estimate_size = estimate_size_coef * quality_params_estimate_size(self, list_rq, desktop->video_regs) / 1024;
        }
      }
    }
  }
  progressive_display_add_update_order(desktop, list_rq, u->current_update_list);
  list_delete(list_rq);
}

float quality_params_select_regions(struct quality_params* self, struct xrdp_screen* screen, float bw, struct list* in, struct list* out)
{
  float estimate_size;
  int i;
  estimate_size = self->coef_size* quality_params_estimate_progressive_display_size(self, in) / 1024;
  if (estimate_size < bw)
  {
    // copy
    for (i = in->count - 1; i >= 0; i--)
    {
      struct update_rect * cur = (struct update_rect*) list_get_item(in, i);
      struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
      g_memcpy(tmp, cur, sizeof(struct update_rect));
      list_add_item(out, (tbus) tmp);
      list_remove_item(in, i);
    }
    list_clear(in);
  }
  else
  { // Reduce quality

    bool q_params_min = quality_params_is_min(self, in);//, desktop->video_regs);
    int cpt = in->count * (self->progressive_display_nb_level - 1);
    while (estimate_size > bw /*&& q_params_min */&& cpt-- > 0)
    {
      quality_params_decrease_quality_progressive_display(self, in);//, desktop->video_regs);
      estimate_size = self->coef_size* quality_params_estimate_progressive_display_size(self, in) / 1024;
      q_params_min = quality_params_is_min(self, in);//, desktop->video_regs);
    }
    if (estimate_size <= bw)
    {
      for (i = in->count - 1; i >= 0; i--)
      {
        struct update_rect * cur = (struct update_rect*) list_get_item(in, i);
        struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
        g_memcpy(tmp, cur, sizeof(struct update_rect));
        list_add_item(out, (tbus) tmp);
        list_remove_item(in, i);
      }
      list_clear(in);
    }
    else
    {
      // Reduce size of list
      bool r_list = xrdp_screen_reduce_update_list(screen, in);
      estimate_size = self->coef_size * quality_params_estimate_progressive_display_size(self, in) / 1024;
      while (estimate_size >= bw && r_list)
      {
        r_list = xrdp_screen_reduce_update_list(screen, in);
        estimate_size = self->coef_size* quality_params_estimate_progressive_display_size(self, in) / 1024;
      }
      // copy
      for (i = in->count - 1; i >= 0; i--)
      {
        struct update_rect * cur = (struct update_rect*) list_get_item(in, i);
        struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
        g_memcpy(tmp, cur, sizeof(struct update_rect));
        list_add_item(out, (tbus) tmp);
        list_remove_item(in, i);
      }
      list_clear(in);
    }
  }
  return estimate_size;
}

void quality_params_prepare_data3(struct quality_params* self, struct xrdp_screen* desktop, struct userChannel* u)
{
  int i;
  float estimate_size = 0;
  struct list* list_rq = list_create();
  list_rq->auto_free = true;
  struct list* list_final= list_create();
  list_final->auto_free = true;
  struct xrdp_wm* wm = (struct xrdp_wm*) u->wm;
  struct list* l_update = NULL;
  unsigned int bandwidth =
      (u->bandwidth) ? (u->bandwidth) : wm->session->bandwidth;

  float bw = ((float) (bandwidth)) / ((float) (self->fps));

  if (wm->session->next_request_time >= MAX_REQUEST_TIME)
  {
    self->coef_size = 3;
  }
  else if (wm->session->next_request_time >= MID_REQUEST_TIME && wm->session->next_request_time < MAX_REQUEST_TIME)
  {
    self->coef_size = (self->coef_size > 3) ? 3 : self->coef_size * 1.5;
  }
  else if (wm->session->next_request_time < MID_REQUEST_TIME && wm->session->next_request_time >= MIN_REQUEST_TIME)
  {
    self->coef_size =
        (self->coef_size * 0.2 < 0.05) ? 0.05 : self->coef_size * 0.2;
  }

  if (desktop->update_rects->count > 0)
  {
    l_update = desktop->update_rects;
  }
  else
  {
    l_update = list_create();
    l_update->auto_free = true;
  }

  if (desktop->candidate_update_rects->count > 0)
  {
    while (!fifo_is_empty(desktop->candidate_update_rects))
    {
      struct update_rect* cur = fifo_pop(desktop->candidate_update_rects);
      list_add_item(l_update, (tbus) cur);
    }
  }
  if (l_update->count > 0)
  {
    // Get Priority data (not already send)
    for (i = l_update->count - 1; i >= 0; i--)
    {
      struct update_rect * cur = (struct update_rect*) list_get_item(l_update, i);
      if (cur->quality_already_send == -1)
      {
        struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
        g_memcpy(tmp, cur, sizeof(struct update_rect));
        list_add_item(list_rq, (tbus) tmp);
        list_remove_item(l_update, i);
      }
    }

    if (list_rq->count > 0)
    {
      estimate_size = quality_params_select_regions(self, desktop, bw, list_rq, list_final);
    }
    else
    {
      // Get Priority data (not already send)
      for (i = l_update->count - 1; i >= 0; i--)
      {
        struct update_rect * cur = (struct update_rect*) list_get_item(l_update, i);
        if (cur->quality_already_send != -1)
        {
          struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
          g_memcpy(tmp, cur, sizeof(struct update_rect));
          list_add_item(list_rq, (tbus) tmp);
          list_remove_item(l_update, i);
        }
      }
      estimate_size = quality_params_select_regions(self, desktop, bw, list_rq, list_final);
    }
  }
  quality_params_add_update_order(desktop, list_final, u->current_update_list);

  if (self->is_video_detection_enable)
  {
    video_detection_add_update_order(desktop, u->current_update_list, u);
  }
  list_delete(list_rq);
  list_delete(list_final);
  if (l_update != desktop->update_rects)
  {
    list_delete(l_update);
  }
}

void quality_params_add_update_order(struct xrdp_screen* self, struct list* p_display, struct list* update_list)
{
  struct ip_image* desktop = self->screen;
  int i;
  update* up;
  int bpp = (self->bpp + 7) / 8;
  if (bpp == 3)
  {
    bpp = 4;
  }
  if (p_display->count > 0)
  {
    for (i = p_display->count - 1; i >= 0; i--)
    {
      struct update_rect* cur = (struct update_rect*) list_get_item(p_display, i);
      up = (update*) g_malloc(sizeof(update), 1);
      up->order_type = paint_rect;
      up->x = cur->rect.left;
      up->y = cur->rect.top;
      int w = rect_width(&cur->rect);
      int h = rect_height(&cur->rect);
      up->cx = w;
      up->cy = h;
      up->width = w;
      up->height = h;
      up->srcx = 0;
      up->srcy = 0;
      up->quality = cur->quality;
      up->data_len = up->cx * up->cy * bpp;
      up->data = g_malloc(up->data_len, 0);
      ip_image_crop(desktop, up->x, up->y, up->cx, up->cy, up->data);
      list_add_item(update_list, (tbus) up);
      if (cur->quality != 0)
      {
        struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
        g_memcpy(tmp, cur, sizeof(struct update_rect));
        tmp->quality = 0;
        tmp->quality_already_send = cur->quality;
        fifo_push(self->candidate_update_rects, tmp);
      }
      list_remove_item(p_display, i);
    }
  }
}

