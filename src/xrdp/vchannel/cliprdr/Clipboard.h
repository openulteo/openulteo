/**
 * Copyright (C) 2011-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011, 2012
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

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include <list.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <os_calls.h>


typedef struct _Clipboard
{
	int format_count;
	struct list* current_clipboard_formats;
	struct list* supported_formats;

	unsigned char** clipboard_data;
	int* clipboard_data_size;

} Clipboard;


void clipboard_init(Clipboard* clip, int format_count);
void clipboard_release(Clipboard* clip);
void clipboard_add_format_support(Clipboard* clip, Atom format);
void clipboard_current_clipboard_clear(Clipboard* clip);

unsigned char* clipboard_get_current_clipboard_data(Clipboard* clip, Atom format);
int clipboard_get_current_clipboard_data_size(Clipboard* clip, Atom format);
int clipboard_get_current_clipboard_format_index(Clipboard* clip, Atom format);
Atom clipboard_get_current_clipboard_format(Clipboard* clip, int index);

void clipboard_add_current_clipboard_data(Clipboard* clip, unsigned char* data, int data_size, Atom format);
void clipboard_add_current_clipboard_format(Clipboard* clip, Atom format);
int clipboard_current_clipboard_size(Clipboard* clip);

Bool clipboard_current_clipboard_format_exist(Clipboard* clip, Atom format);
Bool clipboard_format_supported(Clipboard* clip, Atom format);

void send_dummy_event();

#endif /* CLIPBOARD_H_ */
