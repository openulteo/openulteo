/**
 * Copyright (C) 2011 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011
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

#include "Clipboard.h"

void
clipboard_init(Clipboard* clip, int format_count)
{
	clip->format_count = format_count;

	clip->clipboard_data = g_malloc(format_count * sizeof(unsigned char*), 1);
	clip->clipboard_data_size = g_malloc(format_count * sizeof(int), 1);

	clip->current_clipboard_formats = list_create();
	clip->current_clipboard_formats->auto_free = 0;

	clip->supported_formats = list_create();
	clip->supported_formats->auto_free = 0;
}

void
clipboard_release(Clipboard* clip)
{
	clipboard_current_clipboard_clear(clip);
	g_free(clip->clipboard_data);
	g_free(clip->clipboard_data_size);

	list_delete(clip->current_clipboard_formats);
	list_clear(clip->supported_formats);
	list_delete(clip->supported_formats);
}

void
clipboard_add_format_support(Clipboard* clip, Atom format)
{
	list_add_item(clip->supported_formats, format);
}

void
clipboard_current_clipboard_clear(Clipboard* clip) {
	int i = 0;
	list_clear(clip->current_clipboard_formats);

	for(i = 0 ; i < clip->format_count ; i++)
	{
		g_free(clip->clipboard_data[i]);

		clip->clipboard_data[i] = NULL;
		clip->clipboard_data_size[i] = 0;
	}
}

unsigned char*
clipboard_get_current_clipboard_data(Clipboard* clip, Atom format)
{
	int index = list_index_of(clip->current_clipboard_formats, (tbus)format);
	if (index == -1)
		return NULL;

	return clip->clipboard_data[index];
}

int
clipboard_get_current_clipboard_data_size(Clipboard* clip, Atom format)
{
	int index = list_index_of(clip->current_clipboard_formats, (tbus)format);
	if (index == -1)
		return -1;

	return clip->clipboard_data_size[index];
}

int
clipboard_get_current_clipboard_format_index(Clipboard* clip, Atom format)
{
	return list_index_of(clip->current_clipboard_formats, (tbus)format);
}

Bool
clipboard_current_clipboard_format_exist(Clipboard* clip, Atom format)
{
	return list_index_of(clip->current_clipboard_formats, (tbus)format) != -1;
}

Bool
clipboard_format_supported(Clipboard* clip, Atom format)
{
	return list_index_of(clip->supported_formats, (tbus)format) != -1;
}

void
clipboard_add_current_clipboard_data(Clipboard* clip, unsigned char* data, int data_size, Atom format)
{
	int index = list_index_of(clip->current_clipboard_formats, (tbus)format);
	if (index == -1 || index > clip->format_count)
		return;

	clip->clipboard_data[index] = data;
	clip->clipboard_data_size[index] = data_size;
}

void
clipboard_add_current_clipboard_format(Clipboard* clip, Atom format)
{
	list_add_item(clip->current_clipboard_formats, format);
}

int
clipboard_current_clipboard_size(Clipboard* clip)
{
	return clip->current_clipboard_formats->count;
}

Atom
clipboard_get_current_clipboard_format(Clipboard* clip, int index)
{
	return list_get_item(clip->current_clipboard_formats, index);
}

