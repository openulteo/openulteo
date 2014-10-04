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

#include "xutils.h"
#include <log.h>

extern struct log_config* l_config;

/*****************************************************************************/
int
hex2int(const char* hexa_string)
{
  int ret = 0, t = 0, n = 0;
  const char *c = hexa_string+2;

  while(*c && (n < 16)) {
    if((*c >= '0') && (*c <= '9'))
    	t = (*c - '0');
    else if((*c >= 'A') && (*c <= 'F'))
    	t = (*c - 'A' + 10);
    else if((*c >= 'a') && (*c <= 'f'))
    	t = (*c - 'a' + 10);
    else break;
    n++;
    ret *= 16;
    ret += t;
    c++;
    if(n >= 8) break;
  }
  return ret;
}

/*****************************************************************************/
int
get_property( Display* display, Window w, const char* property, unsigned long *nitems, unsigned char** data)
{
  Atom actual_type;
  int actual_format;
  unsigned long bytes;
  int status;

  log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_property]: "
				"Get property : %s", property);
  status = XGetWindowProperty(display,
                              w,
                              XInternAtom(display, property, True),
                              0,
                              (~0L),
                              False,
                              AnyPropertyType,
                              &actual_type,
                              &actual_format,
                              nitems,
                              &bytes,
                              data);

  if(status != Success || *nitems==0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_property]: "
					"Unable to get atom %s",property);
	return 1;
  }
  return 0;
}

/*****************************************************************************/
Window
get_in_window(Display* display,  Window w)
{
  Window root;
  Window parent;
  Window *children;
  unsigned int nchildren;

  if (!XQueryTree(display, w, &root, &parent, &children, &nchildren) || nchildren == 0){
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_in_window]: "
  			"No child windows");
    return 0;
  }

	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_in_window]: "
				"In window : %i",(int)children[nchildren-1]);
  return children[nchildren-1];
}


int is_good_window(Display* display,  Window w)
{
  unsigned char* data;
  unsigned long nitems;
  int status;

  status = get_property(display, w, "WM_HINTS", &nitems, &data);
  if((status != 0) || (data == 0))
  {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[is_good_window]: "
					" %i did not contain the right information", (int)w);
  }
  return status;
}

/*****************************************************************************/
int get_window_name(Display* display, Window w, unsigned char** name)
{
  unsigned long nitems;
  int status;
//TODO replace , par _
  status = get_property(display, w, "WM_NAME", &nitems, name);
  if(status != 0)
  {
		log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				" enable to get atom WM_NAME");
    return False;
  }
  if(name == 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
  			" no windows name in atom WM_NAME");
    return False;
  }
  log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				" windows name : %s\n",*name);
  return True;
}


int get_window_type(Display* display, Window w, Atom* atom)
{
  unsigned char* data;
  unsigned long nitems;
  int status;
  *atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL",False);

  status = get_property(display, w, "_NET_WM_WINDOW_TYPE", &nitems, &data);
  if(status != 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_type]: "
  			"Unable to get window type");
    return 1;
  }
  if(data == 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				"No window type");
    return 0;
  }

  *atom = (Atom) \
   	    ( \
   	      (*((unsigned char*) data+0) << 0) | \
   	      (*((unsigned char*) data + 1) << 8) | \
   	      (*((unsigned char*) data+ 2) << 16) | \
   	      (*((unsigned char*) data+ 3) << 24) \
   	    );

  log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_name]: "
				"Window type : %s", XGetAtomName(display, *atom));
  return 0;
}

/*****************************************************************************/
int get_window_pid(Display* display, Window w, int* pid){
  unsigned char* data;
  unsigned long     nitems;
  int status;

  status = get_property(display, w, "_NET_WM_PID", &nitems, &data);
  if(status != 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_pid]: "
					"Unable to get pid of window %i", (int)w);
    return False;
  }
  if(data == 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_pid]: "
					"No pid for window %i", (int)w);
    return False;
  }
  *pid = (int) \
		( \
		  (*((unsigned char*) data+0) << 0) | \
		  (*((unsigned char*) data + 1) << 8) | \
		  (*((unsigned char*) data+ 2) << 16) | \
		  (*((unsigned char*) data+ 3) << 24) \
		);
  log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_window_pid]: "
				"Window pid: %i", *pid);
  return True;
}

/*****************************************************************************/
int get_parent_window(Display* display, Window w, Window* parent){
  unsigned char* data;
  unsigned long nitems;
  int status;


  status = get_property(display, w, "WM_TRANSIENT_FOR", &nitems, &data);
  if(status != 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_parent_window]: "
  			"Unable to get parent of window %i", (int)w);
    *parent = 0;
    return False;
  }
  if(data == 0)
  {
  	log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_parent_window]: "
  			"No parent window for window %i", (int)w);
    *parent = 0;
    return False;
  }


  *parent = (Window) \
		( \
		  (*((unsigned char*) data+0) << 0) | \
		  (*((unsigned char*) data + 1) << 8) | \
		  (*((unsigned char*) data+ 2) << 16) | \
		  (*((unsigned char*) data+ 3) << 24) \
		);
  log_message(l_config, LOG_LEVEL_DEBUG, "XHook[get_parent_window]: "
				"Parent window for window %i : %i\n", (int)w, (int)parent);
  return True;
}
