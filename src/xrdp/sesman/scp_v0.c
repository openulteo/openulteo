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
   Copyright (C) Jay Sorg 2005-2008
*/

/**
 *
 * @file scp_v0.c
 * @brief scp version 0 implementation
 * @author Jay Sorg, Simone Fedele
 *
 */

#include "sesman.h"
#include <verify_user_pam.h>

extern struct config_sesman* g_cfg; /* in sesman.c */
static tbus session_creation_lock;

#ifdef CHECK_PREMIUM_EDITION
extern long last_time_premium_edition_check;
#include "check_premium.h"
#include "os_calls.h"
#endif // CHECK_PREMIUM_EDITION

void DEFAULT_CC
scp_init_mutex()
{
	session_creation_lock = tc_mutex_create();
}

void DEFAULT_CC
scp_remove_mutex()
{
	tc_mutex_delete(session_creation_lock);
}

/******************************************************************************/
void DEFAULT_CC
scp_v0_process(struct SCP_CONNECTION* c, struct SCP_SESSION* s)
{
	int display = 0;
	tbus data;
	struct session_item* s_item;

	tc_mutex_lock(session_creation_lock);
	data = auth_userpass(NULL, s->username, s->password);

	#ifdef CHECK_PREMIUM_EDITION
	bool valid = true;
	if (get_module_version(get_module_name()) & PREMIUM_EDITION) {
	  printf("%s %i %i  %i \n", __FUNCTION__, g_time3(), last_time_premium_edition_check, CHECK_INTERVAL);
	  if (((g_time3() - last_time_premium_edition_check) > CHECK_INTERVAL) ||  last_time_premium_edition_check == 0) {
	    printf("%s FOFOFOOF\n", __FUNCTION__);
	    valid = check_premium_edition();
	  }
	}
	if (!valid) {
	  data = 0;
	  scp_v0s_deny_connection(c, "Unable to launch the session\nInvalid License\nPlease contact your administrator\n");
	  tc_mutex_unlock(session_creation_lock);
	  return;
	}
#endif

	if (data == 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "User %s failed to authenticate", s->username);
		scp_v0s_deny_connection(c, "Your username or \nyour password is invalid");
		tc_mutex_unlock(session_creation_lock);
		return;
	}
	lock_chain_acquire();
	s_item = session_get_bydata(s->username);
	lock_chain_release();

	if (s_item != 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_INFO, "A session for User %s already exist", s->username);
		display = s_item->display;
		if (s_item->status == SESMAN_SESSION_STATUS_TO_DESTROY)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Session for user %s is in destroy, unable to initialize a new session", s->username);
			scp_v0s_deny_connection(c, "Your last session is currently \nended, retry later");
		}
		else
		{
			session_update_status_by_user(s_item->name, SESMAN_SESSION_STATUS_ACTIVE);
			log_message(&(g_cfg->log), LOG_LEVEL_INFO, "switch from status DISCONNECTED to ACTIVE");
			session_switch_resolution(s->width, s->height, display);
			session_add_client_pid(s_item->name, s->client_pid);

			scp_v0s_allow_connection(c, display);
		}

		auth_end(data);
		tc_mutex_unlock(session_creation_lock);
		return;
	}
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "No session already started for the user %s", s->username);
	if (access_login_allowed(s->username) == 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "User %s is not allow to start session", s->username);
		display = 0;
		scp_v0s_deny_connection(c, "You are not allowed\nto start a session\n");

		auth_end(data);
		tc_mutex_unlock(session_creation_lock);
		return;
	}

	log_message(&(g_cfg->log), LOG_LEVEL_INFO, "granted TS access to user %s", s->username);
	if (SCP_SESSION_TYPE_XVNC == s->type)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_INFO, "starting Xvnc session for the user %s ...", s->username);
		display = session_start(s->width, s->height, s->bpp, s->username,
				s->password, data, SESMAN_SESSION_TYPE_XVNC,
				s->domain, s->program, s->directory, s->keylayout, s->client_pid, s->use_scim);
	}
	else
	{
		log_message(&(g_cfg->log), LOG_LEVEL_INFO, "starting X11rdp session for the user %s ...", s->username);
		display = session_start(s->width, s->height, s->bpp, s->username,
				s->password, data, SESMAN_SESSION_TYPE_XRDP,
				s->domain, s->program, s->directory, s->keylayout, s->client_pid, s->use_scim);
	}

	auth_end(data);
	if (display == 0)
	{
		data = 0;
		scp_v0s_deny_connection(c, "Unable to launch the session\nPlease contact\nyour administrator\n");
	}
	else
	{
		scp_v0s_allow_connection(c, display);
	}

	tc_mutex_unlock(session_creation_lock);
}

