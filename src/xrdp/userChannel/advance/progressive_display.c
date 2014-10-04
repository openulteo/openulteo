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

#include <stdio.h>

#include "progressive_display.h"
#include "fifo.h"
#include "list.h"
#include "defines.h"
#include "funcs.h"
#include "ip_image.h"
#include "video_detection.h"
#include "update_order.h"

extern int count;
void fifo_rect_progressive_display(struct fifo* self)
{
  struct update_rect* cur;
  struct xrdp_rect rect;
  printf("Fifo : %i elements : \n", self->count);
  struct fifo_item* head = self->head;
  while (head != NULL )
  {
    cur = (struct update_rect*) head->data;
    rect = cur->rect;
    printf("  rect : [%i %i %i %i] [%i, %i], level %i \n", rect.left, rect.top, rect.right, rect.bottom, rect_width(&rect), rect_height(&rect), cur->quality);
    head = head->next;
  }
}

void list_rect_progressive_display(struct list* self)
{
  int i;
  struct update_rect* cur;
  struct xrdp_rect rect;
  printf("List : %i elements : \n", self->count);
  for (i = 0; i < self->count; i++)
  {
    cur = (struct update_rect*) list_get_item(self, i);
    rect = cur->rect;
    printf("  rect : [%i %i %i %i] [%i, %i], level %i \n", rect.left, rect.top, rect.right, rect.bottom, rect_width(&rect), rect_height(&rect), cur->quality);
  }
}

void progressive_display_add_rect(struct xrdp_screen * self)
{
  int i, j;
  struct list* update_rects = self->update_rects;
  struct update_rect* urect;
  struct update_rect* fu_rect;
  struct update_rect* ur;
  struct xrdp_rect intersection;
  struct update_rect* tmp;
  struct list* l_tmp = list_create();
  l_tmp->auto_free = 1;

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
            progressive_display_rect_union(fu_rect, urect, l_tmp);
            list_remove_item(update_rects, i);
            for (j = 0; j < l_tmp->count; j++)
            {
              ur = (struct update_rect*) list_get_item(l_tmp, j);
              tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
              g_memcpy(tmp, ur, sizeof(struct update_rect));
              fifo_push(self->candidate_update_rects, tmp);
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
        list_add_progressive_display_rect(update_rects, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
      }
    }
    else
    {
      list_add_progressive_display_rect(update_rects, fu_rect->rect.left, fu_rect->rect.top, fu_rect->rect.right, fu_rect->rect.bottom, fu_rect->quality, fu_rect->quality_already_send);
    }
  }
  list_delete(l_tmp);
}

void progressive_display_add_update_order(struct xrdp_screen* self, struct list* p_display, struct list* update_list)
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

void progressive_display_update_level(struct xrdp_screen* self, long *t0)
{
  struct list* p_display = self->update_rects;
  int i;
  if ((g_time3() - *t0) > 1000)
  {
    for (i = 0; i < p_display->count; i++)
    {
      struct update_rect* cur = (struct update_rect*) list_get_item(p_display, i);
      if (cur->quality > 0)
      {
        cur->quality--;
      }
    }
    *t0 = g_time3();
  }
}

int list_add_progressive_display_rect(struct list* self, int left, int top, int right, int bottom, int quality, int send)
{
  if (left != right && top != bottom)
  {
    struct update_rect* tmp = (struct update_rect*) g_malloc(sizeof(struct update_rect), 0);
    tmp->quality = quality;
    tmp->quality_already_send = send;
    tmp->rect.top = top;
    tmp->rect.left = left;
    tmp->rect.right = right;
    tmp->rect.bottom = bottom;
    list_add_item(self, (tbus) tmp);
  }
  return 1;
}

void progressive_display_split_and_merge(struct list* self)
{
  int i, j;
  struct update_rect* item1;
  struct update_rect* item2;
  struct xrdp_rect first;
  struct xrdp_rect second;
  bool merged = true;
  bool remove;

  while (merged)
  {
    merged = false;
    for (i = 0; i < self->count; i++)
    {
      item1 = (struct update_rect*) list_get_item(self, i);
      first = item1->rect;
      for (j = self->count - 1; j > i; j--)
      {
        item2 = (struct update_rect*) list_get_item(self, j);
        second = item2->rect;
        if (item1->quality_already_send == item2->quality_already_send)
        {
          if (!rect_equal(&first, &second))
          {
            int adj = rect_adjacency(&first, &second);
            switch (adj)
            {
            case 0:
              break;
            case 1: // first is at right of second
              merged = true;
              if (rect_height(&first) == rect_height(&second))
              {
                item1->rect.left = item2->rect.left;
                list_remove_item(self, j);
              }
              else
              {
                progressive_display_rect_split_h(self, item1, item2, &remove);
                if (remove)
                {
                  list_remove_item(self, j);
                  list_remove_item(self, i);
                }
                else
                  merged = false;
              }
              break;
            case 2: // first is at left of second
              merged = true;
              if (rect_height(&first) == rect_height(&second))
              {
                item1->rect.right = item2->rect.right;
                list_remove_item(self, j);
              }
              else
              {
                progressive_display_rect_split_h(self, item2, item1, &remove);
                if (remove)
                {
                  list_remove_item(self, j);
                  list_remove_item(self, i);
                }
                else
                  merged = false;
              }
              break;
            case 3: // first is under second
              merged = true;
              if (rect_width(&first) == rect_width(&second))
              {
                item1->rect.top = item2->rect.top;
                list_remove_item(self, j);
              }
              else
              {
                progressive_display_rect_split_v(self, item1, item2, &remove);
                if (remove)
                {
                  list_remove_item(self, j);
                  list_remove_item(self, i);
                }
                else
                  merged = false;
              }
              break;
            case 4: // first is above second
              merged = true;
              if (rect_width(&first) == rect_width(&second))
              {
                item1->rect.bottom = item2->rect.bottom;
                list_remove_item(self, j);
              }
              else
              {
                progressive_display_rect_split_v(self, item2, item1, &remove);
                if (remove)
                {
                  list_remove_item(self, j);
                  list_remove_item(self, i);
                }
                else
                  merged = false;
              }
              break;
            }
          }
          else
          {
            list_remove_item(self, j);
            merged = true;
          }
        }
        else
        {
          merged = false;
        }
        if (merged)
          break;
      }
      if (merged)
        break;
    }
  }
}

void progressive_display_rect_split_v(struct list* self, struct update_rect* f, struct update_rect* s, bool* remove)
{
  struct xrdp_rect first = f->rect;
  struct xrdp_rect second = s->rect;
  int l = f->quality;
  int w1 = rect_width(&first);
  int h1 = rect_height(&first);
  int w2 = rect_width(&second);
  int h2 = rect_height(&second);
  int area1 = w1 * h1;
  int area2 = w2 * h2;
  int send1 = f->quality_already_send;
  int send2 = s->quality_already_send;
  *remove = false;
  if (first.left < second.left)
  {
    if (first.right < second.right)
    {
      int temp_area = (second.left - first.right) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, second.left, first.bottom, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, first.right, first.bottom, l, send2);
        list_add_progressive_display_rect(self, first.right, second.top, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.right > second.right)
    {
      int temp_area = (second.right - second.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, second.left, first.bottom, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.bottom, l, send2);
        list_add_progressive_display_rect(self, second.right, first.top, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
    else
    {
      int temp_area = (second.right - second.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, second.left, first.bottom, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.bottom, l, send2);
        *remove = true;
      }
    }
  }
  else if (first.left > second.left)
  {
    if (first.right < second.right)
    {
      int temp_area = (first.right - first.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, first.left, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.top, first.right, first.bottom, l, send1);
        list_add_progressive_display_rect(self, first.right, second.top, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.right > second.right)
    {
      int temp_area = (second.right - first.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, first.left, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.top, second.right, first.bottom, l, send2);
        list_add_progressive_display_rect(self, second.right, first.top, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
    else
    {
      int temp_area = (second.right - first.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, first.left, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.top, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
  }
  else
  {
    if (first.right < second.right)
    {
      int temp_area = (first.right - first.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, second.top, first.right, first.bottom, l, send1);
        list_add_progressive_display_rect(self, first.right, second.top, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.right > second.right)
    {
      int temp_area = (second.right - first.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.bottom, l, send2);
        list_add_progressive_display_rect(self, second.right, first.top, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
  }
}

void progressive_display_rect_split_h(struct list* self, struct update_rect* f, struct update_rect* s, bool* remove)
{
  struct xrdp_rect first = f->rect;
  struct xrdp_rect second = s->rect;
  int l = f->quality;
  int w1 = rect_width(&first);
  int h1 = rect_height(&first);
  int w2 = rect_width(&second);
  int h2 = rect_height(&second);
  int send1 = f->quality_already_send;
  int send2 = s->quality_already_send;
  int area1 = w1 * h1;
  int area2 = w2 * h2;
  *remove = false;
  if (first.top < second.top)
  {
    if (first.bottom < second.bottom)
    {
      int temp_area = (first.right - second.left) * (first.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, first.right, second.top, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, first.right, first.bottom, l, send2);
        list_add_progressive_display_rect(self, second.left, first.bottom, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.bottom > second.bottom)
    {
      int temp_area = (first.right - second.left) * (second.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, first.right, second.top, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, first.right, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.bottom, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
    else
    {
      int temp_area = (first.right - second.left) * (second.bottom - second.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, first.left, first.top, first.right, second.top, l, send1);
        list_add_progressive_display_rect(self, second.left, second.top, first.right, second.bottom, l, send2);
        *remove = true;
      }
    }
  }
  else if (first.top > second.top)
  {
    if (first.bottom < second.bottom)
    {
      int temp_area = (first.right - second.left) * (first.bottom - first.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.top, l, send2);
        list_add_progressive_display_rect(self, second.left, first.top, first.right, first.bottom, l, send1);
        list_add_progressive_display_rect(self, second.left, first.bottom, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.bottom > second.bottom)
    {
      int temp_area = (first.right - second.left) * (second.bottom - first.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.top, l, send2);
        list_add_progressive_display_rect(self, second.left, first.top, first.right, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.bottom, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
    else
    {
      int temp_area = (first.right - second.left) * (second.bottom - first.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, second.right, first.top, l, send2);
        list_add_progressive_display_rect(self, second.left, first.top, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
  }
  else
  {
    if (first.bottom < second.bottom)
    {
      int temp_area = (first.right - second.left) * (first.bottom - first.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, first.top, first.right, first.bottom, l, send1);
        list_add_progressive_display_rect(self, second.left, first.bottom, second.right, second.bottom, l, send2);
        *remove = true;
      }
    }
    else if (first.bottom > second.bottom)
    {
      int temp_area = (first.right - second.left) * (second.bottom - first.top);
      if (temp_area > area1 && temp_area > area2)
      {
        list_add_progressive_display_rect(self, second.left, second.top, first.right, second.bottom, l, send2);
        list_add_progressive_display_rect(self, first.left, second.bottom, first.right, first.bottom, l, send1);
        *remove = true;
      }
    }
  }
}

void progressive_display_rect_union(struct update_rect* r1, struct update_rect* r2, struct list* out)
{
  struct xrdp_rect in1 = r1->rect;
  struct xrdp_rect in2 = r2->rect;
  int l1 = r1->quality;
  int l2 = r2->quality;
  int send1 = r1->quality_already_send;
  int send2 = r2->quality_already_send;
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
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.left, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in2.top, in1.right, in2.bottom, l2, send1);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.right >= in2.right && in1.bottom < in2.bottom && in1.top <= in2.top)
  { /* partially covered(whole top) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.left, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in2.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.top <= in2.top && in1.bottom >= in2.bottom && in1.right < in2.right && in1.left <= in2.left)
  { /* partially covered(left) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.left, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.right >= in2.right && in1.top > in2.top && in1.bottom >= in2.bottom)
  { /* partially covered(bottom) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.left, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.top, in2.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in1.top, in1.right, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.top <= in2.top && in1.bottom >= in2.bottom && in1.left > in2.left && in1.right >= in2.right)
  { /* partially covered(right) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in2.top, in1.right, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.top <= in2.top && in1.right < in2.right && in1.bottom < in2.bottom)
  { /* partially covered(top left) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.left, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left <= in2.left && in1.bottom >= in2.bottom && in1.right < in2.right && in1.top > in2.top)
  { /* partially covered(bottom left) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.left, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left > in2.left && in1.right >= in2.right && in1.top <= in2.top && in1.bottom < in2.bottom)
  { /* partially covered(top right) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.top, in2.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in2.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right >= in2.right && in1.top > in2.top && in1.bottom >= in2.bottom)
  { /* partially covered(bottom right) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.left, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.left, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in1.top, in1.right, in2.bottom, l1, send1);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left > in2.left && in1.top <= in2.top && in1.right < in2.right && in1.bottom >= in2.bottom)
  { /* 2 rects, one on each end */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send2);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.top, in1.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.left <= in2.left && in1.top > in2.top && in1.right >= in2.right && in1.bottom < in2.bottom)
  { /* 2 rects, one on each end */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.top, in2.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right < in2.right && in1.top <= in2.top && in1.bottom < in2.bottom)
  { /* partially covered(top) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.top, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in2.top, in1.left, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.top, in1.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in2.top, in2.right, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.top > in2.top && in1.bottom < in2.bottom && in1.left <= in2.left && in1.right < in2.right)
  { /* partially covered(left) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.left, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in1.top, in2.right, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
  else if (in1.left > in2.left && in1.right < in2.right && in1.bottom >= in2.bottom && in1.top > in2.top)
  { /* partially covered(bottom) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.left, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in2.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in1.right, in1.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in2.bottom, in1.right, in1.bottom, l1, send1);
    }
  }
  else if (in1.top > in2.top && in1.bottom < in2.bottom && in1.right >= in2.right && in1.left > in2.left)
  { /* partially covered(right) */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.right, in1.top, in1.right, in1.bottom, l1, send1);
    }
    else
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.left, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.left, in1.top, in2.right, in1.bottom, l2, nsend);
      list_add_progressive_display_rect(out, in2.right, in1.top, in1.right, in1.bottom, l1, send1);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send1);
    }
  }
  else if (in1.left > in2.left && in1.top > in2.top && in1.right < in2.right && in1.bottom < in2.bottom)
  { /* totally contained, 4 rects */
    if (send1 == send2)
    {
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in2.bottom, l2, send2);
    }
    else
    {
      list_add_progressive_display_rect(out, in1.left, in1.top, in1.right, in1.bottom, l1, nsend);
      list_add_progressive_display_rect(out, in2.left, in2.top, in2.right, in1.top, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.top, in1.left, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in1.right, in1.top, in2.right, in1.bottom, l2, send2);
      list_add_progressive_display_rect(out, in2.left, in1.bottom, in2.right, in2.bottom, l2, send2);
    }
  }
}
