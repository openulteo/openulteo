/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2013
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
   Copyright (C) Jay Sorg 2004-2009

   main include file

*/

/* include other h files */
#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif
#include "arch.h"
#include "parse.h"
#include "trans.h"
#include "libxrdpinc.h"
#include "xrdp_types.h"
#include "xrdp_constants.h"
#include "defines.h"
#include "os_calls.h"
#include "ssl_calls.h"
#include "thread_calls.h"
#include "list.h"
#include "file.h"
#include "file_loc.h"


//#define OLD_LOG_VERSION		1

/* xrdp.c */
long APP_CC
g_xrdp_sync(long (*sync_func)(long param1, long param2), long sync_param1,
            long sync_param2);
int APP_CC
g_is_term(void);
void APP_CC
g_set_term(int in_val);
tbus APP_CC
g_get_term_event(void);
tbus APP_CC
g_get_sync_event(void);
void APP_CC
g_loop(void);


/* xrdp_process.c */
struct xrdp_process* APP_CC
xrdp_process_create(struct xrdp_listen* owner, tbus done_event);
void APP_CC
xrdp_process_delete(struct xrdp_process* self);
int APP_CC
xrdp_process_main_loop(struct xrdp_process* self);

/* xrdp_listen.c */
struct xrdp_listen* APP_CC
xrdp_listen_create(void);
void APP_CC
xrdp_listen_delete(struct xrdp_listen* self);
int APP_CC
xrdp_listen_main_loop(struct xrdp_listen* self);

bool
xrdp_module_load(struct xrdp_process* self, const char* module_name);
int
xrdp_module_unload(struct xrdp_process* self);

