/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Vincent Roullier <vincent.roullier@ulteo.com> 2012
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

#include "os_calls.h"
#include "libxrdp.h"

int APP_CC
xrdp_image_compress_jpeg(int width, int height, int bpp, unsigned char* data, int quality, char* dest) {
    struct stream *s;
    int bufsize;
    make_stream(s);
    init_stream(s, JPEG_BUFFER_SIZE);
    xrdp_bitmap_jpeg_compress(data, width, height, s, bpp, quality);
    bufsize = (int) (s->p - s->data);
    g_memcpy(dest, s->data, bufsize);
    free_stream(s);
    return bufsize;
}

int APP_CC
xrdp_image_compress_rle(int width, int height, int bpp, unsigned char* data, char* dest)
{
   struct stream *s;
   struct stream *tmp_s;
   int e;
   int i;
   int bufsize;
   make_stream(s);
   init_stream(s, IMAGE_TILE_MAX_BUFFER_SIZE);
   make_stream(tmp_s);
   init_stream(tmp_s, IMAGE_TILE_MAX_BUFFER_SIZE);
   e = width % 4;

   if (e != 0)
       e = 4 - e;

   i = height;
   xrdp_bitmap_compress(data, width, height, s, bpp, IMAGE_TILE_MAX_BUFFER_SIZE, i - 1, tmp_s, e);
   bufsize = (int) (s->p - s->data);
   g_memcpy(dest, s->data, bufsize);
   free_stream(tmp_s);
   free_stream(s);
   return bufsize;
}

void APP_CC
xrdp_image_compute_buffer_size(struct xrdp_client_info* self, int width,
                               int height, int bpp,
                               char* data, char* dest, int* dest_size, int* type)
{
    int jpeg_size;
    int rle_size;
    int raw_size;
    char* jpeg_buf = 0;
    char* rle_buf = 0;
    jpeg_size = JPEG_BUFFER_SIZE;
    rle_size = IMAGE_TILE_MAX_BUFFER_SIZE;
    raw_size = width*height*((bpp+7)/8);

    if (self->use_jpeg == 1)
    {
        jpeg_buf = (char*) g_malloc(jpeg_size, 0);
        jpeg_size = xrdp_image_compress_jpeg(width, height, bpp, data, self->jpeg_quality, jpeg_buf);
    }
    if (self->use_bitmap_comp)
    {
        rle_buf = (char*) g_malloc(rle_size,0);
        rle_size = xrdp_image_compress_rle(width, height, bpp, data, rle_buf);
    }

    if (jpeg_size < rle_size)
    {
       if (jpeg_size < raw_size)
       {
           *type = JPEG_TILE;
           g_memcpy(dest, jpeg_buf, jpeg_size);
           *dest_size = jpeg_size;
       }
       else
       {
           *type = RAW_TILE;
           g_memcpy(dest, data, raw_size);
           *dest_size = raw_size;
       }
    }
    else
    {
        if (rle_size < raw_size)
        {
           *type = RLE_TILE;
           g_memcpy(dest, rle_buf, rle_size);
           *dest_size = rle_size;
        }
        else
        {
           *type = RAW_TILE;
           g_memcpy(dest, data, raw_size);
           *dest_size = raw_size;
        }
    }
    g_free(jpeg_buf);
    g_free(rle_buf);
}
