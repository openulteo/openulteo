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


#include "rdpdr.h"
#include "fuse_dev.h"
#include "rdpfs.h"

struct log_config	*l_config;
int disk_up = 0;
char username[256];
char mount_point[256];
pthread_t vchannel_thread;


/*****************************************************************************/
int
disk_init()
{
	char filename[256];
	char log_filename[256];
	struct list* names;
	struct list* values;
	char* name;
	char* value;
	int index;
	int display_num;
	int res;

	display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	if(display_num == 0)
	{
		g_printf("vchannel_rdpdr[disk_init]: Display must be different of 0\n");
		return ERROR;
	}
	l_config = g_malloc(sizeof(struct log_config), 1);
	l_config->program_name = "vchannel_rdpdr";
	l_config->log_file = 0;
	l_config->fd = 0;
	l_config->log_level = LOG_LEVEL_DEBUG;
	l_config->enable_syslog = 0;
	l_config->syslog_level = LOG_LEVEL_DEBUG;

	names = list_create();
	names->auto_free = 1;
	values = list_create();
	values->auto_free = 1;
	g_snprintf(filename, 255, "%s/rdpdr.conf", XRDP_CFG_PATH);
	if (file_by_name_read_section(filename, RDPDR_CFG_GLOBAL, names, values) == 0)
	{
		for (index = 0; index < names->count; index++)
		{
			name = (char*)list_get_item(names, index);
			value = (char*)list_get_item(values, index);
			if (0 == g_strcasecmp(name, RDPDR_CFG_NAME))
			{
				if( g_strlen(value) > 1)
				{
					l_config->program_name = (char*)g_strdup(value);
				}
			}
		}
	}
	if (file_by_name_read_section(filename, RDPDR_CFG_LOGGING, names, values) == 0)
	{
		for (index = 0; index < names->count; index++)
		{
			name = (char*)list_get_item(names, index);
			value = (char*)list_get_item(values, index);
			if (0 == g_strcasecmp(name, RDPDR_CFG_LOG_DIR))
			{
				l_config->log_file = (char*)g_strdup(value);
			}
			if (0 == g_strcasecmp(name, RDPDR_CFG_LOG_LEVEL))
			{
				l_config->log_level = log_text2level(value);
			}
			if (0 == g_strcasecmp(name, RDPDR_CFG_LOG_ENABLE_SYSLOG))
			{
				l_config->enable_syslog = log_text2bool(value);
			}
			if (0 == g_strcasecmp(name, RDPDR_CFG_LOG_SYSLOG_LEVEL))
			{
				l_config->syslog_level = log_text2level(value);
			}
		}
	}
	if( g_strlen(l_config->log_file) > 1 && g_strlen(l_config->program_name) > 1)
	{
		g_sprintf(log_filename, "%s/%i/%s.log", l_config->log_file, display_num, l_config->program_name);
		g_free(l_config->log_file);
		l_config->log_file = (char*)g_strdup(log_filename);
	}
	list_delete(names);
	list_delete(values);
	res = log_start(l_config);
	if( res != LOG_STARTUP_OK)
	{
		g_printf("vchannel_rdpdr[rdpdr_init]: Unable to start log system [%i]\n", res);
		return res;
	}
	return LOG_STARTUP_OK;
}

/*****************************************************************************/
void disk_deinit()
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[disk_deinit]: "
						"unmounting drive");
	rdpfs_close();
	fuse_unmount(mount_point);
	g_exit(0);
}

/*****************************************************************************/
int main(int argc, char** argv, char** environ)
{
	int fuse_group = 0;
	int ok = 0;
	char* home_dir = g_getenv("HOME");

	l_config = g_malloc(sizeof(struct log_config), 1);
	if (argc != 2)
	{
		g_printf("Usage : vchannel_rdpdr USERNAME\n");
		g_free(l_config);
		return 1;
	}

	if (disk_init() != LOG_STARTUP_OK)
	{
		g_printf("vchannel_rdpdr[main]: Unable to init log system\n");
		g_free(l_config);
		return 1;
	}

	if ( g_getuser_info(argv[1], 0, 0, 0, 0, 0) == 1)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[main]: "
				"The username '%s' did not exist\n", argv[1]);
	}
	g_strncpy(username, argv[1], sizeof(username));
	g_getgroup_info("fuse", &fuse_group);
	if (g_check_user_in_group(username, fuse_group, &ok) == 1)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[main]: "
				"Error while testing if user %s is member of fuse group", username);
		g_free(l_config);
		return 1;
	}
	if (ok == 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[main]: "
				"User %s is not allow to use fuse", username);
		g_free(l_config);
		return 1;
	}

	if (vchannel_init() == ERROR)
	{
		g_printf("vchannel_rdpdr[main]: Unable to init channel system\n");
		g_free(l_config);
		return 1;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[main]: "
				"Open channel to rdpdr main apps");
	if (rdpfs_open() == 1)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[main]: "
					"Unable to open a connection to RDP filesystem");
	}

//	share_desktop_purge();
	share_bookmark_purge();
	share_symlink_purge();

	g_sprintf(mount_point, "%s/%s", home_dir, RDPDRIVE_NAME);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[main]: "
				"Rdpdrive is located on %s", mount_point);

	if (fuse_run() == 1)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[main]: "
				"Fail to start fuse");
	}
	g_free(l_config);

	return 0;
}
