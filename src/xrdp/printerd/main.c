/**
 * Copyright (C) 2008 Ulteo SAS
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

#include "printerd.h"

static int g_pid;
static int stop = 0;
static int thread_count = 0;
struct log_config *l_config;
static THREAD_POOL* pool;
static char pid_file[256] = {0};


/******************************************************************************/
void DEFAULT_CC
sig_printerd_shutdown(int sig)
{
	log_message(l_config, LOG_LEVEL_INFO, "shutting down xrdp-printerd");

	if (g_getpid() != g_pid)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "g_getpid() [%d] differs from g_pid [%d]", (g_getpid()), g_pid);
		return;
	}
	stop = 1;
	printer_purge_all();
	g_file_delete(pid_file);
}

/******************************************************************************/
void DEFAULT_CC
sig_printerd_pipe(int sig)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "Catch sig pipe");

}

/******************************************************************************/
void* DEFAULT_CC
thread_routine()
{
	int job = 0;

	while(stop == 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "sesman[thread_routine]: "
  			"Wait job");

		thread_pool_wait_job(pool);

		job = thread_pool_pop_job(pool);
		if (job == 0)
		{
			continue;
		}

		xml_printerd_process_request(job);
	}
}

/******************************************************************************/
void DEFAULT_CC
printerd_main_loop()
{
	int server = g_create_unix_socket(PRINTER_SOCKET_NAME);
	g_chmod_hex(PRINTER_SOCKET_NAME, 0xFFFF);

	log_message(l_config, LOG_LEVEL_INFO, "xrdp_printer[printer_main_loop]: "
					"Create pool thread [%i]", thread_count);
	pool = thread_pool_init_pool(thread_count);

	log_message(l_config, LOG_LEVEL_INFO, "xrdp_printer[printer_main_loop]: "
					"Start pool thread");
	thread_pool_start_pool_thread(pool, thread_routine);

	xmlInitParser();
	printer_purge_all();
	while (stop == 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "xrdp_printer[printer_main_loop]: "
			"wait connection");
		int client = g_wait_connection(server);
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "xrdp_printer[printer_main_loop]: "
			"New client connection [%i]",client);

		if (client < 0)
		{
			log_message(l_config, LOG_LEVEL_WARNING, "xrdp_printer[printer_main_loop]: "
				"Unable to get client from management socket [%s]", strerror(g_get_errno()));
			continue;
		}

		thread_pool_push_job(pool, client);
	}
}


int DEFAULT_CC
printerd_init()
{
	char filename[256];
	struct list *names;
	struct list *values;
	char *name;
	char *value;
	int index;
	int res;

	l_config = g_malloc(sizeof(struct log_config), 1);
	l_config->program_name = g_strdup("printerd");
	l_config->log_file = 0;
	l_config->fd = 0;
	l_config->log_level = LOG_LEVEL_DEBUG;
	l_config->enable_syslog = 0;
	l_config->syslog_level = LOG_LEVEL_DEBUG;

	names = list_create();
	names->auto_free = 1;
	values = list_create();
	values->auto_free = 1;
	g_snprintf(filename, 255, "%s/printerd.ini", XRDP_CFG_PATH);

	if (file_by_name_read_section (filename, PRINTERD_CFG_LOGGING, names, values) == 0) {
		for (index = 0; index < names->count; index++) {
			name = (char *)list_get_item(names, index);
			value = (char *)list_get_item(values, index);
			if (0 == g_strcasecmp(name, PRINTERD_CFG_LOG_DIR)) {
				l_config->log_file = (char *)g_strdup(value);
			}
			if (0 == g_strcasecmp(name, PRINTERD_CFG_LOG_LEVEL)) {
				l_config->log_level = log_text2level(value);
			}
			if (0 ==
			    g_strcasecmp(name, PRINTERD_CFG_LOG_ENABLE_SYSLOG)) {
				l_config->enable_syslog = log_text2bool(value);
			}
			if (0 == g_strcasecmp(name, PRINTERD_CFG_LOG_SYSLOG_LEVEL)) {
				l_config->syslog_level = log_text2level(value);
			}
		}
	}

	if (file_by_name_read_section
	    (filename, PRINTERD_CFG_GLOBAL, names, values) == 0) {
		for (index = 0; index < names->count; index++) {
			name = (char *)list_get_item(names, index);
			value = (char *)list_get_item(values, index);
			if (0 == g_strcasecmp(name, PRINTERD_CFG_GLOBAL_THREAD_COUNT)) {
				thread_count = g_atoi(value);
			}
		}
	}
	list_delete(names);
	list_delete(values);
	res = log_start(l_config);

	if (res != LOG_STARTUP_OK)
	{
		g_printf("xrdp-printerd[printerd_init]: Unable to start log system[%i]\n", res);
		return res;
	}
	else
	{
		return LOG_STARTUP_OK;
	}
}



/******************************************************************************/
int DEFAULT_CC
main(int argc, char** argv)
{
	int fd = 0;
	int error = 0;
	int daemon = 1;
	int pid = 0;
	char pid_s[8] = {0};

	if(g_is_root() != 0)
	{
		g_printf("Error, xrdp-printerd service must be start with root privilege\n");
		return 0;
	}


	g_snprintf(pid_file, 255, "%s/xrdp-printerd.pid", XRDP_PID_PATH);
	if (1 == argc)
	{
		/* no options on command line. normal startup */
		g_printf("starting xrdp-printerd...");
		daemon = 1;
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--nodaemon")) ||
                           (0 == g_strcasecmp(argv[1], "-n")) ||
                           (0 == g_strcasecmp(argv[1], "-ns"))))
	{
		/* starts xrdp-printerd not daemonized */
		g_printf("starting xrdp-printerd in foregroud...");
		daemon = 0;
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--help")) ||
                           (0 == g_strcasecmp(argv[1], "-h"))))
	{
		/* help screen */
		g_printf("xrdp-printerd - xrdp printer manager\n\n");
		g_printf("usage: xrdp-printerd [command]\n\n");
		g_printf("command can be one of the following:\n");
		g_printf("-n, -ns, --nodaemon  starts xrdp-printerd in foreground\n");
		g_printf("-k, --kill           kills running xrdp-printerd\n");
		g_printf("-h, --help           shows this help\n");
		g_printf("if no command is specified, xrdp-printerd is started in background");
		g_exit(0);
	}
	else if ((2 == argc) && ((0 == g_strcasecmp(argv[1], "--kill")) || (0 == g_strcasecmp(argv[1], "-k"))))
	{
		/* killing running xrdp-printerd */
		/* check if xrdp-printerd is running */
		if (!g_file_exist(pid_file))
		{
			g_printf("xrdp-printerd is not running (pid file not found - %s)\n", pid_file);
			g_exit(1);
		}

		fd = g_file_open(pid_file);

		if (-1 == fd)
		{
			g_printf("error opening pid file[%s]: %s\n", pid_file, g_get_strerror());
			return 1;
		}

		error = g_file_read(fd, pid_s, 7);
		if (-1 == error)
		{
			g_printf("error reading pid file: %s\n", g_get_strerror());
			g_file_close(fd);
			g_exit(error);
		}
		g_file_close(fd);
		pid = g_atoi(pid_s);

		error = g_sigterm(pid);
		if (0 != error)
		{
			g_printf("error killing xrdp-printerd: %s\n", g_get_strerror());
		}
		else
		{
			g_file_delete(pid_file);
		}

		g_exit(error);
	}
	else
	{
		/* there's something strange on the command line */
		g_printf("xrdp-printerd - xrdp printer manager\n\n");
		g_printf("error: invalid command line\n");
		g_printf("usage: xrdp-printerd [ --nodaemon | --kill | --help ]\n");
		g_exit(1);
	}

	if (g_file_exist(pid_file))
	{
		g_printf("xrdp-printerd is already running.\n");
		g_printf("if it's not running, try removing ");
		g_printf(pid_file);
		g_printf("\n");
		g_exit(1);
	}

	if (printerd_init() != LOG_STARTUP_OK)
	{
		switch (error)
		{
		case LOG_ERROR_MALLOC:
			g_printf("error on malloc. cannot start logging. quitting.\n");
			break;
		case LOG_ERROR_FILE_OPEN:
			g_printf("error opening log file [%s]. quitting.\n", l_config->log_file);
			break;
		}
		g_exit(1);
	}

	if (daemon)
	{
		/* start of daemonizing code */
		if (g_daemonize(pid_file) == 0)
		{
			g_writeln("problem daemonize");
			g_exit(1);
		}
	}

	/* signal handling */
	g_pid = g_getpid();

	g_signal_user_interrupt(sig_printerd_shutdown);		/* SIGINT  */
	g_signal_kill(sig_printerd_shutdown);				/* SIGKILL */
	g_signal_terminate(sig_printerd_shutdown);			/* SIGTERM */
	g_signal_pipe(sig_printerd_pipe);			/* SIGPIPE */

	/* start program main loop */
	log_message(l_config, LOG_LEVEL_INFO,
              "starting xrdp-printerd with pid %d", g_pid);


	if (!g_directory_exist(XRDP_SOCKET_PATH))
	{
		g_create_dir(XRDP_SOCKET_PATH);
		g_chmod_hex(XRDP_SOCKET_PATH, 0x1777);
	}

	printerd_main_loop();


	log_end(l_config);

	return 0;
}

