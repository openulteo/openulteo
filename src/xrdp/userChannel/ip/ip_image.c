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

#include "ip_image.h"
#include "defines.h"
#include "os_calls.h"

struct ip_image* ip_image_create(int width, int height, int bpp)
{
  struct ip_image* ip;
  ip = (struct ip_image*) g_malloc(sizeof(struct ip_image), 1);
  ip->width = width;
  ip->height = height;
  ip->bpp = bpp;
  int Bpp = (bpp + 7) / 8;
  if (Bpp == 3)
  {
    Bpp = 4;
  }
  ip->data = (char*) g_malloc(width * height * Bpp, 1);
  return ip;
}

void ip_image_delete(struct ip_image* self)
{
  if (self->data)
    g_free(self->data);

  g_free(self);
}

void ip_image_merge(struct ip_image* self, int x, int y, int w, int h, char* data)
{
  int i, j, ii, jj, pixel;
  int width = ((x + w) > self->width) ? self->width : x + w;
  int height = ((y + h) > self->height) ? self->height : y + h;

  for (j = y, jj = 0; j < height; j++, jj++)
  {
    for (i = x, ii = 0; i < width; i++, ii++)
    {
      if (self->bpp == 8)
      {
        pixel = GETPIXEL8(data, ii, jj, w);
        SETPIXEL8(self->data, i, j, self->width, pixel);
      }
      else if (self->bpp == 15)
      {
        pixel = GETPIXEL16(data, ii, jj, w);
        SETPIXEL16(self->data, i, j, self->width, pixel);
      }
      else if (self->bpp == 16)
      {
        pixel = GETPIXEL16(data, ii, jj, w);
        SETPIXEL16(self->data, i, j, self->width, pixel);
      }
      else if (self->bpp == 24)
      {
        pixel = GETPIXEL32(data, ii, jj, w);
        SETPIXEL32(self->data, i, j, self->width, pixel);
      }
    }
  }
}

void ip_image_crop(struct ip_image* self, int x, int y, int cx, int cy, char* out)
{
  int i, j;
  for (j = 0; j < cy; j++)
  {
    for (i = 0; i < cx; i++)
    {
      if (self->bpp == 8)
      {
        SETPIXEL8(out, i, j, cx, GETPIXEL8(self->data, x+i, y+j, self->width));
      }
      else if (self->bpp == 15)
      {
        SETPIXEL16(out, i, j, cx, GETPIXEL16(self->data, x+i, y+j, self->width));
      }
      else if (self->bpp == 16)
      {
        SETPIXEL16(out, i, j, cx, GETPIXEL16(self->data, x+i, y+j, self->width));
      }
      else if (self->bpp == 24)
      {
        SETPIXEL32(out, i, j, cx, GETPIXEL32(self->data, x+i, y+j, self->width));
      }
    }
  }
}

int ip_image_mean(char* data, int x, int y, int x_size, int y_size, int w, int h, int bpp)
{
  int i, j, r, g, b, pixel;
  int sumr = 0;
  int sumg = 0;
  int sumb = 0;
  int res;
  int width = ((x + x_size) > w) ? w : x + x_size;
  int height = ((y + y_size) > h) ? h : y + y_size;
  for (j = y; j < height; j++)
  {
    for (i = x; i < width; i++)
    {
      if (bpp == 8)
      {
        pixel = GETPIXEL8(data, i, j, w);
        r = pixel << 5;
        g = (pixel >> 3) << 5;
        b = (pixel >> 6) << 6;
      }
      else if (bpp == 15)
      {
        pixel = GETPIXEL16(data, i, j, w);
        SPLITCOLOR15(r, g, b, pixel);
      }
      else if (bpp == 16)
      {
        pixel = GETPIXEL16(data, i, j, w);
        SPLITCOLOR16(r, g, b, pixel);
      }
      else if (bpp == 24 || bpp == 32)
      {
        pixel = GETPIXEL32(data, i, j, w);
        SPLITCOLOR32(r, g, b, pixel);
      }
      sumr += r;
      sumg += g;
      sumb += b;
    }
  }
  sumr /= (x_size * y_size);
  sumg /= (x_size * y_size);
  sumb /= (x_size * y_size);
  if (bpp == 8)
  {
    res = COLOR8(sumr, sumg, sumb);
  }
  else if (bpp == 15)
  {
    res = COLOR15(sumr, sumg, sumb);
  }
  else if (bpp == 16)
  {
    res = COLOR16(sumr, sumg, sumb);
  }
  else if (bpp == 24 || bpp == 32)
  {
    res = COLOR24RGB(sumr, sumg, sumb);
  }
  return res;
}

void ip_image_subsampling(char* data, int w, int h, int bpp, int quality, int scale, char* out)
{
  int i, j;
  int ii, jj;
  int cy, cx;

  int dim = scale << quality;
  int size_y = (dim > h) ? h : dim;
  int size_x = (dim > w) ? w : dim;
  int mean;

  for (j = 0; j < h; j += size_y)
  {
    cy = (j + size_y >= h) ? h - j : size_y;
    for (i = 0; i < w; i += size_x)
    {
      cx = (i + size_x >= w) ? w - i : size_x;
      mean = ip_image_mean(data, i, j, cx, cy, w, h, bpp);
      for (jj = j; jj < j + cy; jj++)
      {
        for (ii = i; ii < i + cx; ii++)
        {
          switch (bpp)
          {
          case 8:
            SETPIXEL8(out, ii, jj, w, mean);
            break;
          case 15:
            SETPIXEL16(out, ii, jj, w, mean);
            break;
          case 16:
            SETPIXEL16(out, ii, jj, w, mean);
            break;
          case 24:
            SETPIXEL32(out, ii, jj, w, mean);
            break;
          }
        }
      }
    }
  }
}
