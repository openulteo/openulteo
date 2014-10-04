/**
 * Copyright (C) 2009-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2009-2011
 * Author Thomas MOUTON <thomas@ulteo.com> 2010-2012
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

#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MOTIF_WM_HINTS_ELEMENTS    5
typedef struct
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
}
PropMotifWmHints;

/* Xlib constant  */
#define _NET_WM_STATE_REMOVE		0	/* remove/unset property */
#define _NET_WM_STATE_ADD		1	/* add/set property */
#define _NET_WM_STATE_TOGGLE		2	/* toggle property  */

#define REQUEST_FROM_APPLICATION	1	/* normal applications request */
#define REQUEST_FROM_USER		2	/* direct user action */

#define STATE_NORMAL			0x00
#define STATE_ICONIFIED			0x01
#define STATE_MAXIMIZED_HORIZ		0x02
#define STATE_MAXIMIZED_VERT		0x04
#define STATE_MAXIMIZED_BOTH		(STATE_MAXIMIZED_HORIZ | STATE_MAXIMIZED_VERT)
#define STATE_FULLSCREEN		0x08

int hex2int(const char *hexa_string);
const char *gravityToStr(int gravity);
Window get_in_window(Display * display, Window w);
int get_window_name(Display * display, Window w, unsigned char **name);
int get_window_state(Display * display, Window wnd);
int set_window_state(Display* display, Window w, int state);
Bool is_menu(Display * display, Window wnd);
int is_splash_window(Display * display, Window w);
int is_modal_window(Display * display, Window w);
int get_window_type(Display * display, Window w, Atom * atom);
int get_window_pid(Display * display, Window w, int *pid);
int get_parent_window(Display * display, Window w, Window * parent);
int is_good_window(Display * display, Window w);
Bool is_button_proxy_window(Display * display, Window wnd);
void set_window_class_exceptions_list(struct list * wm_classnames);
void set_wm_classnames_list(struct list * wm_classnames);
void xutils_delete_all_lists();
int
get_property(Display * display, Window w, const char *property,
	     unsigned long *nitems, unsigned char **data);

void initializeXUtils(Display *dpy);
void close_menu(Display* display);
void close_window(Display* display, Window wnd);
Bool get_window_class(Display * display, Window wnd, char ** classname);
Bool is_WM_menu(Display * display, Window wnd);
Bool is_windows_class_exception(Display * display, Window wnd);
Bool getFrameExtents(Display * display, Window wnd, int * left, int * right, int * top, int * bottom);
Atom getActiveWindowAtom();
Window getActiveWindow(Display * display);
void setActiveWindow(Display * display, Window wnd);
Bool is_dropdown_menu(Display * display, Window wnd);
Bool isNameAtom(Display * display, Atom atom);
Bool isStateAtom(Display * display, Atom atom);
Bool exists_window(Display * display, Window wnd);
void set_focus(Display* display, Window w);

#endif				/* XUTILS_H_ */
