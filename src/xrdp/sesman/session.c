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
   Copyright (C) 2012 Ulteo SAS http://www.ulteo.com
    Author David LECHEVALIER <david@ulteo.com> 2012
*/

/**
 *
 * @file session.c
 * @brief Session management code
 * @author Jay Sorg, Simone Fedele
 *
 */

#include "sesman.h"
#include "libscp_types.h"
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <config.h>
#include <unistd.h>
#include <sys/mount.h>


//#include <time.h>

#define SCIM_PATH    "/usr/bin/"
#define SCIM_BIN     "scim"
#define SCIM_PANEL_BIN "xrdp-scim-panel"
#define X_LOG_PREFIX "/tmp/X_"
extern tbus g_sync_event;
extern unsigned char g_fixedkey[8];
extern struct config_sesman* g_cfg; /* in sesman.c */
struct session_chain* g_sessions;
int g_session_count;

static int g_sync_width;
static int g_sync_height;
static int g_sync_keylayout;
static int g_sync_use_scim;
static int g_sync_bpp;
static char* g_sync_username;
static char* g_sync_password;
static char* g_sync_domain;
static char* g_sync_program;
static char* g_sync_directory;
static tbus g_sync_data;
static tui8 g_sync_type;
static int g_sync_result;
static int g_sync_client_pid;


/******************************************************************************/
struct session_item* DEFAULT_CC
session_copy(struct session_item* src)
{
  struct session_item* copy;

  if (src == NULL)
  {
    return NULL;
  }
  copy = g_malloc(sizeof(struct session_item), 1);
  if (copy == NULL)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "Internal error: unable to copy a session");
    g_free(copy);
    return NULL;
  }

  copy->display = src->display;
  copy->status = src->status;
  copy->height = src->height;
  copy->width = src->width;
  copy->data = src->data;
  copy->type = src->type;
  copy->bpp = src->bpp;
  copy->pid = src->pid;

  if (src->homedir)
  {
    g_snprintf(copy->homedir, sizeof(copy->homedir), src->homedir);
  }
  if (src->name)
  {
    g_snprintf(copy->name, sizeof(copy->name), src->name);
  }

  return copy;
}


/******************************************************************************/
struct session_item* DEFAULT_CC
session_get_bydata(char* name)
{
  struct session_chain* tmp;

  tmp = g_sessions;

  while (tmp != 0)
  {
    if (g_strncmp(name, tmp->item->name, 255) == 0)
    {
      return session_copy(tmp->item);
    }
    tmp = tmp->next;
  }

  return 0;
}

/******************************************************************************/
/**
 *
 * @brief checks if there's a server running on a display
 * @param display the display to check
 * @return 0 if there isn't a display running, nonzero otherwise
 *
 */
static int DEFAULT_CC
x_server_running_check_ports(int display)
{
  char display_file[256] = {0};
  char display_lock[256] = {0};
  char port_str[256] = {0};
  int x_running = 0;
  int sck = 0;
  int port = 0;

  port = 5900 + display;
  g_sprintf(display_file, "/tmp/.X11-unix/X%d", display);
  g_sprintf(display_lock, "/tmp/.X%d-lock", display);
  g_sprintf(port_str, "%2.2d", port);

  x_running = g_file_exist(display_file);
  if (!x_running)
  {
    x_running = g_file_exist(display_lock);
  }
  if (!x_running) /* check port */
  {
    sck = g_tcp_socket();
    x_running = g_tcp_bind(sck, port_str);
    g_tcp_close(sck);
  }
  return x_running;
}

/******************************************************************************/
/**
 *
 * @brief checks if there's a server running on a display
 * @param display the display to check
 * @return 0 if there isn't a display running, nonzero otherwise
 *
 */
static int DEFAULT_CC
x_server_running(int display)
{
  char text[256];
  int x_running;

  g_sprintf(text, "/tmp/.X11-unix/X%d", display);
  x_running = g_file_exist(text);
  if (!x_running)
  {
    g_sprintf(text, "/tmp/.X%d-lock", display);
    x_running = g_file_exist(text);
  }
  return x_running;
}

/******************************************************************************/
static void DEFAULT_CC
session_start_sessvc(int xpid, int wmpid, long data)
{
  struct list* sessvc_params;
  char wmpid_str[25];
  char xpid_str[25];
  char exe_path[262];
  int i;

  /* new style waiting for clients */
  g_sprintf(wmpid_str, "%d", wmpid);
  g_sprintf(xpid_str, "%d",  xpid);
  log_message(&(g_cfg->log), LOG_LEVEL_INFO,
              "starting xrdp-sessvc - xpid=%s - wmpid=%s",
              xpid_str, wmpid_str);

  sessvc_params = list_create();
  sessvc_params->auto_free = 1;

  /* building parameters */
  g_snprintf(exe_path, 261, "%s/xrdp-sessvc", XRDP_SBIN_PATH);

  list_add_item(sessvc_params, (long)g_strdup(exe_path));
  list_add_item(sessvc_params, (long)g_strdup(xpid_str));
  list_add_item(sessvc_params, (long)g_strdup(wmpid_str));
  list_add_item(sessvc_params, (long)g_strdup(g_sync_username));
  list_add_item(sessvc_params, 0); /* mandatory */

  /* executing sessvc */
  g_execvp(exe_path, ((char**)sessvc_params->items));

  /* should not get here */
  log_message(&(g_cfg->log), LOG_LEVEL_ERROR,
              "error starting xrdp-sessvc - pid %d - xpid=%s - wmpid=%s",
              g_getpid(), xpid_str, wmpid_str);

  /* logging parameters */
  /* no problem calling strerror for thread safety: other threads
     are blocked */
  log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "errno: %d, description: %s",
              errno, g_get_strerror());
  log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "execve parameter list:");
  for (i = 0; i < (sessvc_params->count); i++)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "        argv[%d] = %s", i,
                (char*)list_get_item(sessvc_params, i));
  }
  list_delete(sessvc_params);

  /* keep the old waitpid if some error occurs during execlp */
  g_waitpid(wmpid);
  g_sigterm(xpid);
  g_sigterm(wmpid);
  g_sleep(1000);
  auth_end(data);
  g_exit(0);
}

/******************************************************************************/
/* called with the main thread
   returns boolean */
static int APP_CC
session_is_display_in_chain(int display)
{
  struct session_chain* chain;
  struct session_item* item;

  chain = g_sessions;
  while (chain != 0)
  {
    item = chain->item;
    if (item->display == display)
    {
      return 1;
    }
    chain = chain->next;
  }
  return 0;
}

/******************************************************************************/
/* called with the main thread */
static int APP_CC
session_get_aval_display_from_chain(void)
{
  int display;

  display = 10;
  while ((display - 10) <= g_cfg->sess.max_sessions)
  {
    if (!session_is_display_in_chain(display))
    {
      if (!x_server_running_check_ports(display))
      {
        lock_chain_release();
        return display;
      }
    }
    display++;
  }
  return 0;
}


/******************************************************************************/
char* APP_CC
get_kbcode(int keylayout)
{
  switch(keylayout){
  case 0x40c:
	  return "fr";
	  break;
  case 0x0409:
	  return "us";
	  break;
  case 0x0407:
	  return "de";
	  break;
  case 0x0410:
	  return "it";
	  break;
  case 0x0419:
	  return "ru";
	  break;
  case 0x041d:
	  return "se";
	  break;
  default:
	  log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "the keylayout %04x is unknowned", keylayout);
	  return "us";
  }
}


/******************************************************************************/
static int APP_CC
wait_for_xserver(int display, int can_log)
{
  int i;
  char x_log_file_path[256];

  /* give X a bit to start */
  /* wait up to 10 secs for x server to start */
  int x_waiting_step = (g_cfg->sess.x_session_timeout / 250);
  i = 0;
  while (!x_server_running(display))
  {
    i++;
    if (i > x_waiting_step)
    {
    	if (can_log)
    	{
				g_snprintf(x_log_file_path, sizeof(x_log_file_path), "%s%i.log", X_LOG_PREFIX, display);
				log_message(&(g_cfg->log), LOG_LEVEL_ERROR,
										"X server for display %d startup timeout",
										display);
				log_file(&(g_cfg->log), LOG_LEVEL_ERROR, x_log_file_path);
    	}
      return 1;
    }
    g_sleep(250);
  }
  return 0;
}

/******************************************************************************/
int APP_CC
user_can_launch_program(char* username)
{
	char buffer[1024];
	int fd;
	int size;
	g_snprintf(buffer,sizeof(buffer), "%s/%s/%s", XRDP_USER_PREF_DIRECTORY,
			username, "AuthorizeAlternateShell");
	fd = g_file_open(buffer);
	if ( fd < 0)
	{
		/* By default, the user can override his shell */
		return 0;
	}
	size = g_file_read(fd, buffer, 1024);
	buffer[size] = 0;
	if(g_strncmp(buffer, "True", g_strlen("True")) == 0)
	{
		g_file_close(fd);
		return 0;
	}
	g_file_close(fd);
	return 1;
}

/******************************************************************************/
int APP_CC
get_user_shell(char* username, char* shell)
{
	char buffer[1024];
	int fd;
	int size;
	g_snprintf(buffer,sizeof(buffer), "%s/%s/%s", XRDP_USER_PREF_DIRECTORY, username, "DefaultShell");
	fd = g_file_open(buffer);
	if ( fd < 0)
	{
		return 1;
	}
	size = g_file_read(fd, shell, 1024);
	if (size > 0)
	{
		shell[size] = 0;
		g_file_close(fd);
		return 0;
	}
	g_file_close(fd);
	return 1;
}


/******************************************************************************/
/* called with the main thread */
static int APP_CC
session_start_fork(int width, int height, int bpp, char* username,
                   char* password, tbus data, tui8 type, char* domain,
                   char* program, char* directory, int keylayout, int client_pid,
                   int use_scim)
{
  int display;
  int pid;
  int wmpid;
  int xpid;
  int i;
  char geometry[32] = {0};
  char depth[32] = {0};
  char screen[32] = {0};
  char text[256] = {0};
  char x_log_file_path[256] = {0};
  char session_spool_dir[256];
  char passwd_file[256] = {0};
  char** pp1;
  struct session_chain* temp;
  struct list* xserver_params=0;
  time_t ltime;
  struct tm stime;
  char* kb_util = "/usr/bin/setxkbmap";
  char default_shell[256] = {0};
  char user_dir[1024] = {0};

  /* check to limit concurrent sessions */
  if (g_session_count >= g_cfg->sess.max_sessions)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_INFO, "max concurrent session limit "
                "exceeded. login for user %s denied", username);
    return 0;
  }

  temp = (struct session_chain*)g_malloc(sizeof(struct session_chain), 0);
  if (temp == 0)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "cannot create new chain "
                "element - user %s", username);
    return 0;
  }
  temp->item = (struct session_item*)g_malloc(sizeof(struct session_item), 0);
  if (temp->item == 0)
  {
    g_free(temp);
    log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "cannot create new session "
                "item - user %s", username);
    return 0;
  }
  display = session_get_aval_display_from_chain();
  if (display == 0)
  {
    g_free(temp->item);
    g_free(temp);
    return 0;
  }

  if(get_user_shell(username, default_shell) != 0)
  {
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_start_fork]: "
					"no alternative shell declared");
  	default_shell[0] = 0;
  }
  else
  {
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_start_fork]: "
					"alternative shell : %s",default_shell);
		program = 0;
  }
  if( user_can_launch_program(username) == 1)
  {
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_start_fork]: "
					"user can not override his shell");
  	default_shell[0] = 0;
  	program = 0;
  }

  g_getuser_info(username, 0, 0, 0, user_dir, 0);
  if (user_dir == NULL || user_dir[0] == 0)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Unable to create session for an user without homedir ");
    return 1;
  }

  g_snprintf(x_log_file_path, sizeof(x_log_file_path), "%s%i.log", X_LOG_PREFIX, display);
  g_file_delete(x_log_file_path);
	if (g_file_exist(x_log_file_path))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"sesman[session_start_fork]: "
				"Unable to delete last X log file %s", x_log_file_path);
		return 0;
	}

  g_snprintf(session_spool_dir, sizeof(session_spool_dir), "%s/%i", "/var/spool/xrdp", display);
  g_remove_dirs(session_spool_dir);
  g_mkdir(session_spool_dir);
  if (! g_directory_exist(session_spool_dir))
  {
    log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"sesman[session_start_fork]: "
        "Unable to delete xrdp session spool directory %s: %s", session_spool_dir, g_get_strerror());
    return 0;
  }

  g_chown(session_spool_dir, username);

  wmpid = 0;
  pid = g_fork();
  if (pid == -1)
  {
  }
  else if (pid == 0) /* child sesman */
  {
    auth_start_session(data, display);
    g_sprintf(geometry, "%dx%d", width, height);
    g_sprintf(depth, "%d", bpp);
    g_sprintf(screen, ":%d", display);
    wmpid = g_fork();
    if (wmpid == -1)
    {
    }
    else if (wmpid == 0) /* child (child sesman) xserver */
    {
      if (wait_for_xserver(display, 1) == 1)
      {
      	g_exit(0);
      }
      env_set_user(username, 0, display);
      if (x_server_running(display))
      {
//    	  int pid2 = g_fork();
//    	  if(pid2 == 0){
//    	    g_execlp3(kb_util, kb_util, get_kbcode(keylayout));
//    	    g_exit(0);
//    	  }

        if (use_scim)
        {
          int error = 0;
          char user_scim_files[256];
          char scim_socket_buffer[512];

          g_snprintf(scim_socket_buffer, sizeof(scim_socket_buffer), "local:%s/scim-socket", session_spool_dir);
          g_setenv("SCIM_SOCKET_ADDRESS", scim_socket_buffer, 1);

          g_snprintf(scim_socket_buffer, sizeof(scim_socket_buffer), "local:%s/scim-panel-socket", session_spool_dir);
          g_setenv("SCIM_PANEL_SOCKET_ADDRESS", scim_socket_buffer, 1);

          g_snprintf(scim_socket_buffer, sizeof(scim_socket_buffer), "local:%s/scim-helper-manager-socket", session_spool_dir);
          g_setenv("SCIM_HELPER_MANAGER_SOCKET_ADDRESS", scim_socket_buffer, 1);

          /* Push config */
          g_snprintf(user_scim_files, sizeof(user_scim_files), "%s/.scim/", user_dir);
          if (! g_directory_exist(user_scim_files))
          {
            g_mkdirs(user_scim_files);
          }

          g_snprintf(user_scim_files, sizeof(user_scim_files), "%s/.scim/config", user_dir);
          g_file_delete(user_scim_files);
          if (g_file_copy("/etc/xrdp/scim/config", user_scim_files) == -1)
          {
            error = 1;
          }

          g_snprintf(user_scim_files, sizeof(user_scim_files), "%s/.scim/global", user_dir);
          g_file_delete(user_scim_files);
          if (g_file_copy("/etc/xrdp/scim/global", user_scim_files) == -1)
          {
            error = 1;
          }

          if (!error)
          {
            int scimpid;
            struct list* scim_params = list_create();
            scim_params->auto_free = 1;
            list_add_item(scim_params, (long)g_strdup(SCIM_PATH SCIM_BIN));
            list_add_item(scim_params, (long)g_strdup("-e"));
            list_add_item(scim_params, (long)g_strdup("xrdp"));
            list_add_item(scim_params, 0);

            scimpid = g_launch_process(display, scim_params, 1);

            list_delete(scim_params);

            if (scimpid == 0) /* Error */
            {
              log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"error : unable to fork for scim");
              /* no scim started, no env pushed */
            } else { /* fork successfull */
              int panelpid;
              struct list* panel_params = list_create();
              panel_params->auto_free = 1;
              list_add_item(panel_params, (long)g_strdup(SCIM_PANEL_BIN));
              list_add_item(panel_params, 0);

              panelpid = g_launch_process(display, panel_params, 1);

              list_delete(panel_params);

              if (panelpid == 0) /* Error */
              {
                log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"sesman[session_start_fork]: Unable to fork for scim-panel");
              } else { /* fork successfull */
                /* Push scim env */
                g_setenv("XMODIFIERS", "@im=SCIM", 1);
                g_setenv("GTK_IM_MODULE", SCIM_BIN, 1);
                g_setenv("QT_IM_MODULE", SCIM_BIN, 1);
                log_message(&(g_cfg->log), LOG_LEVEL_INFO,"sesman[session_start_fork]: Scim env initialized");
              }
            }
          }
          else
          {
            log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"error : unable to initialize scim.");
          }
        }

        auth_set_env(data);

        if (directory != 0)
        {
          if (directory[0] != 0)
          {
            g_set_current_dir(directory);
          }
        }
        /* try to execute user window manager if enabled */
        g_sprintf(text, "%s/%s", XRDP_CFG_PATH, g_cfg->default_wm);
        if (program !=0)
        {
        	g_strcpy(default_shell, program);
        }
        log_message(&(g_cfg->log), LOG_LEVEL_INFO,"sesman[session_start_fork]: "
							"default shell : '%s'",default_shell);
				g_execlp3(text, g_cfg->default_wm, default_shell);
				log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"error starting user "
            		"wm for user %s - pid %d", username, g_getpid());
            /* logging parameters */
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "errno: %d, "
								"description: %s", errno, g_get_strerror());
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG,"execlp3 parameter "
								"list:");
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "        argv[0] = %s",
								text);
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "        argv[1] = %s",
								g_cfg->user_wm);
      }
      log_message(&(g_cfg->log), LOG_LEVEL_DEBUG,"aborting connection...");
      g_exit(0);
    }
    else /* parent (child sesman) */
    {
      xpid = g_fork();
      if (xpid == -1)
      {
      }
      else if (xpid == 0) /* child */
      {
        int x_log_file;
        env_set_user(username, passwd_file, display);
        env_check_password_file(passwd_file, password);
        if (type == SESMAN_SESSION_TYPE_XVNC)
        {
          x_log_file = g_file_open(x_log_file_path);
          if (x_log_file < 0)
          {
            log_message(&(g_cfg->log), LOG_LEVEL_ERROR,"sesman[session_start_fork]: "
            		"Unable to create last X log file %s", x_log_file_path);
            g_exit(0);
          }
          g_file_close(STDERR_FILENO);
          dup2(x_log_file, STDERR_FILENO);
          xserver_params = list_create();
          xserver_params->auto_free = 1;
          /* these are the must have parameters */
          list_add_item(xserver_params, (long)g_strdup("Xvnc"));
          list_add_item(xserver_params, (long)g_strdup(screen));
          list_add_item(xserver_params, (long)g_strdup("-geometry"));
          list_add_item(xserver_params, (long)g_strdup(geometry));
          list_add_item(xserver_params, (long)g_strdup("-depth"));
          list_add_item(xserver_params, (long)g_strdup(depth));
          list_add_item(xserver_params, (long)g_strdup("-rfbauth"));
          list_add_item(xserver_params, (long)g_strdup(passwd_file));

          /* additional parameters from sesman.ini file */
          //config_read_xserver_params(SESMAN_SESSION_TYPE_XVNC,
          //                           xserver_params);
          list_append_list_strdup(g_cfg->vnc_params, xserver_params, 0);

          /* make sure it ends with a zero */
          list_add_item(xserver_params, 0);
          //list_dump_items(xserver_params);
          pp1 = (char**)xserver_params->items;
          g_execvp("Xvnc", pp1);
          log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "Xvnc did not exist on the system");

          g_exit(0);
        }
        else if (type == SESMAN_SESSION_TYPE_XRDP)
        {
          xserver_params = list_create();
          xserver_params->auto_free = 1;
          /* these are the must have parameters */
          list_add_item(xserver_params, (long)g_strdup("X11rdp"));
          list_add_item(xserver_params, (long)g_strdup(screen));
          list_add_item(xserver_params, (long)g_strdup("-geometry"));
          list_add_item(xserver_params, (long)g_strdup(geometry));
          list_add_item(xserver_params, (long)g_strdup("-depth"));
          list_add_item(xserver_params, (long)g_strdup(depth));

          /* additional parameters from sesman.ini file */
          //config_read_xserver_params(SESMAN_SESSION_TYPE_XRDP,
          //                           xserver_params);
	  list_append_list_strdup(g_cfg->rdp_params, xserver_params, 0);

          /* make sure it ends with a zero */
          list_add_item(xserver_params, 0);
          pp1 = (char**)xserver_params->items;
          g_execvp("X11rdp", pp1);
          g_exit(0);
        }
        else
        {
          log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "bad session type - "
                      "user %s - pid %d", username, g_getpid());
          g_exit(1);
        }

        /* should not get here */
        log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "error starting X server "
                    "- user %s - pid %d", username, g_getpid());

        /* logging parameters */
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "errno: %d, description: "
                    "%s", errno, g_get_strerror());
        log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "execve parameter list: "
                    "%d", (xserver_params)->count);

        for (i=0; i<(xserver_params->count); i++)
        {
          log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "        argv[%d] = %s",
                      i, (char*)list_get_item(xserver_params, i));
        }
        list_delete(xserver_params);
        g_exit(1);
      }
      else /* parent (child sesman)*/
      {
        if (wait_for_xserver(display, 0) == 1)
        {
        	g_exit(0);
        }
        g_snprintf(text, 255, "%d", display);
        g_setenv("XRDP_SESSVC_DISPLAY", text, 1);
        g_snprintf(text, 255, ":%d.0", display);
        g_setenv("DISPLAY", text, 1);
        /* new style waiting for clients */
        session_start_sessvc(xpid, wmpid, data);
      }
    }
  }
  else /* parent sesman process */
  {
    if (wait_for_xserver(display, 0) == 1)
    {
      g_free(temp);
      log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "xrdp-sesman[session_start_fork]: "
                            "X server on display %i for the user '%s' did not respond", display, username);
      return 0;
    }
    temp->item->pid = pid;
    temp->item->display = display;
    temp->item->width = width;
    temp->item->height = height;
    temp->item->bpp = bpp;
    temp->item->data = data;
    g_strncpy(temp->item->name, username, 255);
    g_strncpy(temp->item->homedir, user_dir, sizeof(temp->item->homedir));

    ltime = g_time1();
    localtime_r(&ltime, &stime);
    temp->item->connect_time.year = (tui16)(stime.tm_year + 1900);
    temp->item->connect_time.month = (tui8)stime.tm_mon;
    temp->item->connect_time.day = (tui8)stime.tm_mday;
    temp->item->connect_time.hour = (tui8)stime.tm_hour;
    temp->item->connect_time.minute = (tui8)stime.tm_min;
    zero_time(&(temp->item->disconnect_time));
    zero_time(&(temp->item->idle_time));

    temp->item->type=type;
    temp->item->status=SESMAN_SESSION_STATUS_ACTIVE;

    temp->item->client_id_list = list_create();
    list_add_item(temp->item->client_id_list, client_pid);
    log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "xrdp-sesman[session_start_fork]: "
                                "new client pid %i", client_pid);

    lock_chain_acquire();
    temp->next=g_sessions;
    g_sessions=temp;
    g_session_count++;
    lock_chain_release();
  }
  return display;
}


/******************************************************************************/
/* called by a worker thread, ask the main thread to call session_sync_start
   and wait till done */
int DEFAULT_CC
session_start(int width, int height, int bpp, char* username, char* password,
              long data, tui8 type, char* domain, char* program,
              char* directory, int keylayout, int client_pid, int use_scim)
{
  int display;

  /* lock mutex */
  lock_sync_acquire();
  /* set shared vars */
  g_sync_width = width;
  g_sync_height = height;
  g_sync_bpp = bpp;
  g_sync_username = username;
  g_sync_password = password;
  g_sync_domain = domain;
  g_sync_program = program;
  g_sync_directory = directory;
  g_sync_data = data;
  g_sync_type = type;
  g_sync_keylayout = keylayout;
  g_sync_client_pid = client_pid;
  g_sync_use_scim = use_scim;
  /* set event for main thread to see */
  g_set_wait_obj(g_sync_event);
  /* wait for main thread to get done */
  lock_sync_sem_acquire();
  /* read result(display) from shared var */
  display = g_sync_result;
  /* unlock mutex */
  lock_sync_release();
  return display;
}

/******************************************************************************/
/* called with the main thread */
int APP_CC
session_sync_start(void)
{
  lock_chain_acquire();
  g_sync_result = session_start_fork(g_sync_width, g_sync_height, g_sync_bpp,
                                     g_sync_username, g_sync_password,
                                     g_sync_data, g_sync_type, g_sync_domain,
                                     g_sync_program, g_sync_directory, g_sync_keylayout,
                                     g_sync_client_pid, g_sync_use_scim);
  lock_chain_release();
  lock_sync_sem_release();
  return g_sync_result;
}

/******************************************************************************/
int APP_CC
session_test_line(int fd, int *file_pos)
{
	int size = 0;
	char buffer[1024] = {0};
	char* p = NULL;
	char* endline = NULL;

	size = g_file_read(fd, buffer, 1024);
	if( size < XRDP_TAG_LEN)
	{
		return -1;
	}
	buffer[size] = 0;

	endline = g_strchr(buffer, '\0');
	if (endline == NULL || endline == buffer)
	{
		return -1;
	}

	if( g_strstr(buffer, XRDP_TAG ) != NULL)
	{
		return 0;
	}
	else
	{
		*file_pos += endline + 1 - buffer;
	}
	return 1;
}

/******************************************************************************/
int APP_CC
session_is_tagged(int pid)
{
	char environ_filename[1024];
	char* buffer;
	char* p;
	int fd;
	int file_pos = 0;
	int res;
	int size;

	size = g_sprintf(environ_filename, "/proc/%i/environ",pid);
	environ_filename[size] = 0;
	fd = g_file_open(environ_filename);
	if( fd < 1)
	{
		return 1;
	}
	while ((res = session_test_line(fd, &file_pos)) != -1)
	{
		if(res == 0)
		{
			g_file_close(fd);
			return 0;
		}

		if (g_file_seek(fd, file_pos) < 0)
		{
			break;
		}
	}
	g_file_close(fd);
	return 1;

}

int DEFAULT_CC
session_unmount_drive(struct session_item* sess)
{
	char path[1024] = {0};
	char user_dir[1024] = {0};
	int pid = 0;

	if (sess == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Unable to destroy an empty session");
		return 1;
	}
	if (sess->name == NULL || sess->name[0] == 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Unable to destroy session for an empty user");
		return 1;
	}
	if (sess->homedir == NULL || sess->homedir[0] == 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "User '%' dont have a userdir", sess->name);
		return 1;
	}

	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "Verify mounted drive for user %s on %s", sess->name, sess->homedir);
	g_snprintf(path, 1024, "%s/%s", sess->homedir, RDPDRIVE_NAME );
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "Try to unmount the path %s", path);

	pid = g_fork();
	if (pid == -1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "Unable to start unmount %s [%s]", path, strerror(errno));
		return 1;
	}
	if (pid == 0)
	{
		//child process
		g_execlp3(UMOUNT_UTILS, UMOUNT_UTILS, path);
		g_exit(0);
	}
	g_waitpid(pid);

	g_snprintf(path, 1024, "%s/%s", sess->homedir, GVFSDRIVE_NAME );
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "Try to unmount the path %s", path);

	pid = g_fork();
	if (pid == -1)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "Unable to start unmount %s [%s]", path, strerror(errno));
		return 1;
	}
	if (pid == 0)
	{
		//child process
		g_execlp3(UMOUNT_UTILS, UMOUNT_UTILS, path);
		g_exit(0);
	}
	g_waitpid(pid);
//	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "Remove mount point: %s", path);
//	g_remove_dirs(path);
//	if (g_directory_exist(path))
//	{
//		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Failed to remove mount point: %s %s", path, strerror(errno));
//		return 1;
//	}
	return 0;
}

/******************************************************************************/
int DEFAULT_CC
session_get_X_pid(int display)
{
	char x_temp_file[256] = {0};
	char buffer[50] = {0};
	int fd = 0;
	int error = 0;

	g_snprintf(x_temp_file, sizeof(x_temp_file), "/tmp/.X%i-lock", display);

	if (! g_file_exist(x_temp_file))
	{
		return 0;
	}

	fd = g_file_open(x_temp_file);
	if (fd < 0)
	{
		return 0;
	}

	error = g_file_read(fd, buffer, sizeof(buffer));
	if (error < 0)
	{
		return 0;
	}

	g_strtrim(buffer, 3);
	return atoi(buffer);
}

/******************************************************************************/
int DEFAULT_CC
session_clean_display(struct session_item* sess)
{
	char x_temp_file[256] = {0};
	int display = sess->display;
	int pid = 0;
	int try_count = 5;

	pid = session_get_X_pid(display);
	if (pid == 0)
	{
		return 0;
	}
	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_clean_display]: "
			"Pid of the X server: %i", pid);

	while (g_testpid(pid) >= 0 && try_count > 0)
	{
		g_sigterm(pid);
		g_sleep(100);
		try_count --;
	}

	g_snprintf(x_temp_file, sizeof(x_temp_file), "/tmp/.X11-unix/X%i", display);
	if (g_file_exist(x_temp_file))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_clean_display]: "
				"Removing the X temp file: %s", x_temp_file);
		if (g_file_delete(x_temp_file) == 0)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[session_clean_display]: "
					"Unable to remove: %s", x_temp_file);
		}
	}

	g_snprintf(x_temp_file, sizeof(x_temp_file), "/tmp/.X%i-lock", display);
	if (g_file_exist(x_temp_file))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_clean_display]: "
				"Removing the X temp file: %s", x_temp_file);
		if (g_file_delete(x_temp_file) == 0)
		{
			log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "sesman[session_clean_display]: "
					"Unable to remove: %s", x_temp_file);
		}
	}
	return 0;
}

/******************************************************************************/
int DEFAULT_CC
session_destroy(struct session_item* sess)
{
	int pid = 0;
	int uid = 0;
	char path[1024] = {0};
	struct dirent *dir_entry = NULL;
	struct stat st;
	int i = 0;
	DIR *dir;
	char session_spool_dir[256];

	if (sess == NULL)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Unable to destroy a NULL session");
		return 1;
	}

	if (sess->name == NULL || sess->name[0] == 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_WARNING, "Unable to destroy session for an empty user");
		return 1;
	}
	g_getuser_info(sess->name, 0, &uid, 0, 0, 0);
	if( uid == 0)
	{
//    log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "Unable to kill root processus "
//                "or user did not exist");

		return;
	}


	for(i=0 ; i<2 ; i++)
	{
		dir = opendir("/proc" );
		if( dir == NULL)
		{
			return 0;
		}
		while ((dir_entry = readdir(dir)) != NULL)
		{
			if( 	 (g_strcmp(dir_entry->d_name, ".") == 0)
					|| (g_strcmp(dir_entry->d_name, "..") == 0)
					|| (dir_entry->d_type != DT_DIR))
			{
				continue;
			}
			g_sprintf(path, "%s/%s", "/proc", dir_entry->d_name);
			st.st_uid = 0;
			if( stat(path, &st) == -1 )
			{
				continue;
			}
			if(st.st_uid == uid)
			{
				pid = g_atoi(dir_entry->d_name);
				if (pid != 0 && session_is_tagged(pid) == 0)
				{
					kill(pid, i==0 ? SIGTERM : SIGKILL);
				}
			}
		}
		closedir(dir);
		g_sleep(100);
	}
	session_unmount_drive(sess);
	session_clean_display(sess);
	
	g_snprintf(session_spool_dir, sizeof(session_spool_dir), "%s/%i", "/var/spool/xrdp", sess->display);
	if (g_directory_exist(session_spool_dir))
	{
		g_remove_dirs(session_spool_dir);
	}
	
	//g_free(dir);
	return 0;
}

/******************************************************************************/
int DEFAULT_CC
session_kill(int pid)
{
  struct session_chain* tmp;
  struct session_chain* prev;
  /*THREAD-FIX require chain lock */
  //lock_chain_acquire();
  tmp=g_sessions;
  prev=0;

  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "pid %d is null!", pid);
      if (prev == 0)
      {
        /* prev does no exist, so it's the first element - so we set
           g_sessions */
        g_sessions = tmp->next;
      }
      else
      {
        prev->next = tmp->next;
      }
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return SESMAN_SESSION_KILL_NULLITEM;
    }

    if (tmp->item->pid == pid)
    {
      /* deleting the session */
      session_destroy(tmp->item);
      log_message(&(g_cfg->log), LOG_LEVEL_INFO, "session %d - user %s - "
                  "terminated", tmp->item->display, tmp->item->name);
      list_delete(tmp->item->client_id_list);
      g_free(tmp->item);
      if (prev == 0)
      {
        /* prev does no exist, so it's the first element - so we set
           g_sessions */
        g_sessions = tmp->next;
      }
      else
      {
        prev->next = tmp->next;
      }
      g_free(tmp);
      g_session_count--;
      //g_waitpid(tmp->item->name);
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return SESMAN_SESSION_KILL_OK;
    }

    /* go on */
    prev = tmp;
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
  lock_chain_release();
  return SESMAN_SESSION_KILL_NOTFOUND;
}

/******************************************************************************/
int DEFAULT_CC
session_kill_by_display(int display)
{
  struct session_chain* tmp;
  struct session_chain* prev;

  /*THREAD-FIX require chain lock */
  lock_chain_acquire();

  tmp=g_sessions;
  prev=0;

  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "display %d is null!", display);
      if (prev == 0)
      {
        /* prev does no exist, so it's the first element - so we set
           g_sessions */
        g_sessions = tmp->next;
      }
      else
      {
        prev->next = tmp->next;
      }
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return SESMAN_SESSION_KILL_NULLITEM;
    }

    if (tmp->item->display == display)
    {
      /* deleting the session */
      log_message(&(g_cfg->log), LOG_LEVEL_INFO, "session %d - user %s - "
                  "terminated", tmp->item->display, tmp->item->name);
      session_destroy(tmp->item);
      list_delete(tmp->item->client_id_list);
      g_free(tmp->item);
      if (prev == 0)
      {
        /* prev does no exist, so it's the first element - so we set
           g_sessions */
        g_sessions = tmp->next;
      }
      else
      {
        prev->next = tmp->next;
      }
      g_free(tmp);
      g_session_count--;
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return SESMAN_SESSION_KILL_OK;
    }

    /* go on */
    prev = tmp;
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
  lock_chain_release();
  return SESMAN_SESSION_KILL_NOTFOUND;
}

/******************************************************************************/
void DEFAULT_CC
session_sigkill_all()
{
  struct session_chain* tmp;

  /*THREAD-FIX require chain lock */
//  lock_chain_acquire();

  tmp=g_sessions;

  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "found null session "
                  "descriptor!");
    }
    else
    {
      session_destroy(tmp->item);
    }

    /* go on */
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
//  lock_chain_release();
}

/******************************************************************************/
struct session_item* DEFAULT_CC
session_get_bypid(int pid)
{
  struct session_chain* tmp;
  struct session_item* dummy;

  dummy = g_malloc(sizeof(struct session_item), 1);
  if (0 == dummy)
  {
    log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "internal error", pid);
    return 0;
  }

  /*THREAD-FIX require chain lock */
  lock_chain_acquire();

  tmp = g_sessions;
  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "pid %d is null!", pid);
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return 0;
    }
    if (tmp->item->pid == pid)
    {
      /*THREAD-FIX release chain lock */
      g_memcpy(dummy, tmp->item, sizeof(struct session_item));
      lock_chain_release();
      /*return tmp->item;*/
      return dummy;
    }
    /* go on */
    tmp=tmp->next;
  }
  /*THREAD-FIX release chain lock */
  lock_chain_release();
  return 0;
}

/******************************************************************************/
struct session_item* DEFAULT_CC
session_get_by_display(int display)
{
  struct session_chain* tmp;
  struct session_item* dummy;

  /*THREAD-FIX require chain lock */
//  lock_chain_acquire();

  tmp = g_sessions;
  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "display %d is null!", display);
      /*THREAD-FIX release chain lock */
//      lock_chain_release();
      return 0;
    }
    if (tmp->item->display == display)
    {
      /*THREAD-FIX release chain lock */
//      lock_chain_release();
      /*return tmp->item;*/
      return session_copy(tmp->item);;
    }
    /* go on */
    tmp=tmp->next;
  }
  /*THREAD-FIX release chain lock */
//  lock_chain_release();
  return 0;
}

void DEFAULT_CC
session_monit()
{
	  struct session_chain* tmp;
	  struct session_chain* prev;

	  /*THREAD-FIX require chain lock */
//	  lock_chain_acquire();

	  tmp=g_sessions;
	  prev=0;
  	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[session_monit]: "
  			"Monitoring sessions");
	  while (tmp != 0)
	  {
	    if (tmp->item == 0)
	    {
	      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_monit]: "
	      		"Session descriptor for is null");
	      if (prev == 0)
	      {
	        /* prev does no exist, so it's the first element - so we set
	           g_sessions */
	        g_sessions = tmp->next;
	      }
	      else
	      {
	        prev->next = tmp->next;
	      }
	      /*THREAD-FIX release chain lock */
//	      lock_chain_release();
	      return;
	    }
	    if (tmp->item->status == SESMAN_SESSION_STATUS_TO_DESTROY ||
	    		(tmp->item->status == SESMAN_SESSION_STATUS_DISCONNECTED && g_cfg->sess.kill_disconnected == 1))
	    {
	      /* deleting the session */
	      log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_monit]: "
	      		"Cleanning session %d - user %s", tmp->item->display, tmp->item->name);
	      g_sigterm(tmp->item->pid);
	      if (g_testpid(tmp->item->pid) == tmp->item->pid)
	      {
	      	session_destroy(tmp->item);
	      }
	      list_delete(tmp->item->client_id_list);
	      g_free(tmp->item);
	      if (prev == 0)
	      {
	    	  g_sessions = tmp->next;
	    	  g_free(tmp);
	    	  tmp = g_sessions;
	      }
	      else
	      {
	    	  prev->next = tmp->next;
	    	  g_free(tmp);
	    	  tmp = prev->next;
	      }
	      g_session_count--;
	      continue;
	    }
	    else
	    {
	    	log_message(&(g_cfg->log), LOG_LEVEL_DEBUG_PLUS, "sesman[session_monit]: "
	    			"Inspect session %d - user %s - ", tmp->item->display, tmp->item->name);
	    }
	    /* go on */
	    prev = tmp;
	    tmp=tmp->next;
	  }

	  /*THREAD-FIX release chain lock */
//	  lock_chain_release();
	  return ;
}


/******************************************************************************/
int
session_get_user_display(char* username)
{
  struct session_chain* tmp;

  /*THREAD-FIX require chain lock */
  lock_chain_acquire();

  tmp = g_sessions;
  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "pid %s is null!", username);
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return 0;
    }

    if (g_strcmp(username, tmp->item->name)==0)
    {
    	lock_chain_release();
      return tmp->item->display;
    }

    /* go on */
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
  lock_chain_release();
  return 0;
}



/******************************************************************************/
struct SCP_DISCONNECTED_SESSION*
session_get_byuser(char* user, int* cnt, unsigned char flags)
{
  struct session_chain* tmp;
  struct SCP_DISCONNECTED_SESSION* sess;
  int count;
  int index;

  count=0;

  /*THREAD-FIX require chain lock */
  lock_chain_acquire();

  tmp = g_sessions;
  while (tmp != 0)
  {
    LOG_DBG(&(g_cfg->log), "user: %s", user);
    if ((NULL == user) || (!g_strncasecmp(user, tmp->item->name, 256)))
    {
      LOG_DBG(&(g_cfg->log), "session_get_byuser: status=%d, flags=%d, "
              "result=%d", (tmp->item->status), flags,
              ((tmp->item->status) & flags));
      if ((tmp->item->status) & flags)
      {
        count++;
      }
    }

    /* go on */
    tmp=tmp->next;
  }

  if (count==0)
  {
    (*cnt)=0;
    /*THREAD-FIX release chain lock */
    lock_chain_release();
    return 0;
  }

  /* malloc() an array of disconnected sessions */
  sess=g_malloc(count * sizeof(struct SCP_DISCONNECTED_SESSION),1);
  if (sess==0)
  {
    (*cnt)=0;
    /*THREAD-FIX release chain lock */
    lock_chain_release();
    return 0;
  }

  tmp = g_sessions;
  index = 0;
  while (tmp != 0)
  {
#warning FIXME: we should get only disconnected sessions!
    if ((NULL == user) || (!g_strncasecmp(user, tmp->item->name, 256)))
    {
      if ((tmp->item->status) & flags)
      {
        (sess[index]).SID=tmp->item->pid;
        (sess[index]).type=tmp->item->type;
        (sess[index]).height=tmp->item->height;
        (sess[index]).width=tmp->item->width;
        (sess[index]).bpp=tmp->item->bpp;
#warning FIXME: setting idle times and such
        /*(sess[index]).connect_time.year = tmp->item->connect_time.year;
        (sess[index]).connect_time.month = tmp->item->connect_time.month;
        (sess[index]).connect_time.day = tmp->item->connect_time.day;
        (sess[index]).connect_time.hour = tmp->item->connect_time.hour;
        (sess[index]).connect_time.minute = tmp->item->connect_time.minute;
        (sess[index]).disconnect_time.year = tmp->item->disconnect_time.year;
        (sess[index]).disconnect_time.month = tmp->item->disconnect_time.month;
        (sess[index]).disconnect_time.day = tmp->item->disconnect_time.day;
        (sess[index]).disconnect_time.hour = tmp->item->disconnect_time.hour;
        (sess[index]).disconnect_time.minute = tmp->item->disconnect_time.minute;
        (sess[index]).idle_time.year = tmp->item->idle_time.year;
        (sess[index]).idle_time.month = tmp->item->idle_time.month;
        (sess[index]).idle_time.day = tmp->item->idle_time.day;
        (sess[index]).idle_time.hour = tmp->item->idle_time.hour;
        (sess[index]).idle_time.minute = tmp->item->idle_time.minute;*/
        (sess[index]).conn_year = tmp->item->connect_time.year;
        (sess[index]).conn_month = tmp->item->connect_time.month;
        (sess[index]).conn_day = tmp->item->connect_time.day;
        (sess[index]).conn_hour = tmp->item->connect_time.hour;
        (sess[index]).conn_minute = tmp->item->connect_time.minute;
        (sess[index]).idle_days = tmp->item->idle_time.day;
        (sess[index]).idle_hours = tmp->item->idle_time.hour;
        (sess[index]).idle_minutes = tmp->item->idle_time.minute;
        (sess[index]).status = tmp->item->status;
        index++;
      }
    }

    /* go on */
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
  lock_chain_release();
  (*cnt)=count;
  return sess;
}



/******************************************************************************/
void DEFAULT_CC
session_update_status_by_user(char* user, int status)
{
  struct session_chain* tmp;

  /*THREAD-FIX require chain lock */
  lock_chain_acquire();
  tmp = g_sessions;
  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "session descriptor for "
                  "user %s is null!", user);
      /*THREAD-FIX release chain lock */
      lock_chain_release();
      return ;
    }

    if (g_strcmp(user, tmp->item->name) == 0)
    {
    	if (status == SESMAN_SESSION_STATUS_DISCONNECTED)
    	{
          struct list* l = tmp->item->client_id_list;
          int index;
          if (l->count == 0)
          {
            log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_update_status_by_user]: "
                        "No session to disconnect");
          }

          for (index = 0; index < l->count; index++)
          {
            g_sigterm(list_get_item(l, index));
          }
          list_clear(l);
          session_unmount_drive(tmp->item);
    	}

    	if (status == SESMAN_SESSION_STATUS_TO_DESTROY)
    	{
    		//TODO work on a copy
    		session_destroy(tmp->item);
    	}
      /*THREAD-FIX release chain lock */
    	//char* str2 = session_get_status_string(tmp->item->status);
    	if (tmp->item->status == SESMAN_SESSION_STATUS_TO_DESTROY)
    	{
        lock_chain_release();
        return;
    	}
      tmp->item->status = status;
      lock_chain_release();
      /*return tmp->item;*/
      return ;
    }

    /* go on */
    tmp=tmp->next;
  }

  /*THREAD-FIX release chain lock */
  lock_chain_release();
  return;
}


/******************************************************************************/
void DEFAULT_CC
session_add_client_pid(char* user, int client_pid)
{
  struct session_chain* tmp;
  lock_chain_acquire();
  tmp = g_sessions;
  while (tmp != 0)
  {
    if (tmp->item == 0)
    {
      lock_chain_release();
      return ;
    }

    if (g_strcmp(user, tmp->item->name) == 0)
    {
      list_add_item(tmp->item->client_id_list, client_pid);

      lock_chain_release();
      return ;
    }

    tmp=tmp->next;
  }

  lock_chain_release();
  return;
}


/* list sessions  */
/******************************************************************************/
struct session_item*
session_list_session(int* count)
{
  struct session_chain* tmp;
  struct session_item* sess = g_malloc(sizeof(struct session_item) * g_session_count, 0);
  struct session_item* current_item = NULL;

  *count = 0;
  current_item = sess;
  /*THREAD-FIX require chain lock */
//  lock_chain_acquire();
  tmp = g_sessions;
  while (tmp != 0)
  {
	  log_message(&(g_cfg->log), LOG_LEVEL_DEBUG, "sesman[session_list_session]: "
					"name : %s",tmp->item->name);
	  g_memcpy(current_item, tmp->item, sizeof(struct session_item));
	  current_item++;

	  tmp=tmp->next;
	  *count += 1;
  }

  /*THREAD-FIX release chain lock */
//  lock_chain_release();
  return sess;
}

/******************************************************************************/
char* DEFAULT_CC
session_get_status_string(int i)
{
  switch(i)
  {
  case SESMAN_SESSION_STATUS_ACTIVE:
	  return "ACTIVE";
  case SESMAN_SESSION_STATUS_DISCONNECTED:
	  return "DISCONNECT";
  case SESMAN_SESSION_STATUS_TO_DESTROY:
  	return "CLOSING";
  case SESMAN_SESSION_STATUS_UNKNOWN:
  	return "UNKNOWN";
  default:
	  return "UNKNOWN";
  }
}


/******************************************************************************/
int DEFAULT_CC
session_set_user_pref(char* username, char* key, char* value)
{
	char pref_dir[1024];
	char pref_key_file[1024];
	int fd;

	if (!g_directory_exist(XRDP_TEMP_DIR))
	{
		if (!g_create_dir(XRDP_TEMP_DIR))
		{
			log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_set_user_pref]: "
						"Unable to create %s",XRDP_TEMP_DIR);
			return 1;
		}
	}
	if (!g_directory_exist(XRDP_USER_PREF_DIRECTORY))
	{
		if (!g_create_dir(XRDP_USER_PREF_DIRECTORY))
		{
			log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_set_user_pref]: "
						"Unable to create %s",XRDP_USER_PREF_DIRECTORY);
			return 1;
		}
	}

	g_sprintf(pref_dir, "%s/%s",XRDP_USER_PREF_DIRECTORY, username);
	if (!g_directory_exist(pref_dir))
	{
		if (!g_create_dir(pref_dir))
		{
			log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_set_user_pref]: "
						"Unable to create %s", pref_dir);
			return 1;
		}
	}
	g_sprintf(pref_key_file, "%s/%s/%s",XRDP_USER_PREF_DIRECTORY, username,key);
	/* set value */
	g_file_delete(pref_key_file);
	fd = g_file_open(pref_key_file);
	if( fd < 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_set_user_pref]: "
					"Unable to create %s", pref_key_file);
		return 1;
	}
	g_file_write(fd, value, g_strlen(value));
	g_file_close(fd);
	return 0;
}

/******************************************************************************/
int DEFAULT_CC
session_get_user_pref(char* username, char* key, char* value)
{
	char pref_dir[1024];
	char pref_key_file[1024];
	int fd;

	if (!g_directory_exist(XRDP_TEMP_DIR))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_get_user_pref]: "
					"Unable to access %s", XRDP_TEMP_DIR);
		g_strcpy(value, "");
		return 0;
	}

	g_sprintf(pref_dir, "%s/%s",XRDP_USER_PREF_DIRECTORY, username);
	if (!g_directory_exist(XRDP_USER_PREF_DIRECTORY))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_get_user_pref]: "
					"Unable to access %s", XRDP_USER_PREF_DIRECTORY);
		g_strcpy(value, "");
		return 0;
	}
	if (!g_directory_exist(pref_dir))
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_get_user_pref]: "
					"Unable to access %s", pref_dir);
		g_strcpy(value, "");
		return 0;
	}
	g_sprintf(pref_key_file, "%s/%s/%s",XRDP_USER_PREF_DIRECTORY, username, key);
	/* get value */
	fd = g_file_open(pref_key_file);
	if( fd < 0)
	{
		log_message(&(g_cfg->log), LOG_LEVEL_ERROR, "sesman[session_get_user_pref]: "
						"Unable to open file %s", pref_key_file);
		g_strcpy(value, "");
		return 0;
	}
	int size;
	size = g_file_read(fd, value, 1024);
	value[size] = 0;
	g_file_close(fd);
	return 0;
}

void DEFAULT_CC
session_switch_resolution(int width, int height, int display) {
   char resolution_path[512];
   log_message(&(g_cfg->log), LOG_LEVEL_INFO, "sesman[session_switch_resolution]: switch resolution to %dx%d on display :%d", width, height, display);

   g_snprintf(resolution_path, 256, "%s/%s %d %d :%d.0", XRDP_SBIN_PATH, "xrdp-switch-resolution",  width, height, display);
   system(resolution_path);
}
