/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>

#include <freerdp/freerdp.h>
#include <freerdp/utils/stream.h>
#include <freerdp/utils/memory.h>
#include <freerdp/codec/color.h>

#include "urdp_jpeg.h"

/* Called by the JPEG library upon encountering a fatal error */
static void fatal_jpeg_error(j_common_ptr cinfo) {
	jpeg_destroy(cinfo);
}


typedef struct tjpeg_source_mgr {
	struct jpeg_source_mgr pub;
	uint8 *buffer;
	int size;
	int pointer;
} tjpeg_source_mgr;


static void tjpeg_init_source(j_decompress_ptr cinfo)
{
	tjpeg_source_mgr *src = (tjpeg_source_mgr*)cinfo->src;
	src->pointer = 0;
}


static int tjpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	tjpeg_source_mgr *src = (tjpeg_source_mgr*)cinfo->src;

	if (src->size - src->pointer <= 0)
	{
		static unsigned char jpeg_eof[] = { 0xFF, JPEG_EOI};

		if (src->pointer==0)
		{
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		}
		WARNMS(cinfo, JWRN_JPEG_EOF);
		src->pub.next_input_byte = jpeg_eof;
		src->pub.bytes_in_buffer = sizeof(jpeg_eof);
		return TRUE;
	}

	src->pub.next_input_byte = src->buffer + src->pointer;
	src->pub.bytes_in_buffer = src->size - src->pointer;
	src->pointer += src->size;

	return TRUE;
}


static void tjpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  tjpeg_source_mgr *src = (tjpeg_source_mgr*)cinfo->src;
  if (num_bytes > 0)
  {
    src->pub.next_input_byte += (size_t)num_bytes;
    src->pub.bytes_in_buffer -= (size_t)num_bytes;
    src->pointer += num_bytes;
  }
}


static void tjpeg_term_source(j_decompress_ptr cinfo)
{

}


void jpeg_decompress(void* jpeg_data, uint16 bufsize, void* pixels)
{
	static struct jpeg_decompress_struct cinfo;
	static struct jpeg_error_mgr jerr;
	JSAMPROW rowptr[1];
	unsigned int i;
	int retval;

	memset(&cinfo, 0, sizeof(cinfo));
	memset(&jerr, 0, sizeof(jerr));

	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = fatal_jpeg_error;
	jpeg_create_decompress(&cinfo);

	tjpeg_source_mgr *src;
	if (cinfo.src == NULL) {
		/* first time for this JPEG object? */
		cinfo.src = (struct jpeg_source_mgr*)(*cinfo.mem->alloc_small)((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(tjpeg_source_mgr));
		src = (tjpeg_source_mgr*)cinfo.src;
	}
	src = (tjpeg_source_mgr*)cinfo.src;
	src->buffer = jpeg_data;
	src->size = bufsize;
	src->pub.init_source = tjpeg_init_source;
	src->pub.fill_input_buffer = tjpeg_fill_input_buffer;
	src->pub.skip_input_data = tjpeg_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = tjpeg_term_source;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */

	jpeg_save_markers(&cinfo, JPEG_APP0 + 14, 256);

	retval = jpeg_read_header(&cinfo, TRUE);
	if (retval != JPEG_HEADER_OK) {
		return;
	}

#ifdef ANDROID_RGB
	cinfo.out_color_space = JCS_RGB_565;
#else
	cinfo.out_color_space = JCS_RGB;
#endif

	if (jpeg_start_decompress(&cinfo) != TRUE) {
		return;
	}

#ifdef ANDROID_RGB
	if (cinfo.out_color_space != JCS_RGB_565 || cinfo.output_components != 3) {
		return;
	}

	int rowbytes = cinfo.output_width * 2;
#else
	if (cinfo.out_color_space != JCS_RGB || cinfo.output_components != 3) {
		return;
	}

	int rowbytes = cinfo.output_width * 3;
#endif
	for (i = 0; i < cinfo.output_height; ++i) {
		rowptr[0] = pixels + i * rowbytes;
		JDIMENSION nrows = jpeg_read_scanlines(&cinfo, rowptr, 1);
		if (nrows != 1) {
			return;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}


void update_gdi_cache_jpeg(rdpContext* context, CACHE_JPEG_ORDER* cache_jpeg)
{
	rdpBitmap* bitmap;
	rdpBitmap* prevBitmap;
	rdpCache* cache = context->cache;

	bitmap = Bitmap_Alloc(context);
	Bitmap_SetDimensions(context, bitmap, cache_jpeg->bitmapWidth, cache_jpeg->bitmapHeight);
	bitmap->compressed = false;
#ifdef ANDROID_RGB
	bitmap->length = cache_jpeg->bitmapWidth * cache_jpeg->bitmapHeight * 2;
#else
	bitmap->length = cache_jpeg->bitmapWidth * cache_jpeg->bitmapHeight * 3;
#endif
	bitmap->bpp = cache_jpeg->bitmapBpp;
	bitmap->data = xmalloc(bitmap->length);

	jpeg_decompress(cache_jpeg->bitmapDataStream, cache_jpeg->bitmapLength, bitmap->data);

#ifndef ANDROID_RGB
	if (cache_jpeg->bitmapBpp == 2) {
		int i, j;
		for ( i = 0, j = 0; i < bitmap->length ; i += 3, j += 2) {
			*((uint16*)(&(bitmap->data)[j])) = (uint16)(RGB16(bitmap->data[i], bitmap->data[i+1], bitmap->data[i+2]));
		}
		bitmap->length = j;
		bitmap->data = realloc(bitmap->data, bitmap->length);
	}
#endif

	bitmap->New(context, bitmap);

	prevBitmap = bitmap_cache_get(cache->bitmap, cache_jpeg->cacheId, cache_jpeg->cacheIndex);
	if (prevBitmap != NULL)
		Bitmap_Free(context, prevBitmap);

	bitmap_cache_put(cache->bitmap, cache_jpeg->cacheId, cache_jpeg->cacheIndex, bitmap);
}

void jpeg_cache_register_callbacks(rdpUpdate* update)
{
	update->secondary->CacheJpeg = update_gdi_cache_jpeg;
}
