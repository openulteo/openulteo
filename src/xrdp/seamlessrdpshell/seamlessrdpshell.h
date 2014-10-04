/**
 * Copyright (C) 2009-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2009
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

#ifndef SEAMLESSRDPSHELL_H_
#define SEAMLESSRDPSHELL_H_

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <stdio.h>

/* config constant */
#define XHOOK_CFG_GLOBAL						"Globals"
#define XHOOK_CFG_NAME							"Name"
#define XHOOK_CFG_LOGGING						"Logging"
#define XHOOK_CFG_LOG_DIR						"LogDir"
#define XHOOK_CFG_LOG_LEVEL					"LogLevel"
#define XHOOK_CFG_LOG_ENABLE_SYSLOG	"EnableSyslog"
#define XHOOK_CFG_LOG_SYSLOG_LEVEL	"SyslogLevel"
#define XHOOK_CFG_SEAMLESS					"Seamless"
#define XHOOK_CFG_SEAMLESS_WM_CLASSNAMES			"WMClassnames"
#define XHOOK_CFG_SEAMLESS_HIDDEN_CLASSNAMES			"HiddenClassnames"

/* socket constant */
#define ERROR	-1
#define	SOCKET_ERRNO	errno
#define	ERRNO		errno

/* seamlessrdpshell constant */
#define SEAMLESS_HELLO_RECONNECT	0x0001

#define SEAMLESSRDP_CREATE_MODAL    0x1
#define SEAMLESSRDP_CREATE_TOPMOST  0x2
#define SEAMLESS_CREATE_POPUP       0x4
#define SEAMLESS_CREATE_FIXEDSIZE   0x8
#define SEAMLESS_CREATE_TOOLTIP     0x10

#define SEAMLESSRDP_NOTYETMAPPED		-1
#define SEAMLESSRDP_NORMAL				0
#define SEAMLESSRDP_MAXIMIZED			2
#define SEAMLESSRDP_MINIMIZED			1
#define SEAMLESSRDP_FULLSCREEN                  3

#define SEAMLESS_FOCUS_REQUEST      0X0000
#define SEAMLESS_FOCUS_RELEASE      0X0001

#define SEAMLESS_ICON_SIZE 32

#if __WORDSIZE==64
	typedef long int Icon_data;
#else
	typedef int Icon_data;
#endif

typedef struct {
	int width;
	int height;
	Icon_data *p;

} WindowIcon;

typedef struct {
	int state;
	int ack_id;
} StateOrder;

typedef struct {
	int state;
	Window window_id;
	Window win_out;
	Window parent;
	char * name;
        StateOrder* waiting_state;
} Window_item;

typedef struct {
	Window_item list[1024];
	int item_count;

} Window_list;

/* windows list macro */
#define Window_list_init(window_list){\
		window_list.item_count = 0;\
}\

#define Window_add(window_list, window, win_out){\
	int count = window_list.item_count;\
	Window_item* temp;\
	Window_get(window_list, window,temp);\
	if(temp == 0){\
		window_list.list[count].state = SEAMLESSRDP_NOTYETMAPPED;\
		window_list.list[count].window_id = window;\
		window_list.list[count].win_out = win_out;\
		window_list.list[count].parent = (Window) 0;\
		window_list.list[count].name = NULL;\
                window_list.list[count].waiting_state = NULL;\
		window_list.item_count++;\
	};\
}\


#define Window_get(window_list, window, window_item){\
	int i;\
	int count = window_list.item_count;\
	window_item =0;\
	for(i=0 ; i < count; i++){\
		if(window_list.list[i].window_id == window || window_list.list[i].win_out == window){\
			window_item = &window_list.list[i];\
			break;\
		}\
	}\
}\


#define Window_del(window_list, window){\
	int count = window_list.item_count;\
	Window_item* temp;\
	Window_get(window_list, window,temp);\
	if(temp != 0){\
		temp->state = window_list.list[count-1].state;\
		temp->window_id = window_list.list[count-1].window_id;\
		temp->win_out = window_list.list[count-1].win_out;\
		temp->parent = window_list.list[count-1].parent;\
		g_free(temp->name);\
		temp->name = window_list.list[count-1].name;\
		window_list.item_count--;\
	}\
}\

#define Window_dump(window_list){\
	int i;\
	int count = window_list.item_count;\
	printf("\tdump list of %i elements\n",count);\
	for(i=0 ; i < count; i++){\
		printf("\t elem %i :: state->%i || "\
			"window_id->0x%08lx || win_out->0x%08lx || "\
			"parent->0x%08lx\n", \
			i, \
			window_list.list[i].state, \
			window_list.list[i].window_id, \
			window_list.list[i].win_out, \
			window_list.list[i].parent);\
	}\
}\


#endif				/* SEAMLESSRDPSHELL_H_ */
