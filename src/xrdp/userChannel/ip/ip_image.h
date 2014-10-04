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

#ifndef _IP_IMAGE_H
#define _IP_IMAGE_H

struct ip_image
{
  int width;
  int height;
  int bpp;
  char* data;
};

struct ip_image* ip_image_create();
void ip_image_delete(struct ip_image* self);
void ip_image_merge(struct ip_image* self, int x, int y, int w, int h, char* data);
void ip_image_crop(struct ip_image* self, int x, int y, int cx, int cy, char* out);
int ip_image_mean(char* data, int x, int y, int x_size, int y_size, int w, int h, int bpp);
void ip_image_subsampling(char* data, int w, int h, int bpp, int scale, int quality, char* out);

#endif // _IP_IMAGE_H
