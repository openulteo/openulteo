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

#include <freerdp/utils/memory.h>
#include <freerdp/codec/bitmap.h>

#include "urdp_pointer.h"

/* Pointer Class */

void urdp_pointer_new(rdpContext* context, rdpPointer* pointer) {
	log_debug("urdp_pointer_new %dx%d -> %dx%d", pointer->width, pointer->height, pointer->xPos, pointer->yPos);

	/* Allocate data for image */
	uint8* data = (uint8*) malloc(pointer->width * pointer->height * 4);

	/* Convert to alpha cursor if mask data present */
	if (pointer->andMaskData && pointer->xorMaskData)
		freerdp_alpha_cursor_convert(data, pointer->xorMaskData, pointer->andMaskData, pointer->width, pointer->height, pointer->xorBpp, context->gdi->clrconv);

	((urdp_pointer*) pointer)->data_rgba = data;
}

void urdp_pointer_free(rdpContext* context, rdpPointer* pointer) {
	log_debug("urdp_pointer_free");
	free(((urdp_pointer*) pointer)->data_rgba);
}

void urdp_pointer_set(rdpContext* context, rdpPointer* pointer) {
	log_debug("urdp_pointer_set");
	if (((urdp_context*) context)->urdp_set_pointer)
		((urdp_context*) context)->urdp_set_pointer((urdp_context*) context, ((urdp_pointer*) pointer)->data_rgba, pointer->width, pointer->height, pointer->xPos, pointer->yPos);
}

void urdp_pointer_color(rdpContext* context, POINTER_COLOR_UPDATE* pointer_color) {
	rdpPointer* pointer;
	rdpCache* cache = context->cache;

	log_debug("urdp_pointer_color");

	pointer = Pointer_Alloc(context);

	if (pointer != NULL) {
		pointer->xorBpp = 24;
		pointer->xPos = pointer_color->xPos;
		pointer->yPos = pointer_color->yPos;
		pointer->width = pointer_color->width;
		pointer->height = pointer_color->height;
		pointer->lengthAndMask = pointer_color->lengthAndMask;
		pointer->lengthXorMask = pointer_color->lengthXorMask;
		pointer->xorMaskData = pointer_color->xorMaskData;
		pointer->andMaskData = pointer_color->andMaskData;

		pointer->New(context, pointer);
		pointer_cache_put(cache->pointer, pointer_color->cacheIndex, pointer);
		Pointer_Set(context, pointer);
	}
}

/* Graphics Module */

void urdp_register_pointer(urdp_context* context) {
	rdpPointer* pointer;

	log_debug("urdp_register_pointer");

	pointer = xnew(rdpPointer);
	pointer->size = sizeof(urdp_pointer);

	pointer->New = urdp_pointer_new;
	pointer->Free = urdp_pointer_free;
	pointer->Set = urdp_pointer_set;

	graphics_register_pointer(context->context.graphics, pointer);
	context->context.instance->update->pointer->PointerColor = urdp_pointer_color;
	xfree(pointer);
}
