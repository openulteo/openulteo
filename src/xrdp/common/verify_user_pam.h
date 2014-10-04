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

#ifndef VERIFY_USER_PAM
#define VERIFY_USER_PAM

#include "arch.h"
#include "os_calls.h"

#include <stdio.h>
#include <security/pam_appl.h>


struct t_user_pass
{
  char user[256];
  char pass[256];
};

struct t_auth_info
{
  struct t_user_pass user_pass;
  int session_opened;
  int did_setcred;
  struct pam_conv pamc;
  pam_handle_t* ph;
};



long APP_CC
auth_userpass(const char* service, char* user, char* pass);
int APP_CC
auth_start_session(long in_val, int in_display);
int APP_CC
auth_end(long in_val);
int APP_CC
auth_set_env(long in_val);

#endif

