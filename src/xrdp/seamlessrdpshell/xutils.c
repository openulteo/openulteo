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

#include "os_calls.h"
#include "xutils.h"
#include <log.h>
#include <stdlib.h>

extern struct log_config *l_config;

static char *gravity_strs[] = { "ForgetGravity",
	"NorthWestGravity",
	"NorthGravity",
	"NorthEastGravity",
	"WestGravity",
	"CenterGravity",
	"EastGravity",
	"SouthWestGravity",
	"SouthGravity",
	"SouthEastGravity",
	"StaticGravity"
};

static char *wmstate_strs[] = {
	"WithdrawnState",
	"NormalState",
	"UnknownState",
	"IconicState"
};

struct list *window_class_exceptions_list = NULL;

struct list *wm_classnames_list = NULL;


static Display *g_display;

static Atom g_atom_net_active_window = None;

static Atom g_atom_net_frame_extends = None;

static Atom g_atom_net_wm_state = None;
static Atom g_atom_net_wm_state_maximized_horz = None;
static Atom g_atom_net_wm_state_maximized_vert = None;
static Atom g_atom_net_wm_state_fullscreen = None;
static Atom g_atom_net_wm_state_hidden = None;
static Atom g_atom_net_wm_state_modal = None;

static Atom g_atom_net_wm_window_type = None;
static Atom g_atom_net_wm_window_type_dropdown_menu = None;
static Atom g_atom_net_wm_window_type_popup_menu = None;
static Atom g_atom_net_wm_window_type_utility = None;
static Atom g_atom_net_wm_window_type_normal = None;
static Atom g_atom_net_wm_window_type_splash = None;

static Atom g_atom_net_wm_allowed_actions = None;
static Atom g_atom_net_wm_action_close = None;
static Atom g_atom_net_close_window = None;

static Atom g_atom_net_wm_pid = None;

static Atom g_atom_wm_state = None;
static Atom g_atom_wm_change_state = None;

static Atom g_atom_wm_transient_for = None;

static Atom g_atom_wm_protocols = None;
static Atom g_atom_wm_delete_window = None;

static Atom g_atom_win_desktop_button_proxy = None;

void initializeXUtils(Display *dpy) {
	g_display = dpy;

	g_atom_net_active_window = XInternAtom(g_display, "_NET_ACTIVE_WINDOW", False);

	g_atom_net_frame_extends = XInternAtom(g_display, "_NET_FRAME_EXTENTS", False);

	g_atom_net_wm_state = XInternAtom(g_display, "_NET_WM_STATE", False);
	g_atom_net_wm_state_maximized_horz = XInternAtom(g_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	g_atom_net_wm_state_maximized_vert = XInternAtom(g_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	g_atom_net_wm_state_fullscreen = XInternAtom(g_display, "_NET_WM_STATE_FULLSCREEN", False);
	g_atom_net_wm_state_hidden = XInternAtom(g_display, "_NET_WM_STATE_HIDDEN", False);
	g_atom_net_wm_state_modal = XInternAtom(g_display, "_NET_WM_STATE_MODAL", False);

	g_atom_net_wm_window_type = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE", False);
	g_atom_net_wm_window_type_dropdown_menu = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False);
	g_atom_net_wm_window_type_popup_menu = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
	g_atom_net_wm_window_type_utility = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	g_atom_net_wm_window_type_normal = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	g_atom_net_wm_window_type_splash = XInternAtom(g_display, "_NET_WM_WINDOW_TYPE_SPLASH", False);

	g_atom_net_wm_allowed_actions = XInternAtom(g_display, "_NET_WM_ALLOWED_ACTIONS", False);
	g_atom_net_wm_action_close = XInternAtom(g_display, "_NET_WM_ACTION_CLOSE", False);

	g_atom_net_close_window = XInternAtom(g_display, "_NET_CLOSE_WINDOW", False);

	g_atom_net_wm_pid = XInternAtom(g_display, "_NET_WM_PID", False);

	g_atom_wm_state = XInternAtom(g_display, "WM_STATE", False);
	g_atom_wm_change_state = XInternAtom(g_display, "WM_CHANGE_STATE", False);

	g_atom_wm_transient_for = XInternAtom(g_display, "WM_TRANSIENT_FOR", False);

	g_atom_wm_protocols = XInternAtom(g_display, "WM_PROTOCOLS", False);
	g_atom_wm_delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW", False);

	g_atom_win_desktop_button_proxy = XInternAtom(g_display, "_WIN_DESKTOP_BUTTON_PROXY", False);
}

/*****************************************************************************/
const char *gravityToStr(int gravity)
{
	if (gravity >= (sizeof(gravity_strs) / sizeof(gravity_strs[0])))
		return "BadGravity";

	return gravity_strs[gravity];
}

/*****************************************************************************/
const char *wmstateToStr(int wmstate)
{
	if (wmstate >= (sizeof(wmstate_strs) / sizeof(wmstate_strs[0])))
		return wmstate_strs[2];

	return wmstate_strs[wmstate];
}

/*****************************************************************************/
int hex2int(const char *hexa_string)
{
	int ret = 0, t = 0, n = 0;
	const char *c = hexa_string + 2;

	while (*c && (n < 16)) {
		if ((*c >= '0') && (*c <= '9'))
			t = (*c - '0');
		else if ((*c >= 'A') && (*c <= 'F'))
			t = (*c - 'A' + 10);
		else if ((*c >= 'a') && (*c <= 'f'))
			t = (*c - 'a' + 10);
		else
			break;
		n++;
		ret *= 16;
		ret += t;
		c++;
		if (n >= 8)
			break;
	}
	return ret;
}

void set_window_class_exceptions_list(struct list * wm_classnames) {
	window_class_exceptions_list = wm_classnames;
}

void set_wm_classnames_list(struct list * wm_classnames) {
	wm_classnames_list = wm_classnames;
}

void xutils_delete_all_lists() {
	list_delete(window_class_exceptions_list);
	list_delete(wm_classnames_list);
}

/*****************************************************************************/
int
get_property(Display * display, Window w, const char *property,
	     unsigned long *nitems, unsigned char **data)
{
	Atom actual_type;
	int actual_format;
	unsigned long bytes;
	int status;

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_property]: "
		    "Get property for window 0x%08lx : %s", w, property);
	status = XGetWindowProperty(display,
				    w,
				    XInternAtom(display, property, True),
				    0,
				    (~0L),
				    False,
				    AnyPropertyType,
				    &actual_type,
				    &actual_format, nitems, &bytes, data);

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_property]: "
		    "Get property for window 0x%08lx : %s return status: %i nitems: %lu ", w, property, status, *nitems);

	if (status != Success) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_property]: "
			    "Unable to get atom %s for window 0x%08lx", property, w);
		return 1;
	}
	return 0;
}

void close_menu(Display* display)
{
	int i = 0;
	Window root;
	Window parent;
	Window *children;
	unsigned int nchildren;

	if (!XQueryTree(display, DefaultRootWindow(display), &root, &parent, &children, &nchildren) || nchildren == 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_in_window]: "
				"Unable to get child of root windows");
		return;
	}

	for (i = 0 ; i< nchildren; i++)
	{
		if (is_menu(display, children[i]))
			XUnmapWindow(display, children[i]);
	}
}

void set_focus(Display* display, Window w) {
	close_menu(display);

	XMapRaised(display, w);
	XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);

	XFlush(display);
}

/*****************************************************************************/
Window get_in_window(Display * display, Window w)
{
	Window root;
	Window parent;
	Window *children;
	unsigned int nchildren;

	if (!XQueryTree(display, w, &root, &parent, &children, &nchildren)
	    || nchildren == 0) {
		log_message(l_config, LOG_LEVEL_DEBUG,
			    "XHook[get_in_window]: " "Window 0x%08lx has no child window", w);
		return 0;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_in_window]: "
		    "In window : 0x%08lx Out window: 0x%08lx", children[nchildren - 1], w);
	return children[nchildren - 1];
}

static int get_atoms_from_atom(Display *display, Window wnd, Atom atom, Atom **return_atoms, unsigned long *nitems) {
	char *atom_name;
	unsigned char *data;
	int status;
	int i;

	if (atom == None)
		return -1;

	if (! (atom_name = XGetAtomName(display, atom))) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_atoms_from_atom]: "
			    "Atom does not exist on display %s", XDisplayString(display));
		return -2;
	}
	
	status = get_property(display, wnd, atom_name, nitems, &data);
	if (status != 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_atoms_from_atom]: "
			    "Window(0x%08lx): Failed to get atom %s", wnd, atom_name);
		return -3;
	}
	
	if (*nitems == 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_atoms_from_atom]: "
			    "Window 0x%08lx: atom %s is empty", wnd, atom_name);
		return 0;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_atoms_from_atom]: "
			    "Window 0x%08lx: atom %s contains %ul atom(s)", wnd, atom_name, *nitems);

	*return_atoms = malloc(sizeof(Atom) * *nitems);
	if (! (*return_atoms)) {
		log_message(l_config, LOG_LEVEL_WARNING, "XHook[get_atoms_from_atom]: "
			    "Failed to allocate memory(%i bytes)", (sizeof(Atom) * *nitems));
		return -4;
	}

	for (i = 0; i < *nitems; i++) {
		(*return_atoms)[i] = (Atom)
		    ((*((unsigned char *)data + 0) << 0) |
		     (*((unsigned char *)data + 1) << 8) |
		     (*((unsigned char *)data + 2) << 16) |
		     (*((unsigned char *)data + 3) << 24)
		    );
		data += 4;

		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_atoms_from_atom]: "
			    "Window 0x%08lx: atom %s contains atom: %ul", wnd, atom_name, (*return_atoms)[i]);
	}

	return 0;
}

static Bool getCardinalFromAtom(Display *display, Window wnd, Atom atom, XID *cardinal) {
	Atom *return_atoms;
	unsigned long nitems;
	int status;

	status = get_atoms_from_atom(display, wnd, atom, &return_atoms, &nitems);
	if (status != Success || nitems == 0)
		return False;

	*cardinal = (XID) return_atoms[0];
	
	return True;
}

static Bool isAtomContainedInAtom(Display *display, Window wnd, Atom atom_container, Atom atom_requested) {
	Atom *atoms = NULL;
	unsigned long nitems;
	int status;
	int i;
	Bool isAtomContained = False;

	if (atom_container == None || atom_requested == None)
		return False;

	status = get_atoms_from_atom(display, wnd, atom_container, &atoms, &nitems);
	if (status != 0) {
		char *name = XGetAtomName(display, atom_container);
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[isAtomContainedInAtom]: "
			    "Unable to get atom %s on window 0x%08lx", name, wnd);
		XFree(name);

		return False;
	}

	for (i = 0; i < nitems; i++) {
		if (atoms[i] != atom_requested)
			continue;

		isAtomContained = True;
		break;
	}

	return isAtomContained;
}

int is_good_window(Display * display, Window w)
{
	unsigned char *data;
	unsigned long nitems;
	int status;

	if (is_splash_window(display, w))
		return 0;

	status = get_property(display, w, "WM_HINTS", &nitems, &data);
	if ((status != 0) || nitems == 0 || (data == 0)) {
		status = get_property(display, w, "WM_NORMAL_HINTS", &nitems, &data);
		if ((status != 0) || nitems == 0 || (data == 0)) {
			log_message(l_config, LOG_LEVEL_DEBUG, "XHook[is_good_window]: "
				    "Window 0x%08lx did not contain the right information", w);
			return 1;
		}
	}
	return 0;
}

/*****************************************************************************/
int get_window_name(Display * display, Window w, unsigned char **name)
{
	unsigned long nitems;
	int status;
	unsigned char * p = NULL;

	status = get_property(display, w, "_NET_WM_NAME", &nitems, name);
	if (status != 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
			    "Window 0x%08lx: Unable to get atom _NET_WM_NAME", w);

		status = get_property(display, w, "WM_NAME", &nitems, name);
		if (status != 0) {
			log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				    "Window 0x%08lx: Unable to get atom WM_NAME", w);
			return False;
		}
		if (nitems == 0 || name == 0) {
			log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				    "Window 0x%08lx has no name in atom WM_NAME", w);
			return False;
		}
	}
	else if (nitems == 0 || name == 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
			    "Window 0x%08lx has no name in atom _NET_WM_NAME", w);
		return False;
	}
	
	p = *name;

	while (*p != '\0') {
		if ((*p < 0x20 && *p > 0) || *p == ',')
			*p = '_';
		p++;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
		    "Window(0x%08lx) name : %s\n", w, *name);
	return True;
}

static int get_net_wm_state(Display * display, Window w, Atom ** atoms, unsigned long *nitems)
{
	int status;

	status = get_atoms_from_atom(display, w, g_atom_net_wm_state, atoms, nitems);
	if (status != 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_net_wm_state]: "
			    "Unable to get window(0x%08lx) states", w);
		return 1;
	}

	return 0;
}

static int get_wm_state(Display* display, Window wnd) {
	XID xid;

	if (! getCardinalFromAtom(display, wnd, g_atom_wm_state, &xid))
		return -1;

	return (int) xid;
}

int get_window_state(Display * display, Window wnd) {
	Atom *states;
	unsigned long nstates;
	int i;
	int state = STATE_NORMAL;
	int wmstate;

	wmstate = get_wm_state(display, wnd);
	switch (wmstate) {
		case IconicState:
		case NormalState:
			break;
		default:
			return -1;
	}

	if (get_net_wm_state(display, wnd, &states, &nstates) != 0)
		return -2;

	if (nstates == 0) {
		if (wmstate == NormalState)
			return state;
		else
			return -3;
	}

	for (i = 0; i < nstates; i++) {
		if (states[i] == None)
			continue;
		
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_state]: "
			    "%iWindow 0x%08lx has state: %s", i, wnd, XGetAtomName(display, states[i]));

		if (states[i] == g_atom_net_wm_state_hidden && wmstate == IconicState)
			state |= STATE_ICONIFIED;
		else if (states[i] == g_atom_net_wm_state_maximized_horz && wmstate == NormalState)
			state |= STATE_MAXIMIZED_HORIZ;
		else if (states[i] == g_atom_net_wm_state_maximized_vert && wmstate == NormalState)
			state |= STATE_MAXIMIZED_VERT;
		else if (states[i] == g_atom_net_wm_state_fullscreen && wmstate == NormalState)
			state |= STATE_FULLSCREEN;
	}

	return state;
}

static void change_wm_state(Display* display, Window wnd, int wmstate) {
	int old_wmstate;
	XEvent xev;

	switch (wmstate) {
		case IconicState:
		case NormalState:
			break;
		default:
			log_message(l_config, LOG_LEVEL_ERROR, "XHook[change_wm_state]: "
				    "Unknown state %i", wmstate);
			return;
	}

	old_wmstate = get_wm_state(display, wnd);

	// see http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.4
	
	if (old_wmstate == IconicState && wmstate == NormalState) {
		XMapWindow(display, wnd);
		XFlush(display);

		return;
	}
	else if (old_wmstate == NormalState && wmstate == IconicState) {
		if (g_atom_wm_change_state == None) {
			log_message(l_config, LOG_LEVEL_ERROR, "XHook[change_wm_state]: "
				"Window 0x%08lx: Unable to change WM state from %s(%i) to %s(%i): the atom WM_CHANGE_STATE does not exist", wnd, wmstateToStr(old_wmstate), old_wmstate, wmstateToStr(wmstate), wmstate);
			return;
		}

		xev.type = ClientMessage;
		xev.xclient.window = wnd;
		xev.xclient.message_type = g_atom_wm_change_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = wmstate;
		XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask|SubstructureRedirectMask, &xev);
	}

	XFlush(display);
}

static void remove_net_wm_state(Display* display, Window wnd, Atom state) {
	XEvent xev;

	if (state == None) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[remove_net_wm_state]: "
			    "No state to remove");
		return;
	}

	if (g_atom_net_wm_state == None) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[remove_net_wm_state]: "
			"Unable to remove state %s of window 0x%08lx: the atom _NET_WM_STATE does not exist", XGetAtomName(display, state), wnd);
		return;
	}

	xev.type = ClientMessage;
	xev.xclient.window = wnd;
	xev.xclient.message_type = g_atom_net_wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
	xev.xclient.data.l[1] = state;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 2;
	xev.xclient.data.l[4] = 0;
	XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask|SubstructureRedirectMask, &xev);
	XFlush(display);
}

static void add_net_wm_state(Display* display, Window wnd, Atom state1, Atom state2) {
	XEvent xev;

	if (state1 == None) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[add_net_wm_state]: "
			    "No state to add");
		return;
	}

	if (g_atom_net_wm_state == None) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[add_net_wm_state]: "
			"Unable to add state(s) to window 0x%08lx: the atom _NET_WM_STATE does not exist", wnd);
		return;
	}

	xev.type = ClientMessage;
	xev.xclient.window = wnd;
	xev.xclient.message_type = g_atom_net_wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = _NET_WM_STATE_ADD;
	xev.xclient.data.l[1] = state1;
	xev.xclient.data.l[2] = state2;
	xev.xclient.data.l[3] = 2;
	xev.xclient.data.l[4] = 0;
	XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask|SubstructureRedirectMask, &xev);
	XFlush(display);
}

/*****************************************************************************/
int
set_window_state(Display* display, Window wnd, int state) {
	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[set_window_state]: "
		    "State request: window 0x%08lx state %d", wnd, state);

	if (g_atom_net_wm_state == None) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[set_window_state]: "
			    "Unable to change state of window 0x%08lx to %d: the atom _NET_WM_STATE does not exist", wnd, state);
		return -1;
	}

	switch (state) {
		case STATE_MAXIMIZED_BOTH:
			remove_net_wm_state(display, wnd, g_atom_net_wm_state_hidden);

			// change_wm_state(display, wnd, NormalState); /* Window may become in normal state from iconic state (instead of maximized state) */

			add_net_wm_state(display, wnd, g_atom_net_wm_state_maximized_horz, g_atom_net_wm_state_maximized_vert);
			break;
		case STATE_NORMAL:
			remove_net_wm_state(display, wnd, g_atom_net_wm_state_hidden);
			remove_net_wm_state(display, wnd, g_atom_net_wm_state_maximized_horz);
			remove_net_wm_state(display, wnd, g_atom_net_wm_state_maximized_vert);

			change_wm_state(display, wnd, NormalState);
			break;
		case STATE_ICONIFIED:
			change_wm_state(display, wnd, IconicState);
			
			add_net_wm_state(display, wnd, g_atom_net_wm_state_hidden, None);
			break;
		default:
			log_message(l_config, LOG_LEVEL_DEBUG, "XHook[set_window_state]: "
				    "Unable to change state of window 0x%08lx to %d: The state %d is not supported", wnd, state, state);
			return -1;
	}

	return state;
}

void close_window(Display* display, Window wnd) {
	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[close_window]: "
			    "Window 0x%08lx", wnd);
	if (isAtomContainedInAtom(display, wnd, g_atom_net_wm_allowed_actions, g_atom_net_wm_action_close)) {
		// see http://standards.freedesktop.org/wm-spec/wm-spec-1.4.html#id2551096
		XEvent xev;

		xev.type = ClientMessage;
		xev.xclient.format = 32;
		xev.xclient.message_type = g_atom_net_close_window;
		xev.xclient.window = wnd;
		xev.xclient.data.l[0] = CurrentTime;
		xev.xclient.data.l[1] = REQUEST_FROM_USER;
		xev.xclient.data.l[2] = 0;


		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;

		XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);
	}
	else if (isAtomContainedInAtom(display, wnd, g_atom_wm_protocols, g_atom_wm_delete_window)) {
		// see http://standards.freedesktop.org/wm-spec/wm-spec-1.4.html#id2551096
 		XEvent xev;

		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[close_window]: "
			    "Window 0x%08lx does not support _NET_WM_ACTION_CLOSE, will directly use WM_DELETE_WINDOW protocol", wnd);

 		xev.type = ClientMessage;
 		xev.xclient.format = 32;
		xev.xclient.message_type = g_atom_wm_protocols;
 		xev.xclient.window = wnd;
		xev.xclient.data.l[0] = g_atom_wm_delete_window;
		xev.xclient.data.l[1] = CurrentTime;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;
		
		XSendEvent(display, wnd, False, 0, &xev);
	}
	else {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[close_window]: "
			    "Window 0x%08lx does not support _NET_CLOSE_WINDOW action nor the WM_DELETE_WINDOW protocol, will use an alternate method which would crash the window", wnd);

		/* To improve:
		 * XDestroyWindow kills the window but the process is alive.
		 * XKillClient kills the process of wnd but if wnd is not the main window, the process is also killed
		 */
		XDestroyWindow(display, wnd);
		//XKillClient(display, wnd);
	}

	XFlush(display);
}

Bool is_menu(Display * display, Window wnd) {
	Atom window_type = None;

	if (! get_window_type(display, wnd, &window_type))
		return False;

	if (window_type == g_atom_net_wm_window_type_dropdown_menu)
		return True;

	if (window_type == g_atom_net_wm_window_type_popup_menu)
		return True;

	if (window_type == g_atom_net_wm_window_type_utility)
		return True;

	return False;
}

int is_splash_window(Display * display, Window w) {
	Atom type;
	get_window_type(display, w, &type);
	if (type == g_atom_net_wm_window_type_splash)
		return 1;

	return 0;
}

int is_modal_window(Display * display, Window w) {
	Atom *states;
	unsigned long nstates;
	int i;
	
	if (get_net_wm_state(display, w, &states, &nstates) == 0) {
		for (i = 0; i < nstates; i++) {
			if (states[i] == XInternAtom(display, "_NET_WM_STATE_MODAL", False))
				return 1;
		}
	}

	return 0;
}

int get_window_type(Display * display, Window w, Atom * atom)
{
	XID xid;

	if (! getCardinalFromAtom(display, w, g_atom_net_wm_window_type, &xid))
		return False;

	*atom = (Atom) xid;

	return True;
}

/*****************************************************************************/
int get_window_pid(Display * display, Window w, int *pid)
{
	XID xid;

	if (! getCardinalFromAtom(display, w, g_atom_net_wm_pid, &xid))
		return False;

	*pid = (int) xid;
	
	return True;
}

/*****************************************************************************/
int get_parent_window(Display * display, Window w, Window * parent)
{
	XID xid;

	if (!getCardinalFromAtom(display, w, g_atom_wm_transient_for, &xid))
		return False;

	*parent = (Window) xid;

	return True;
}

Bool is_button_proxy_window(Display * display, Window wnd)
{
	// see ftp://ftp.math.utah.edu/u/ma/hohn/linux/gnome/gnome-libs/devel-docs/WM-GOME.txt Chapter 3 Section 1
	XID xid;

	if (getCardinalFromAtom(display, wnd, g_atom_win_desktop_button_proxy, &xid)) {
		if (((Window) xid) == wnd) {
			log_message(l_config, LOG_LEVEL_DEBUG, "XHook[is_window_displayable]: "
				    "Window 0x%08lx is not displayable: _WIN_DESKTOP_BUTTON_PROXY is this window", wnd);
			return False;
		}
	}

	return True;
}

Bool get_window_class(Display * display, Window wnd, char ** classname)
{
	XClassHint *class_hint = XAllocClassHint();

	if (class_hint == NULL) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[get_window_class]: "
			    "Unable to allocate memory for 'XClassHint' structure");
		return False;
	}
	
	*classname = NULL;

	if (XGetClassHint(display, wnd, class_hint)) {
		*classname = g_strdup(class_hint->res_class);
	}
	if (class_hint) {
		if (class_hint->res_class)
			XFree(class_hint->res_class);
		if (class_hint->res_name)
			XFree(class_hint->res_name);
	}
	
	return (*classname != NULL);
}

Bool is_WM_menu(Display * display, Window wnd)
{
	Atom type = None;
	char* wnd_classname = NULL;
	
	if (! get_window_class(display, wnd, &wnd_classname)) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[is_WM_menu]: "
			    "Failed to get the window 0x%08lx classname", wnd);
		return False;
	}
	
	if (! list_contains_string(wm_classnames_list, wnd_classname)) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[is_WM_menu]: "
			    "Window 0x%08lx is not a WM window", wnd);
		return False;
	}
	
	get_window_type(display, wnd, &type);
	if (type != g_atom_net_wm_window_type_popup_menu) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[is_WM_menu]: "
			    "Window 0x%08lx is not a WM menu", wnd);
		return False;
	}
	
	log_message(l_config, LOG_LEVEL_INFO, "XHook[is_WM_menu]: "
		    "Window 0x%08lx is a WM menu", wnd);
	
	g_free(wnd_classname);

	return True;
}

Bool is_windows_class_exception(Display * display, Window wnd)
{
	Atom type = None;
	char* wnd_classname = NULL;
	Bool result;
	
	if (! get_window_class(display, wnd, &wnd_classname)) {
		log_message(l_config, LOG_LEVEL_ERROR, "XHook[is_windows_class_exception]: "
			    "Failed to get the window 0x%08lx classname", wnd);
		return False;
	}
	
	get_window_type(display, wnd, &type);
	// Allow WM's windows (only menus) to display
	if (type == g_atom_net_wm_window_type_popup_menu)
		return False;
	
	result = list_contains_string(window_class_exceptions_list, wnd_classname);
	
	g_free(wnd_classname);
	
	return result;
}

Bool is_dropdown_menu(Display * display, Window wnd) {
	Atom window_type = None;

	if (! get_window_type(display, wnd, &window_type)) {
		log_message(l_config, LOG_LEVEL_WARNING, "XHook[is_dropdown_menu]: "
			    "[Window 0x%08lx] Failed to get window type", wnd);
		return False;
	}

	if (window_type == g_atom_net_wm_window_type_dropdown_menu)
		return True;

	return False;
}

Atom getActiveWindowAtom() {
	return g_atom_net_active_window;
}

Window getActiveWindow(Display * display) {
	Window activeWindow = None;

	getCardinalFromAtom(display, DefaultRootWindow(display), g_atom_net_active_window, &activeWindow);

	return activeWindow;
}

void setActiveWindow(Display * display, Window wnd) {
	XSetInputFocus(display, wnd, RevertToParent, CurrentTime);
	XFlush(display);
}

Bool getFrameExtents(Display * display, Window wnd, int * left, int * right, int * top, int * bottom) {
	Atom *atoms = NULL;
	unsigned long nitems;
	int status;

	if (! left || ! right || ! top || ! bottom)
		return False;

	if (g_atom_net_frame_extends == None)
		return False;

	status = get_atoms_from_atom(display, wnd, g_atom_net_frame_extends, &atoms, &nitems);
	if (status != 0) {
		char *name = XGetAtomName(display, g_atom_net_frame_extends);
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[getFrameExtents]: "
			    "Unable to get atom %s on window 0x%08lx", name, wnd);
		XFree(name);

		return False;
	}
	if (nitems != 4) {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[getFrameExtents]: "
			    "Window 0x%08lx. Bad content(size: %lu)", wnd, nitems);
		return False;
	}

	*left = atoms[0];
	*right = atoms[1];
	*top = atoms[2];
	*bottom = atoms[3];

	return True;
}

Bool isStateAtom(Display * display, Atom atom) {
	if (! display || atom == None)
		return False;

	if (atom == g_atom_net_wm_state || atom == g_atom_wm_state)
		return True;

	return False;
}

Bool isNameAtom(Display * display, Atom atom) {
	if (! display || atom == None)
		return False;

	if (atom == XInternAtom(display, "_NET_WM_NAME", False) || atom == XInternAtom(display, "WM_NAME", False))
		return True;

	return False;
}

Bool exists_window(Display * display, Window wnd) {
	Window root;
	int x, y;
	unsigned int width, height, border_width, depth;

	if (XGetGeometry(display, wnd, &root, &x, &y, &width, &height, &border_width, &depth))
		return True;

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[exists_window]: "
		    "Window 0x%08lx does not exist", wnd);
	return False;
}
