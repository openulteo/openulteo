/**
 * Copyright (C) 2010 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010
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

#include <stdlib.h>
#include <stdio.h>
#include <os_calls.h>
#include <log.h>
#include <fcntl.h>
#include <sys/stat.h>

static int log_fd = 0;
static int log_socket = 0;
static int running = 0;

/*****************************************************************************/
void DEFAULT_CC
logd_shutdown(int sig)
{
	g_writeln("shutdown xrdp-logd daemon");
	g_tcp_close(log_socket);
	g_file_close(log_fd);
	running = 0;
}

/*****************************************************************************/
void APP_CC
logd_main_loop()
{
	int client = 0;
	char buffer[1024];
	int count;

	log_fd = open(LOGGING_FILE, O_WRONLY | O_CREAT | O_APPEND | O_SYNC, S_IRUSR | S_IWUSR);
	g_mkdir("/var/spool/xrdp");
	g_chmod_hex("/var/spool/xrdp", 0775);
	if ( g_directory_exist("/var/spool/xrdp") == 0)
	{
		g_writeln("Unable to create the logging socket");
		g_exit(1);
	}
	g_writeln("xrdp logging spool application\n");
	log_socket = g_create_unix_socket(LOGGING_SOCKET);
	g_chmod_hex(LOGGING_SOCKET, 0xFFFF);
	if ( log_socket == 1)
	{
		g_writeln("Unable to create logging socket");
		return ;
	}
	if (log_fd < 0){
		g_writeln("Unable to create logging instance\n");
	}
	running = 1;
	while(running)
	{
		client = g_wait_connection(log_socket);

		count = g_tcp_recv(client, (char*)buffer, 1024, 0);
		while(count > 0)
		{
			g_file_write(log_fd, buffer, count);
			count = g_tcp_recv(client, (char*)buffer, 1024, 0);
		}
		count = 0;
		g_tcp_close(client);
	}

}


int APP_CC
main(int argc, char** argv)
{
	int pid;
	int fd;
	int no_daemon;
	char text[256];
	char pid_file[256];

	if(g_is_root() != 0){
	g_printf("Error, xrdp-logd service must be start with root privilege\n");
	return 0;
	}
	g_init();

	g_snprintf(pid_file, 255, "%s/xrdp-logd.pid", XRDP_PID_PATH);
	no_daemon = 0;
	if (argc == 2)
	{
	if ((g_strncasecmp(argv[1], "-kill", 255) == 0) ||
		(g_strncasecmp(argv[1], "--kill", 255) == 0) ||
		(g_strncasecmp(argv[1], "-k", 255) == 0))
	{
		g_writeln("stopping xrdp-logd");
		/* read the xrdp.pid file */
		fd = -1;
		if (g_file_exist(pid_file)) /* xrdp.pid */
		{
			fd = g_file_open(pid_file); /* xrdp.pid */
		}
		if (fd == -1)
		{
			g_writeln("problem opening to xrdp-logd.pid");
			g_writeln("maybe its not running");
		}
		else
		{
			g_memset(text, 0, 32);
			g_file_read(fd, text, 31);
			pid = g_atoi(text);
			g_writeln("stopping process id %d", pid);
			if (pid > 0)
			{
				g_sigterm(pid);
			}
			g_file_close(fd);
		}
		g_exit(0);
	}
	else if (g_strncasecmp(argv[1], "-nodaemon", 255) == 0 ||
			 g_strncasecmp(argv[1], "--nodaemon", 255) == 0 ||
			 g_strncasecmp(argv[1], "-nd", 255) == 0 ||
			 g_strncasecmp(argv[1], "--nd", 255) == 0 ||
			 g_strncasecmp(argv[1], "-ns", 255) == 0 ||
			 g_strncasecmp(argv[1], "--ns", 255) == 0)
	{
		no_daemon = 1;
	}
	else if (g_strncasecmp(argv[1], "-help", 255) == 0 ||
			 g_strncasecmp(argv[1], "--help", 255) == 0 ||
			 g_strncasecmp(argv[1], "-h", 255) == 0)
	{
		g_writeln("");
		g_writeln("xrdp: A Remote Desktop Protocol server.");
		g_writeln("Copyright (C) Jay Sorg 2004-2009");
		g_writeln("See http://xrdp.sourceforge.net for more information.");
		g_writeln("");
		g_writeln("Usage: xrdp-logd [options]");
		g_writeln("   -h: show help");
		g_writeln("   --nodaemon: don't fork into background");
		g_writeln("   -kill: shut down xrdp-logd");
		g_writeln("");
		g_exit(0);
		}
		else if ((g_strncasecmp(argv[1], "-v", 255) == 0) ||
			 (g_strncasecmp(argv[1], "--version", 255) == 0))
		{
			g_writeln("");
			g_writeln("xrdp: A Remote Desktop Protocol server.");
			g_writeln("Copyright (C) Jay Sorg 2004-2009");
			g_writeln("See http://xrdp.sourceforge.net for more information.");
			g_writeln("Version 0.5.0");
			g_writeln("");
			g_exit(0);
		}
		else
		{
			g_writeln("Unknown Parameter");
			g_writeln("xrdp-logd -h for help");
			g_writeln("");
			g_exit(0);
		}
	}
	else if (argc > 1)
	{
		g_writeln("Unknown Parameter");
		g_writeln("xrdp-logd -h for help");
		g_writeln("");
		g_exit(0);
	}
	if (g_file_exist(pid_file)) /* xrdp-logd.pid */
	{
		g_writeln("It looks like xrdp-logd is allready running,");
		g_writeln("if not delete the xrdp-logd.pid file and try again");
		g_exit(0);
	}
	if (!no_daemon)
	{
	/* start of daemonizing code */

		if (g_daemonize(pid_file) == 0)
		{
			g_writeln("problem daemonize");
			g_exit(1);
		}
	}
  g_signal_user_interrupt(logd_shutdown); /* SIGINT */
  g_signal_kill(logd_shutdown); /* SIGKILL */
  g_signal_terminate(logd_shutdown); /* SIGTERM */

	pid = g_getpid();
	logd_main_loop();
	/* delete the xrdp-logd.pid file */
	g_file_delete(pid_file);

	return 0;
}
