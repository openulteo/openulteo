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
 * @file verify_user_pam.c
 * @brief Authenticate user using pam
 * @author Jay Sorg
 * 
 */

#include "verify_user_pam.h"

/******************************************************************************/
static int APP_CC
verify_pam_conv(int num_msg, const struct pam_message** msg,
                struct pam_response** resp, void* appdata_ptr)
{
  int i;
  struct pam_response* reply;
  struct t_user_pass* user_pass;

  reply = g_malloc(sizeof(struct pam_response) * num_msg, 1);
  for (i = 0; i < num_msg; i++)
  {
    switch (msg[i]->msg_style)
    {
      case PAM_PROMPT_ECHO_ON: /* username */
        user_pass = appdata_ptr;
        reply[i].resp = g_strdup(user_pass->user);
        reply[i].resp_retcode = PAM_SUCCESS;
        break;
      case PAM_PROMPT_ECHO_OFF: /* password */
        user_pass = appdata_ptr;
        reply[i].resp = g_strdup(user_pass->pass);
        reply[i].resp_retcode = PAM_SUCCESS;
        break;
      default:
        g_printf( "verify_pam_conv: unknown in verify_pam_conv");
        g_free(reply);
        return PAM_CONV_ERR;
    }
  }
  *resp = reply;
  return PAM_SUCCESS;
}

/******************************************************************************/
static void APP_CC
get_service_name(char* service_name)
{
  service_name[0] = 0;
  if (g_file_exist("/etc/pam.d/xrdp-sesman"))
  {
    g_strncpy(service_name, "xrdp-sesman", 255);
  }
  else
  {
    g_strncpy(service_name, "gdm", 255);
  }
}

/******************************************************************************/
/* returns long, zero is no go */
long APP_CC
auth_userpass(const char* service, char* user, char* pass)
{
  int error;
  struct t_auth_info* auth_info;
  char service_name[256] = {0};
  char *user_name = NULL;

  if (service == NULL)
  {
  	get_service_name(service_name);
  }
  else
  {
  	g_strncpy(service_name, service, sizeof(service_name));
  	user_name = user;
  }

  auth_info = g_malloc(sizeof(struct t_auth_info), 1);
  if (pass != NULL)
  {
    g_strncpy(auth_info->user_pass.pass, pass, 255);
  }
  g_strncpy(auth_info->user_pass.user, user, 255);
  auth_info->pamc.conv = &verify_pam_conv;
  auth_info->pamc.appdata_ptr = &(auth_info->user_pass);

  error = pam_start(service_name, user_name, &(auth_info->pamc), &(auth_info->ph));
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_userpass: Pam_start failed: %s\n", pam_strerror(auth_info->ph, error));
    g_free(auth_info);
    return 0;
  }
  error = pam_authenticate(auth_info->ph, 0);
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_userpass: Pam_authenticate failed: %s\n", pam_strerror(auth_info->ph, error));

    g_printf("auth_userpass: Pam_authenticate failed: (%s,%s)\n", user, pass);

    g_free(auth_info);
    return 0;
  }
  error = pam_acct_mgmt(auth_info->ph, 0);
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_userpass: Pam_acct_mgmt failed: %s\n", pam_strerror(auth_info->ph, error));
    g_free(auth_info);
    return 0;
  }
  return (long)auth_info;
}

/******************************************************************************/
/* returns error */
int APP_CC
auth_start_session(long in_val, int in_display)
{
  struct t_auth_info* auth_info;
  int error;
  char display[256] = {0};

  g_sprintf(display, ":%d", in_display);
  auth_info = (struct t_auth_info*)in_val;
  error = pam_set_item(auth_info->ph, PAM_TTY, display);
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_start_session: Pam_set_item failed: %s\n", pam_strerror(auth_info->ph, error));
    return 1;
  }
  error = pam_setcred(auth_info->ph, PAM_ESTABLISH_CRED);
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_start_session: Pam_setcred failed: %s\n", pam_strerror(auth_info->ph, error));
    return 1;
  }
  auth_info->did_setcred = 1;
  error = pam_open_session(auth_info->ph, 0);
  if (error != PAM_SUCCESS)
  {
    g_printf("auth_start_session: Pam_open_session failed: %s\n", pam_strerror(auth_info->ph, error));
    return 1;
  }
  auth_info->session_opened = 1;
  return 0;
}

/******************************************************************************/
/* returns error */
/* cleanup */
int APP_CC
auth_end(long in_val)
{
  struct t_auth_info* auth_info;

  auth_info = (struct t_auth_info*)in_val;
  if (auth_info != 0)
  {
    if (auth_info->ph != 0)
    {
      if (auth_info->session_opened)
      {
        pam_close_session(auth_info->ph, 0);
      }
      if (auth_info->did_setcred)
      {
        pam_setcred(auth_info->ph, PAM_DELETE_CRED);
      }
      if (auth_info->ph) {
        pam_end(auth_info->ph, PAM_SUCCESS);
        auth_info->ph = 0;
      }
    }
  }
  g_free(auth_info);
  return 0;
}

/******************************************************************************/
/* returns error */
/* set any pam env vars */
int APP_CC
auth_set_env(long in_val)
{
  struct t_auth_info* auth_info;
  char** pam_envlist;
  char** pam_env;
  char item[256] = {0};
  char value[256] = {0};
  int eq_pos;

  auth_info = (struct t_auth_info*)in_val;
  if (auth_info != 0)
  {
    /* export PAM environment */
    pam_envlist = pam_getenvlist(auth_info->ph);
    if (pam_envlist != NULL)
    {
      for (pam_env = pam_envlist; *pam_env != NULL; ++pam_env)
      {
        eq_pos = g_pos(*pam_env, "=");
        if (eq_pos >= 0 && eq_pos < 250)
        {
          g_strncpy(item, *pam_env, eq_pos);
          g_strncpy(value, (*pam_env) + eq_pos + 1, 255);
          g_setenv(item, value, 1);
        }
        g_free(*pam_env);
      }
      g_free(pam_envlist);
    }
  }
  return 0;
}

