/**
 * Copyright (C) 2009 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com>
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


#ifndef XUTILS_H_
#define XUTILS_H_

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <stdio.h>

int
hex2int(const char* hexa_string);
Window
get_in_window(Display* display,  Window w);
int
get_window_name(Display* display, Window w, unsigned char** name);
int
get_window_type(Display* display, Window w, Atom* atom);
int
get_window_pid(Display* display, Window w, int* pid);
int
get_parent_window(Display* display, Window w, Window* parent);
int
is_good_window(Display* display,  Window w);
int
get_property( Display* display, Window w, const char* property, unsigned long *nitems, unsigned char** data);


#endif /* XUTILS_H_ */
