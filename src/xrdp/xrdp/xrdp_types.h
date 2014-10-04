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
   http://www.ulteo.com 2013
   Author David LECHEVALIER <david@ulteo.com> 2013
   types

*/

/* lib */

#include "xrdp_vchannel.h"


/* rdp process */
struct xrdp_process
{
  int status;
  struct trans* server_trans; /* in tcp server mode */
  tbus self_term_event;
  struct xrdp_listen* lis_layer; /* owner */
  struct xrdp_session* session;
  /* create these when up and running */
  struct xrdp_user_channel* mod;
  vchannel* vc; /* virtual channel interface */
  struct xrdp_qos* qos;
  //int app_sck;
  tbus done_event;
  int session_id;
  int cont;
};

/* rdp listener */
struct xrdp_listen
{
  int status;
  struct trans* listen_trans; /* in tcp listen mode */
  struct list* process_list;
  tbus pro_done_event;
};

