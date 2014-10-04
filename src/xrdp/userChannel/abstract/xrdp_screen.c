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

#include "xrdp_screen.h"
#include "update_order.h"
#include "fifo.h"
#include "defines.h"
#include "funcs.h"
#include "os_calls.h"

struct xrdp_screen* xrdp_screen_create(int w, int h, int bpp, struct xrdp_client_info* c)
{
  struct xrdp_screen* self;
  self = (struct xrdp_screen*) g_malloc(sizeof(struct xrdp_screen), 1);
  self->width = w;
  self->height = h;
  self->bpp = bpp;
  self->screen = ip_image_create(w, h, bpp);
  self->update_rects = list_create();
  self->update_rects->auto_free = 1;
  self->client_info = c;
  if (c->use_video_detection)
  {
    self->candidate_video_regs = list_create();
    self->candidate_video_regs->auto_free = 1;
    self->video_regs = list_create();
    self->video_regs->auto_free = 1;
  }
  self->candidate_update_rects = fifo_new();
  return self;
}

void xrdp_screen_delete(struct xrdp_screen* self)
{
  if (self == 0)
    return;

  if (self->screen)
  {
    ip_image_delete(self->screen);
  }

  if (self->update_rects)
  {
    list_delete(self->update_rects);
  }
  if (self->client_info->use_video_detection)
  {
    if (self->candidate_video_regs)
    {
      list_delete(self->candidate_video_regs);
    }
    if (self->video_regs)
    {
      list_delete(self->video_regs);
    }
  }

  if (self->candidate_update_rects)
  {
    fifo_free(self->candidate_update_rects);
  }
  g_free(self);
}

void xrdp_screen_update_screen(struct xrdp_screen* self, int x, int y, int cx, int cy, char* data, int w, int h, int srcx, int srcy)
{
  ip_image_merge(self->screen, x, y, w, h, data);
}

void xrdp_screen_update_desktop(struct xrdp_screen* self)
{
  int i, j;
  struct list* update_rects = self->update_rects;
  struct update_rect* urect;
  struct update_rect* fu_rect;
  struct update_rect* ur;
  struct xrdp_rect intersection;
  struct update_rect* tmp;

  struct list* l_tmp = list_create();
  bool no_inter = true;
  while (!fifo_is_empty(self->candidate_update_rects))
  {
    fu_rect = (struct update_rect*) fifo_pop(self->candidate_update_rects);
    if (update_rects->count > 0)
    {
      no_inter = true;
      for (i = update_rects->count - 1; i >= 0; i--)
      {
        urect = (struct update_rect*) list_get_item(update_rects, i);
        if (!rect_equal(&urect->rect, &fu_rect->rect))
        {
          if (rect_intersect(&urect->rect, &fu_rect->rect, &intersection))
          {
            no_inter = false;
            update_rect_union(fu_rect, urect, l_tmp);
            list_remove_item(update_rects, i);
            for (j = 0; j < l_tmp->count; j++)
            {
              ur = (struct update_rect*) list_get_item(l_tmp, j);
              tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
              g_memcpy(tmp, ur, sizeof(struct update_rect));
              tmp->quality = 0;
              tmp->quality_already_send = -1;
              fifo_push(self->candidate_update_rects, tmp);
            }
            list_clear(l_tmp);
            break;
          }
        }
        else
        {
          no_inter = false;
          break;
        }
      }
      if (no_inter)
      {
        list_add_update_rect(update_rects, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
      }
    }
    else
    {
      list_add_update_rect(update_rects, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
    }
  }
  list_delete(l_tmp);
}

int list_add_update_rect(struct list* self, int left, int top, int right, int bottom, int quality, int send)
{
  struct update_rect* r;
  if (left != right && bottom != top)
  {
    r = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    r->quality = quality;
    r->quality_already_send = send;
    r->rect.left = left;
    r->rect.top = top;
    r->rect.right = right;
    r->rect.bottom = bottom;
    list_add_item(self, (tbus) r);
  }
  return 0;
}

int update_rect_union(struct update_rect* f1, struct update_rect* f2, struct list* out)
{
  struct xrdp_rect in1 = f1->rect;
  struct xrdp_rect in2 = f2->rect;
  struct xrdp_rect tmp;
  int l1 = f1->quality;
  int l2 = f2->quality;
  int send1 = f1->quality_already_send;
  int send2 = f2->quality_already_send;
  if (!rect_intersect(&in1, &in2, &tmp))
  {
    return 0;
  }
  if (rect_equal(&in1, &in2))
  {
    return 0;
  }

  int nsend;
  if (send1 == -1)
    nsend = -1;
  else if (send2 == -1)
    nsend = -1;
  else
    nsend = (send2 <= send1) ? send1 : send2;
  if (in1.left <= in2.left && in1.top <= in2.top && in1.right >= in2.right && in1.bottom >= in2.bottom)
  {
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in1.left, in2.top, in2.left, in2.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in2.top, in1.right, in2.bottom, l2, send1);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.right >= in2.right && in1.bottom < in2.bottom && in1.top <= in2.top)
  { /* partially covered(whole top) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in1.left, in2.top, in2.left, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in2.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.top <= in2.top && in1.bottom >= in2.bottom && in1.right < in2.right && in1.left <= in2.left)
  { /* partially covered(left) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in1.left, in2.top, in2.left, in2.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.right >= in2.right && in1.top > in2.top && in1.bottom >= in2.bottom)
  { /* partially covered(bottom) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.left, in2.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.top, in2.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in1.top, in1.right, in2.bottom, l1, send1);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.top <= in2.top && in1.bottom >= in2.bottom && in1.left > in2.left && in1.right >= in2.right)
  { /* partially covered(right) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.top, in2.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in2.top, in1.right, in2.bottom, l1, send1);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.top <= in2.top && in1.right < in2.right && in1.bottom < in2.bottom)
  { /* partially covered(top left) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in1.left, in2.top, in2.left, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left <= in2.left && in1.bottom >= in2.bottom && in1.right < in2.right && in1.top > in2.top)
  { /* partially covered(bottom left) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.left, in2.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.top, in1.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left > in2.left && in1.right >= in2.right && in1.top <= in2.top && in1.bottom < in2.bottom)
  { /* partially covered(top right) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.top, in2.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in2.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right >= in2.right && in1.top > in2.top && in1.bottom >= in2.bottom)
  { /* partially covered(bottom right) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in1.top, in1.left, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in1.top, in1.right, in2.bottom, l1, send1);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left > in2.left && in1.top <= in2.top && in1.right < in2.right && in1.bottom >= in2.bottom)
  { /* 2 rects, one on each end */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send2);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.top, in1.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.top > in2.top && in1.right >= in2.right && in1.bottom < in2.bottom)
  { /* 2 rects, one on each end */
    if (send1 == send2)
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.top, in2.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right < in2.right && in1.top <= in2.top && in1.bottom < in2.bottom)
  { /* partially covered(top) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_update_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.top, in1.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.top > in2.top && in1.bottom < in2.bottom && in1.left <= in2.left && in1.right < in2.right)
  { /* partially covered(left) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.top, in1.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in1.top, in2.right, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right < in2.right && in1.bottom >= in2.bottom && in1.top > in2.top)
  { /* partially covered(bottom) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in1.top, in1.left, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in1.right, in2.bottom, l2, nsend);
      list_add_update_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.top > in2.top && in1.bottom < in2.bottom && in1.right >= in2.right && in1.left > in2.left)
  { /* partially covered(right) */
    if (send1 == send2)
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_update_rect(out, in2.right, in1.top, in2.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in1.top, in1.left, in1.bottom, l2, send2);
      list_add_update_rect(out, in1.left, in1.top, in2.right, in1.bottom, l2, nsend);
      list_add_update_rect(out, in2.right, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send1);
    }
  }
  else if (in1.left > in2.left && in1.top > in2.top && in1.right < in2.right && in1.bottom < in2.bottom)
  { /* totally contained, 4 rects */
    if (send1 == send2)
    {
      list_add_update_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_update_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, nsend);
      list_add_update_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_update_rect(out, in2.left, in1.top, in1.left, in1.bottom, l2, send2);
      list_add_update_rect(out, in1.right, in1.top, in2.right, in1.bottom, l2, send2);
      list_add_update_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  return 0;
}

void xrdp_screen_add_update_orders(struct xrdp_screen* desktop, struct list* l)
{
  int i;
  update* up;
  int bpp = (desktop->bpp + 7) / 8;
  if (bpp == 3)
  {
    bpp = 4;
  }

  struct list* update_rects;
  update_rects = desktop->update_rects;

  for (i = update_rects->count - 1; i >= 0; i--)
  {
    struct update_rect* ucur = (struct update_rect*) list_get_item(update_rects, i);
    struct xrdp_rect cur = ucur->rect;
    up = g_malloc(sizeof(update), 1);
    up->order_type = paint_rect;
    up->x = cur.left;
    up->y = cur.top;
    int w = cur.right - cur.left;
    int h = cur.bottom - cur.top;
    up->cx = w;
    up->cy = h;
    up->width = w;
    up->height = h;
    up->srcx = 0;
    up->srcy = 0;
    up->data_len = up->cx * up->cy * bpp;
    up->data = g_malloc(up->data_len, 0);
    up->quality = 0;
    ip_image_crop(desktop->screen, up->x, up->y, up->cx, up->cy, up->data);
    list_add_item(l, (tbus) up);
    if (ucur->quality == 0)
      list_remove_item(update_rects, i);
  }
}

bool xrdp_screen_reduce_update_list(struct xrdp_screen* self, struct list* l)
{
  struct update_rect* cur;
  struct update_rect* tmp;
  int i;
  int count = l->count;
  if (l->count <= 1)
  {
    return xrdp_screen_reduce_regions(self, l);
  }
  for (i = l->count - 1; i >= count / 2; i--)
  {
    cur = (struct update_rect*) list_get_item(l, i);
    tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    g_memcpy(tmp, cur, sizeof(struct update_rect));
    fifo_push(self->candidate_update_rects, tmp);
    list_remove_item(l, i);
  }
  return true;
}

bool xrdp_screen_reduce_regions(struct xrdp_screen* self, struct list* l)
{
  struct update_rect* cur;
  struct update_rect* max;
  int i;
  int max_area = 0;
  int tmp_area;
  bool div = false;
  max = (struct update_rect*)list_get_item(l, 0);
  for (i = 1; i < l->count; i++)
  {
    cur = (struct update_rect*) list_get_item(l, i);
    tmp_area = rect_width(&cur->rect) * rect_height(&cur->rect);
    if (tmp_area > max_area)
    {
      max_area = tmp_area;
      max = cur;
    }
  }
  if (rect_height(&max->rect) > 64)
  {
    cur = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    g_memcpy(cur, max, sizeof(struct update_rect));
    cur->rect.top = max->rect.top + rect_height(&max->rect) / 4;
    fifo_push(self->candidate_update_rects, cur);
    max->rect.bottom = max->rect.top + rect_height(&max->rect) / 4;
    div = true;
  }
  else if (rect_width(&max->rect) > 64)
  {
    cur = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    g_memcpy(cur, max, sizeof(struct update_rect));
    cur->rect.left = max->rect.left + rect_width(&max->rect) / 2;
    fifo_push(self->candidate_update_rects, cur);
    max->rect.right = max->rect.left + rect_width(&max->rect) / 2;
    div = true;
  }

  return div;
}

bool xrdp_screen_reduce_rect(struct xrdp_screen* self, struct update_rect* urect, struct list* l)
{
  struct xrdp_rect rect = urect->rect;
  struct update_rect* cur;
  bool div = false;
  if (rect_height(&rect) > 64)
  {
    cur = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    g_memcpy(cur, urect, sizeof(struct update_rect));
    cur->rect.top = rect.top + rect_height(&rect) / 4;
    fifo_push(self->candidate_update_rects, cur);
    urect->rect.bottom =  urect->rect.top + rect_height(&rect) / 4;
    div = true;
  }
  else if (rect_width(&rect) > 64)
  {
    cur = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    g_memcpy(cur, urect, sizeof(struct update_rect));
    cur->rect.left = rect.left + rect_width(&rect) / 4;
    fifo_push(self->candidate_update_rects, cur);
    urect->rect.right =  urect->rect.left + rect_width(&rect) / 4;
    div = true;
  }
  return div;
}

