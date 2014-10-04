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
   Copyright (C) Jay Sorg 2005-2009
   Copyright (C) 2012-2013 Ulteo SAS
    Author David LECHEVALIER <david@ulteo.com> 2012, 2013
*/

/**
 *
 * @file sessvc.c
 * @brief Session supervisor
 * @author Simone Fedele
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif
#include "file_loc.h"
#include "os_calls.h"
#include "thread_calls.h"
#include "arch.h"
#include "parse.h"

static int g_term = 0;
static int wm_pid;
static int x_pid;

/*****************************************************************************/
void DEFAULT_CC
term_signal_handler(int sig)
{
  g_writeln("xrdp-sessvc: term_signal_handler: got signal %d", sig);
  g_term = 1;
  g_sigterm(wm_pid);
  g_waitpid(wm_pid);

  g_sigterm(x_pid);
  g_waitpid(x_pid);
}

/*****************************************************************************/
int DEFAULT_CC
send_disconnect(char* username)
{
	int admin_socket;
	struct stream* s;
	char* data;
	int size;
	int res = 0;
	int rv = 0;

	admin_socket = g_unix_connect("/var/spool/xrdp/xrdp_management");
	if (admin_socket < 0)
	{
		g_writeln("xrdp-sessvc: Unable to connect to session manager, %s", strerror(g_get_errno()));
		rv = 1;
		return rv;
	}
	make_stream(s);
	init_stream(s, 1024);
	data = g_malloc(1024,1);
	size = g_sprintf(data, "<request type=\"internal\" action=\"logoff\" username=\"%s\"/>",
			username);
	out_uint32_be(s,size);
	out_uint8p(s, data, size);
	size = s->p - s->data;
	res = g_tcp_send(admin_socket, s->data, size, 0);
	if (res != size)
	{
		g_writeln("Error while sending data %s",strerror(g_get_errno()));
		rv = 1;
	}
	free_stream(s);
	g_free(data);
	g_tcp_close(admin_socket);
	return rv;
}

/*****************************************************************************/
void DEFAULT_CC
nil_signal_handler(int sig)
{
  g_writeln("xrdp-sessvc: nil_signal_handler: got signal %d", sig);
}

/******************************************************************************/
int DEFAULT_CC
main(int argc, char** argv)
{
  int ret;
  int lerror;
  char exe_path[262];
  char *username;

  if (argc < 4)
  {
    g_writeln("xrdp-sessvc: exiting, not enough params");
    return 1;
  }

  x_pid = g_atoi(argv[1]);
  wm_pid = g_atoi(argv[2]);
  username = argv[3];

  g_writeln("xrdp-sessvc: waiting for X (pid %d) and WM (pid %d)",
             x_pid, wm_pid);
  lerror = 0;

  g_signal_kill(term_signal_handler); /* SIGKILL */
  g_signal_terminate(term_signal_handler); /* SIGTERM */
  g_signal_user_interrupt(term_signal_handler); /* SIGINT */
  g_signal_pipe(nil_signal_handler); /* SIGPIPE */

  /* wait for window manager to get done */
  ret = g_waitpid(wm_pid);
  while ((ret == 0) && !g_term)
  {
    ret = g_waitpid(wm_pid);
  }
  if (ret < 0)
  {
    lerror = g_get_errno();
  }
  g_writeln("xrdp-sessvc: WM is dead (waitpid said %d, errno is %d) "
            "exiting...", ret, lerror);
  /* kill X server */
  g_writeln("xrdp-sessvc: stopping X server");
  g_sigterm(x_pid);
  ret = g_waitpid(x_pid);
  while ((ret == 0) && !g_term)
  {
    ret = g_waitpid(x_pid);
  }
  g_writeln("xrdp-sessvc: clean exit");
  if (send_disconnect(username) != 0)
  {
  	g_writeln("xrdp-sessvc: Unable to send disconnect action");
  }
  return 0;
}
