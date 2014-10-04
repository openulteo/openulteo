/**
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2013
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

#include "xrdp.h"
#include "abstract/xrdp_module.h"


static long DEFAULT_CC
xrdp_module_load_library(long param1, long param2)
{
  long rv;
  char* libname;

  libname = (char*)param1;
  rv = g_load_library(libname);
  return rv;
}

bool
xrdp_module_load(struct xrdp_process* self, const char* module_name)
{
  void* func;
  struct xrdp_wm* wm;
  char text[256];
  if (self == 0)
  {
    return false;
  }
  self->mod = g_malloc(sizeof(struct xrdp_user_channel), 1);

  if (self->mod->handle == 0)
  {
    self->mod->handle = xrdp_module_load_library((long)module_name, 0);
    if (self->mod->handle == 0)
    {
      g_printf("Failed to load library: %s\n", g_get_dlerror());
      return false;
    }
    func = g_get_proc_address(self->mod->handle, "xrdp_module_init");
    if (func == 0)
    {
      func = g_get_proc_address(self->mod->handle, "_xrdp_module_init");
    }
    if (func == 0)
    {
      g_printf("Failed to load function xrdp_module_init: %s\n", g_get_dlerror());
      return false;
    }
    self->mod->init = (bool(*)(struct xrdp_user_channel*))func;
    func = g_get_proc_address(self->mod->handle, "xrdp_module_exit");
    if (func == 0)
    {
      func = g_get_proc_address(self->mod->handle, "_xrdp_module_exit");
    }
    if (func == 0)
    {
      g_printf("Failed to load function xrdp_module_exit %s\n", g_get_dlerror());
      return false;
    }

    self->mod->exit = (int(*)(struct xrdp_user_channel*))func;
    if ((self->mod->init != 0) && (self->mod->exit != 0))
    {
      self->mod->init(self->mod);
      if (self->mod != 0)
      {
        g_writeln("loaded modual '%s' ok", module_name);
      }
      self->mod->is_term = g_is_term;
    }
  }
  return true;
}

int
xrdp_module_unload(struct xrdp_process* self)
{
  if (self->mod->handle && self->mod->exit) {
    return self->mod->exit(self->mod);
  }
  return 0;
}

