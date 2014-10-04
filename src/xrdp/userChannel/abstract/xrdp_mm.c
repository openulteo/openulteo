/**
 * Copyright (C) 2012-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author James B. MacLean <macleajb@ednet.ns.ca> 2012
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2013
 * Author David LECHEVALIER <david@ulteo.com> 2013
 * Author Vincent ROULLIER <vincent.roullier@ulteo.com> 2013
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

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   xrdp: A Remote Desktop Protocol server.
   Copyright (C) Jay Sorg 2004-2010

   module manager

*/

#include "xrdp_mm.h"
#include "xrdp_wm.h"
#include "os_calls.h"
#include "xrdp_painter.h"
#include "xrdp_cache.h"
#include "userChannel.h"
#include "xrdp_module.h"
#include <xrdp/xrdp_types.h>
#include <libxrdp/libxrdp.h>


static int APP_CC
xrdp_mm_sesman_data_in(struct trans* trans);

/*****************************************************************************/
struct xrdp_mm* APP_CC
xrdp_mm_create(struct xrdp_wm* owner)
{
  struct xrdp_mm* self;

  self = (struct xrdp_mm*)g_malloc(sizeof(struct xrdp_mm), 1);
  self->wm = owner;
  self->login_names = list_create();
  self->login_names->auto_free = 1;
  self->login_values = list_create();
  self->login_values->auto_free = 1;
  self->connected = false;
  return self;
}

/*****************************************************************************/
/* called from main thread */
static long DEFAULT_CC
xrdp_mm_sync_unload(long param1, long param2)
{
  return g_free_library(param1);
}

/*****************************************************************************/
/* called from main thread */
static long DEFAULT_CC
xrdp_mm_sync_load(long param1, long param2)
{
  long rv;
  char* libname;

  libname = (char*)param1;
  rv = g_load_library(libname);
  return rv;
}

/*****************************************************************************/
static void APP_CC
xrdp_mm_module_cleanup(struct xrdp_mm* self)
{
  if (self->mod != 0)
  {
    lib_userChannel_cleanup((struct userChannel*)self->mod);
  }
}

/*****************************************************************************/
void APP_CC
xrdp_mm_delete(struct xrdp_mm* self)
{
  if (self == 0)
  {
    return;
  }
  /* free any module stuff */
  xrdp_mm_module_cleanup(self);
  trans_delete(self->sesman_trans);
  self->sesman_trans = 0;
  self->sesman_trans_up = 0;
  list_delete(self->login_names);
  list_delete(self->login_values);
  g_free(self);
}

/*****************************************************************************/
static int APP_CC
xrdp_mm_send_login(struct xrdp_mm* self)
{
  struct stream* s;
  int rv;
  int index;
  int count;
  char* username;
  char* password;
  char* name;
  char* value;

#ifdef OLD_LOG_VERSION
  xrdp_wm_log_msg(self->wm, "sending login info to sesman");
#endif
  username = 0;
  password = 0;
  self->code = 0;
  count = self->login_names->count;
  for (index = 0; index < count; index++)
  {
    name = (char*)list_get_item(self->login_names, index);
    value = (char*)list_get_item(self->login_values, index);
    if (g_strcasecmp(name, "username") == 0)
    {
      username = value;
    }
    else if (g_strcasecmp(name, "password") == 0)
    {
      password = value;
    }
    else if (g_strcasecmp(name, "lib") == 0)
    {
      if ((g_strcasecmp(value, "libxup.so") == 0) ||
          (g_strcasecmp(value, "xup.dll") == 0))
      {
        self->code = 10;
      }
    }
  }
  if ((username == 0) || (password == 0))
  {
#ifdef OLD_LOG_VERSION
    xrdp_wm_log_msg(self->wm, "error finding username and password");
#else
    xrdp_wm_log_error(self->wm, "Username or password");
    xrdp_wm_log_error(self->wm, "are empty");
#endif
    return 1;
  }
  s = trans_get_out_s(self->sesman_trans, 8192);
  s_push_layer(s, channel_hdr, 8);
  /* this code is either 0 for Xvnc or 10 for X11rdp */
  out_uint16_be(s, self->code);
  index = g_strlen(username);
  out_uint16_be(s, index);
  out_uint8a(s, username, index);
  index = g_strlen(password);
  out_uint16_be(s, index);
  out_uint8a(s, password, index);

/*
 * Username needs to be kept in session info so that save PDU can use it (send_logon)
 */

  g_strcpy(self->wm->client_info->username, username);

  out_uint16_be(s, self->wm->screen->width);
  out_uint16_be(s, self->wm->screen->height);
  out_uint16_be(s, self->wm->screen->bpp);
  out_uint16_be(s, g_getpid());
  /* send domain */
  index = g_strlen(self->wm->client_info->domain);
  out_uint16_be(s, index);
  out_uint8a(s, self->wm->client_info->domain, index);
  /* send program / shell */
  index = g_strlen(self->wm->client_info->program);
  out_uint16_be(s, index);
  out_uint8a(s, self->wm->client_info->program, index);
  /* send directory */
  index = g_strlen(self->wm->client_info->directory);
  out_uint16_be(s, index);
  out_uint8a(s, self->wm->client_info->directory, index);
  /* send keylaout */
  out_uint16_be(s, self->wm->client_info->keylayout);
  /* send use_scim option */
  out_uint16_be(s, self->wm->client_info->use_scim);
  s_mark_end(s);
  s_pop_layer(s, channel_hdr);
  out_uint32_be(s, 0); /* version */
  index = (int)(s->end - s->data);
  out_uint32_be(s, index); /* size */
  rv = trans_force_write(self->sesman_trans);
  if (rv != 0)
  {
#ifdef OLD_LOG_VERSION
    xrdp_wm_log_msg(self->wm, "xrdp_mm_send_login: xrdp_mm_send failed");
#else
  	xrdp_wm_log_error(self->wm, "Unable to send");
  	xrdp_wm_log_error(self->wm, "logon informations");
#endif
  }
  return rv;
}

/*****************************************************************************/
/* returns error */
/* this goes through the login_names looking for one called 'aname'
   then it copies the corisponding login_values item into 'dest'
   'dest' must be at least 'dest_len' + 1 bytes in size */
static int APP_CC
xrdp_mm_get_value(struct xrdp_mm* self, char* aname, char* dest, int dest_len)
{
  char* name;
  char* value;
  int index;
  int count;
  int rv;

  rv = 1;
  /* find the library name */
  dest[0] = 0;
  count = self->login_names->count;
  for (index = 0; index < count; index++)
  {
    name = (char*)list_get_item(self->login_names, index);
    value = (char*)list_get_item(self->login_values, index);
    if ((name == 0) || (value == 0))
    {
      break;
    }
    if (g_strcasecmp(name, aname) == 0)
    {
      g_strncpy(dest, value, dest_len);
      rv = 0;
    }
  }
  return rv;
}

/*****************************************************************************/
int APP_CC
xrdp_mm_load_userchannel(struct xrdp_mm* self, const char* lib)
{
  if (self == 0)
  {
    return 1;
  }
  self->mod = (struct xrdp_mod*)lib_userChannel_init();
  self->mod->wm = (long)(self->wm);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_mm_setup_mod2(struct xrdp_mm* self)
{
  char text[256];
  char* name;
  char* value;
  int i;
  int rv;
  int key_flags;
  int device_flags;
  rv = 1;
  text[0] = 0;
  tbus term_event = self->wm->user_channel->self_term_event;

  if (!g_is_wait_obj_set(term_event))
  {
    if (lib_userChannel_mod_start((struct userChannel*)self->mod, self->wm->screen->width, self->wm->screen->height, self->wm->screen->bpp) != 0)
    {
      g_set_wait_obj(term_event); /* kill session */
    }
  }
  if (!g_is_wait_obj_set(term_event))
  {
    if (self->display > 0)
    {
      if (self->code == 0) /* Xvnc */
      {
        g_snprintf(text, 255, "%d", 5900 + self->display);
      }
      else if (self->code == 10) /* X11rdp */
      {
        g_snprintf(text, 255, "%d", 6200 + self->display);
      }
      else
      {
        g_set_wait_obj(term_event); /* kill session */
      }
    }
  }
  if (!g_is_wait_obj_set(term_event))
  {
    /* this adds the port to the end of the list, it will already be in
       the list as -1
       the module should use the last one */
    if (g_strlen(text) > 0)
    {
      list_add_item(self->login_names, (long)g_strdup("port"));
      list_add_item(self->login_values, (long)g_strdup(text));
    }
    /* always set these */
    name = self->wm->session->client_info->hostname;
    self->mod->mod_set_param(self->mod, "hostname", name);
    g_snprintf(text, 255, "%d", self->wm->session->client_info->keylayout);
    self->mod->mod_set_param(self->mod, "keylayout", text);
    for (i = 0; i < self->login_names->count; i++)
    {
      name = (char*)list_get_item(self->login_names, i);
      value = (char*)list_get_item(self->login_values, i);
      self->mod->mod_set_param(self->mod, name, value);
    }
    /* connect */
    if (self->mod->mod_connect(self->mod) == 0)
    {
      rv = 0;
      self->connected = true;
    }
  }
  if (rv == 0)
  {
    /* sync modifiers */
    key_flags = 0;
    device_flags = 0;
    if (self->wm->scroll_lock)
    {
      key_flags |= 1;
    }
    if (self->wm->num_lock)
    {
      key_flags |= 2;
    }
    if (self->wm->caps_lock)
    {
      key_flags |= 4;
    }
    if (self->mod != 0)
    {
      if (self->mod->mod_event != 0)
      {
        self->mod->mod_event(self->mod, 17, key_flags, device_flags,
                             key_flags, device_flags);
      }
    }
  }
  return rv;
}


/*****************************************************************************/
/* this is callback from trans obj
   returns error */
static int APP_CC
xrdp_mm_scim_data_in(struct trans* trans)
{
  struct xrdp_mm* self;
  struct stream* s;
  int state;

  if (trans == 0)
  {
    return 1;
  }
  self = (struct xrdp_mm*)(trans->callback_data);
  s = trans_get_in_s(trans);
  if (s == 0)
  {
    return 1;
  }
  state = (s->p)[0];
  self->wm->compose = state;
  return 0;
}

/*****************************************************************************/
/* returns error */
int APP_CC
xrdp_mm_scim_send_unicode(struct xrdp_mm* self, unsigned int unicode_key)
{
  struct stream* s;

  s = trans_get_out_s(self->scim_trans, 8192);
  if (s == 0)
  {
    return 1;
  }
  out_uint32_le(s, unicode_key);
  s_mark_end(s);
  return trans_force_write(self->scim_trans);
}

/*****************************************************************************/
static int APP_CC
xrdp_mm_process_login_response(struct xrdp_mm* self, struct stream* s)
{
  int ok;
  int display;
  int rv;
  char ip[256];
  char lib[256];
  char display_string[256];

  rv = 0;
  in_uint16_be(s, ok);
  in_uint16_be(s, display);
  if (ok)
  {
    self->display = display;
    g_sprintf(display_string, ":%i.0", display);
    g_setenv("DISPLAY", display_string, true);
#ifdef OLD_LOG_VERSION
    g_snprintf(text, 255, "xrdp_mm_process_login_response: login successful "
               "for display %d", display);
    xrdp_wm_log_msg(self->wm, text);
#endif

    lib[0] = 0;
    if (xrdp_mm_get_value(self, "lib", lib, 255) != 0)
    {
  #ifdef OLD_LOG_VERSION
          g_snprintf(text, 255, "no library name specified in xrdp.ini, please add "
                 "lib=libxrdp-vnc.so or similar");
      xrdp_wm_log_msg(self->wm, text);
  #else
      xrdp_wm_log_error(self->wm, "no library name specified in xrdp.ini");
      xrdp_wm_log_error(self->wm, "please add lib=libxrdp-vnc.so");
  #endif
      return 1;
    }
    if (lib[0] == 0)
    {
  #ifdef OLD_LOG_VERSION
      g_snprintf(text, 255, "empty library name specified in xrdp.ini,\n please "
                 "add lib=libxrdp-vnc.so or similar");
      xrdp_wm_log_msg(self->wm, text);
  #else
      xrdp_wm_log_error(self->wm, "empty library name specified in xrdp.ini");
      xrdp_wm_log_error(self->wm, "please add lib=libxrdp-vnc.so");
  #endif

      return 1;
    }

    if (lib_userChannel_load_library((struct userChannel*)self->mod, lib) == 0)
    {
      if (xrdp_mm_setup_mod2(self) == 0)
      {
        xrdp_mm_get_value(self, "ip", ip, 255);
        xrdp_wm_set_login_mode(self->wm, 10);
        self->wm->dragging = 0;

        if(self->wm->session->client_info->use_scim)
        {
          /* connect scim panel (unix socket) */
          self->scim_trans = trans_create(TRANS_MODE_UNIX, 8192, 8192);
          self->scim_trans->trans_data_in = xrdp_mm_scim_data_in;
          self->scim_trans->header_size = 1;
          self->scim_trans->callback_data = self;
        }
      }
    }
  }
  else
  {
#ifdef OLD_LOG_VERSION
    xrdp_wm_log_msg(self->wm, "Failed to open session, verify the username and the password");
#else
    int msg_len = 0;
    char* msg;

    in_uint16_be(s, msg_len);
    msg = g_malloc(msg_len, 0);
    in_uint8a(s, msg, msg_len);
    xrdp_wm_log_error(self->wm, msg);
    g_free(msg);
#endif
  }
  self->delete_sesman_trans = 1;
  self->connected_state = 0;
  if (self->wm->login_mode != 10)
  {
    xrdp_wm_set_login_mode(self->wm, 1);
    xrdp_mm_module_cleanup(self);
  }
  return rv;
}

/*****************************************************************************/
static int
xrdp_mm_get_sesman_port(char* port, int port_bytes)
{
  int fd;
  int error;
  int index;
  char* val;
  char cfg_file[256];
  struct list* names;
  struct list* values;

  /* default to port 3350 */
  g_strncpy(port, "3350", port_bytes - 1);
  /* see if port is in xrdp.ini file */
  g_snprintf(cfg_file, 255, "%s/sesman.ini", XRDP_CFG_PATH);
  fd = g_file_open(cfg_file);
  if (fd > 0)
  {
    names = list_create();
    names->auto_free = 1;
    values = list_create();
    values->auto_free = 1;
    if (file_read_section(fd, "Globals", names, values) == 0)
    {
      for (index = 0; index < names->count; index++)
      {
        val = (char*)list_get_item(names, index);
        if (val != 0)
        {
          if (g_strcasecmp(val, "ListenPort") == 0)
          {
            val = (char*)list_get_item(values, index);
            error = g_atoi(val);
            if ((error > 0) && (error < 65000))
            {
              g_strncpy(port, val, port_bytes - 1);
            }
            break;
          }
        }
      }
    }
    list_delete(names);
    list_delete(values);
    g_file_close(fd);
  }
  return 0;
}


/*****************************************************************************/
static int APP_CC
xrdp_mm_sesman_data_in(struct trans* trans)
{
  struct xrdp_mm* self;
  struct stream* s;
  int version;
  int size;
  int error;
  int code;

  if (trans == 0)
  {
    return 1;
  }
  self = (struct xrdp_mm*)(trans->callback_data);
  s = trans_get_in_s(trans);
  if (s == 0)
  {
    return 1;
  }
  in_uint32_be(s, version);
  in_uint32_be(s, size);
  error = trans_force_read(trans, size - 8);
  if (error == 0)
  {
    in_uint16_be(s, code);
    switch (code)
    {
      case 3:
        error = xrdp_mm_process_login_response(self, s);
        break;
      default:
        g_writeln("xrdp_mm_sesman_data_in: unknown code %d", code);
        break;
    }
  }
  return error;
}

/*****************************************************************************/
int APP_CC
xrdp_mm_connect(struct xrdp_mm* self)
{
  struct list* names;
  struct list* values;
  int index;
  int count;
  int use_sesman;
  int ok;
  int rv;
  char* name;
  char* value;
  char ip[256];
  char errstr[256];
  char port[8];

  rv = 0;
  use_sesman = 0;
  names = self->login_names;
  values = self->login_values;
  count = names->count;

  for (index = 0; index < count; index++)
  {
    name = (char*)list_get_item(names, index);
    value = (char*)list_get_item(values, index);
    if (g_strcasecmp(name, "ip") == 0)
    {
      g_strncpy(ip, value, 255);
    }
    else if (g_strcasecmp(name, "port") == 0)
    {
      if (g_strcasecmp(value, "-1") == 0)
      {
        use_sesman = 1;
      }
    }
  }
  if (use_sesman)
  {
    ok = 0;
    errstr[0] = 0;
    trans_delete(self->sesman_trans);
    self->sesman_trans = trans_create(TRANS_MODE_TCP, 8192, 8192);
    xrdp_mm_get_sesman_port(port, sizeof(port));
#ifdef OLD_LOG_VERSION
    g_snprintf(text, 255, "connecting to sesman ip %s port %s", ip, port);
    xrdp_wm_log_msg(self->wm, text);
#endif

    self->sesman_trans->trans_data_in = xrdp_mm_sesman_data_in;
    self->sesman_trans->header_size = 8;
    self->sesman_trans->callback_data = self;
    /* try to connect up to 4 times */
    for (index = 0; index < 4; index++)
    {
      if (trans_connect(self->sesman_trans, ip, port, 3000) == 0)
      {
        self->sesman_trans_up = 1;
        ok = 1;
        break;
      }
      g_sleep(1000);
      g_writeln("xrdp_mm_connect: connect failed "
                "trying again...");
    }
    if (ok)
    {
      /* fully connect */
#ifdef OLD_LOG_VERSION
      xrdp_wm_log_msg(self->wm, "sesman connect ok");
#endif
      self->connected_state = 1;
      rv = xrdp_mm_send_login(self);
    }
    else
    {
#ifdef OLD_LOG_VERSION
    	xrdp_wm_log_msg(self->wm, errstr);
#else
    	xrdp_wm_log_error(self->wm, "Connection failed verify:");
    	xrdp_wm_log_error(self->wm, "sesman is not accessible");
#endif
      trans_delete(self->sesman_trans);
      self->sesman_trans = 0;
      self->sesman_trans_up = 0;
      rv = 1;
    }
  }
  else /* no sesman */
  {
    if (xrdp_mm_setup_mod2(self) == 0)
    {
      xrdp_wm_set_login_mode(self->wm, 10);
    }
    if (self->wm->login_mode != 10)
    {
      xrdp_wm_set_login_mode(self->wm, 11);
      xrdp_mm_module_cleanup(self);
    }
  }
  self->sesman_controlled = use_sesman;
  return rv;
}


/*****************************************************************************/
int APP_CC
xrdp_mm_send_disconnect(struct xrdp_mm* self)
{
  int admin_socket;
  struct stream* s;
  char* data = g_malloc(256,1);
  admin_socket = g_unix_connect("/var/spool/xrdp/xrdp_management");
  int size;
  int res = 0;

  make_stream(s);
	init_stream(s, 1024);
  size = g_sprintf(data, "<request type=\"internal\" action=\"disconnect\" username=\"%s\"/>",
  		self->wm->session->client_info->username);
	out_uint32_be(s,size);
	out_uint8p(s, data, size)
	size = s->p - s->data;
	res = g_tcp_send(admin_socket, s->data, size, 0);
	if (res != size)
	{
		g_writeln("Error while sending data");
	}
	free_stream(s);
	g_free(data);
  return 0;
}

int APP_CC
xrdp_mm_end(struct xrdp_mm* self)
{
  lib_userChannel_mod_end((struct userChannel*)self->mod);
  return 0;
}

void APP_CC
xrdp_mm_set_network_stat(struct xrdp_mm* self, long bandwidth, int rtt)
{
  lib_userChannel_set_network_stat((struct userChannel*)self->mod, bandwidth, rtt);
}

/*****************************************************************************/
void APP_CC
xrdp_mm_set_static_framerate(struct xrdp_mm* self, int framerate)
{
  lib_userChannel_set_static_framerate((struct userChannel*)self->mod, framerate);
}

/*****************************************************************************/
int APP_CC
xrdp_mm_get_wait_objs(struct xrdp_mm* self,
                      tbus* read_objs, int* rcount,
                      tbus* write_objs, int* wcount, int* timeout)
{
  int rv;

  if (self == 0)
  {
    return 0;
  }
  rv = 0;
  if ((self->sesman_trans != 0) && self->sesman_trans_up)
  {
    trans_get_wait_objs(self->sesman_trans, read_objs, rcount, timeout);
  }
//  if ((self->vc != 0))
//  {
//    trans_get_wait_objs(self->chan_trans, read_objs, rcount, timeout);
//  }
  if ((self->scim_trans != 0) && self->scim_trans_up)
  {
    trans_get_wait_objs(self->scim_trans, read_objs, rcount, timeout);
  }
  if (self->connected)
  {
    if (self->mod && self->mod->mod_get_wait_objs != 0)
    {
      rv = self->mod->mod_get_wait_objs(self->mod, read_objs, rcount,
                                        write_objs, wcount, timeout);
    }
  }
  return rv;
}

/*****************************************************************************/
int APP_CC
xrdp_mm_check_wait_objs(struct xrdp_mm* self)
{
  int rv;

  if (self == 0)
  {
    return 0;
  }
  rv = 0;
  if ((self->sesman_trans != 0) && self->sesman_trans_up)
  {
    if (trans_check_wait_objs(self->sesman_trans) != 0)
    {
      self->delete_sesman_trans = 1;
    }
  }
  if ((self->scim_trans != 0))
  {
    if (! self->scim_trans_up)
    {
      /* try to connect */
      char socket_file[255];
      g_snprintf(socket_file, 255, "/var/spool/xrdp/xrdp_scim_socket_%d", 7200 + self->display);
      g_writeln("xrdp_mm_check_wait_objs: try to join scim-panel");
      if (trans_connect(self->scim_trans, "127.0.0.1", socket_file, 3000) == 0)
      {
        g_writeln("xrdp_mm_check_wait_objs: scim-panel connection OK");
        self->scim_trans_up = 1;
        self->wm->compose=false;
      }
      else
      {
        g_writeln("xrdp_mm_check_wait_objs: scim-panel connection failed (will retry)");
      }
    }
    else
    {
      if (trans_check_wait_objs(self->scim_trans) != 0)
      {
        self->delete_scim_trans = 1;
      }
    }
  }
  if (self->connected)
  {
    if (self->mod && self->mod->mod_check_wait_objs)
    {
      rv = self->mod->mod_check_wait_objs(self->mod);
    }
  }
  if (self->delete_sesman_trans)
  {
    trans_delete(self->sesman_trans);
    self->sesman_trans = 0;
    self->sesman_trans_up = 0;
    self->delete_sesman_trans = 0;
  }
  if (self->delete_scim_trans)
  {
    trans_delete(self->scim_trans);
    self->scim_trans = 0;
    self->scim_trans_up = 0;
    self->delete_scim_trans = 0;
  }
  return rv;
}


/*****************************************************************************/
int DEFAULT_CC
server_fill_rect(struct userChannel* mod, int x, int y, int cx, int cy)
{
  struct xrdp_wm* wm;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  xrdp_painter_fill_rect(p, wm->screen, x, y, cx, cy);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_screen_blt(struct userChannel* mod, int x, int y, int cx, int cy,
                  int srcx, int srcy)
{
  struct xrdp_wm* wm;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  p->rop = 0xcc;
  xrdp_painter_copy(p, wm->screen, wm->screen, x, y, cx, cy, srcx, srcy, 0);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_paint_update(struct userChannel* mod, int x, int y, int cx, int cy, char* data)
{
  struct xrdp_wm* wm;
  struct xrdp_bitmap* b;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  b = xrdp_bitmap_create_with_data(cx, cy, wm->screen->bpp, data, wm);
  xrdp_wm_send_bitmap(wm, b, x, y, cx, cy);
  xrdp_bitmap_delete(b);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_paint_rect(struct userChannel* mod, int x, int y, int cx, int cy,
                  char* data, int width, int height, int srcx, int srcy, int quality)
{
  struct xrdp_wm* wm;
  struct xrdp_bitmap* b;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  b = xrdp_bitmap_create_with_data(width, height, wm->screen->bpp, data, wm);
  xrdp_painter_copy(p, b, wm->screen, x, y, cx, cy, srcx, srcy, quality);
  xrdp_bitmap_delete(b);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_pointer(struct userChannel* mod, int x, int y,
                   char* data, char* mask)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  xrdp_wm_pointer(wm, data, mask, x, y);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_palette(struct userChannel* mod, int* palette)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  if (g_memcmp(wm->palette, palette, 255 * sizeof(int)) != 0)
  {
    g_memcpy(wm->palette, palette, 256 * sizeof(int));
    xrdp_wm_send_palette(wm);
  }
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_msg(struct userChannel* mod, char* msg, int code)
{
  struct xrdp_wm* wm;

  if (code == 1)
  {
    g_writeln(msg);
    return 0;
  }
  wm = (struct xrdp_wm*)(mod->wm);
#ifdef OLD_LOG_VERSION
  return xrdp_wm_log_msg(wm, msg);
#else
  return xrdp_wm_log_error(wm, msg);
#endif
}

/*****************************************************************************/
int DEFAULT_CC
server_is_term(struct userChannel* mod)
{
  struct xrdp_wm* wm;
  wm = (struct xrdp_wm*)(mod->wm);
  if (wm->user_channel->is_term != NULL)
  {
    return wm->user_channel->is_term();
  }
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_clip(struct userChannel* mod, int x, int y, int cx, int cy)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  return xrdp_painter_set_clip(p, x, y, cx, cy);
}

/*****************************************************************************/
int DEFAULT_CC
server_reset_clip(struct userChannel* mod)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  return xrdp_painter_clr_clip(p);
}

/*****************************************************************************/
int DEFAULT_CC
server_set_fgcolor(struct userChannel* mod, int fgcolor)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->fg_color = fgcolor;
  p->pen.color = p->fg_color;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_bgcolor(struct userChannel* mod, int bgcolor)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->bg_color = bgcolor;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_opcode(struct userChannel* mod, int opcode)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->rop = opcode;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_mixmode(struct userChannel* mod, int mixmode)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->mix_mode = mixmode;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_brush(struct userChannel* mod, int x_orgin, int y_orgin,
                 int style, char* pattern)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->brush.x_orgin = x_orgin;
  p->brush.y_orgin = y_orgin;
  p->brush.style = style;
  g_memcpy(p->brush.pattern, pattern, 8);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_set_pen(struct userChannel* mod, int style, int width)
{
  struct xrdp_painter* p;

  p = (struct xrdp_painter*)(mod->painter);
  p->pen.style = style;
  p->pen.width = width;
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_draw_line(struct userChannel* mod, int x1, int y1, int x2, int y2)
{
  struct xrdp_wm* wm;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  return xrdp_painter_line(p, wm->screen, x1, y1, x2, y2);
}

/*****************************************************************************/
int DEFAULT_CC
server_add_char(struct userChannel* mod, int font, int charactor,
                int offset, int baseline,
                int width, int height, char* data)
{
  struct xrdp_font_char fi;

  fi.offset = offset;
  fi.baseline = baseline;
  fi.width = width;
  fi.height = height;
  fi.incby = 0;
  fi.data = data;
  return libxrdp_orders_send_font(((struct xrdp_wm*)mod->wm)->session,
                                  &fi, font, charactor);
}

/*****************************************************************************/
int DEFAULT_CC
server_draw_text(struct userChannel* mod, int font,
                 int flags, int mixmode, int clip_left, int clip_top,
                 int clip_right, int clip_bottom,
                 int box_left, int box_top,
                 int box_right, int box_bottom,
                 int x, int y, char* data, int data_len)
{
  struct xrdp_wm* wm;
  struct xrdp_painter* p;

  wm = (struct xrdp_wm*)(mod->wm);
  p = (struct xrdp_painter*)(mod->painter);
  return xrdp_painter_draw_text2(p, wm->screen, font, flags,
                                 mixmode, clip_left, clip_top,
                                 clip_right, clip_bottom,
                                 box_left, box_top,
                                 box_right, box_bottom,
                                 x, y, data, data_len);
}

/*****************************************************************************/
int DEFAULT_CC
server_reset(struct userChannel* mod, int width, int height, int bpp)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  if (wm->client_info == 0)
  {
    return 1;
  }
  /* older client can't resize */
  if (wm->client_info->build <= 419)
  {
    return 0;
  }
  /* if same, don't need to do anything */
  if (wm->client_info->width == width &&
      wm->client_info->height == height &&
      wm->client_info->bpp == bpp)
  {
    return 0;
  }
  /* reset lib, client_info gets updated in libxrdp_reset */
  if (libxrdp_reset(wm->session, width, height, bpp) != 0)
  {
    return 1;
  }
  /* reset cache */
  xrdp_cache_reset(wm->cache, wm->client_info);
  /* resize the main window */
  xrdp_bitmap_resize(wm->screen, wm->client_info->width,
                     wm->client_info->height);
  /* load some stuff */
  xrdp_wm_load_static_colors(wm);
  xrdp_wm_load_static_pointers(wm);
  return 0;
}

/*****************************************************************************/
int DEFAULT_CC
server_query_channel(struct userChannel* mod, int index, char* channel_name,
                     int* channel_flags)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  if (wm->mm->sesman_controlled)
  {
    return 1;
  }
  return libxrdp_query_channel(wm->session, index, channel_name,
                               channel_flags);
}

/*****************************************************************************/
/* returns -1 on error */
int DEFAULT_CC
server_get_channel_id(struct userChannel* mod, char* name)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  if (wm->mm->sesman_controlled)
  {
    return -1;
  }
  return libxrdp_get_channel_id(wm->session, name);
}

/*****************************************************************************/
int DEFAULT_CC
server_send_to_channel(struct userChannel* mod, int channel_id,
                       char* data, int data_len,
                       int total_data_len, int flags)
{
  struct xrdp_wm* wm;

  wm = (struct xrdp_wm*)(mod->wm);
  if (wm->mm->sesman_controlled)
  {
    return 1;
  }
  return libxrdp_send_to_channel(wm->session, channel_id, data, data_len,
                                 total_data_len, flags);
}
