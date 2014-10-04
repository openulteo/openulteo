/**
 * Copyright (C) 2010-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010-2013
 * Author James B. MacLean <macleajb@ednet.ns.ca> 2012
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
#include "rdpfs.h"
#include <xrdp_constants.h>
#include <limits.h>
#include <fuse.h>




extern struct log_config *l_config;
extern char username[256];

int rdpdr_sock;
static char client_name[256];
static int is_fragmented_packet = 0;
static int fragment_size;
static struct stream* splitted_packet;
static Action actions[128];
struct request_response rdpfs_response[128];
static int action_index=0;
static struct disk_device disk_devices[MAX_SHARE];
static int disk_devices_count = 0;
extern int disk_up;
static tbus send_mutex;
static tbus action_index_mutex;
static int client_id;
static int vers_major;
static int vers_minor;
static int supported_operation[6] = {0};

extern void disk_deinit();

/*****************************************************************************/
/* get the next action index */
static int
get_next_action_index()
{
	int new_action_index = 0;

	tc_mutex_lock(action_index_mutex);
	if (action_index == 127)
	{
		action_index = 0;
	}
	else
	{
		action_index++;
	}
	new_action_index = action_index;
	tc_mutex_unlock(action_index_mutex);

	return new_action_index;
}

/*****************************************************************************/
/* Convert seconds since 1970 back to filetime */
static time_t
convert_1970_to_filetime(uint64_t ticks)
{
	time_t val;

	ticks /= 10000000;
	ticks -= 11644473600LL;

	val = (time_t) ticks;
	return (val);

}

/*****************************************************************************/
static uint64_t
convert_filetime_to_1970(uint64_t ticks)
{
	ticks += 11644473600LL;
	ticks *= 10000000;
	return (ticks);

}

/*****************************************************************************/
static int
get_attributes_from_mode(int mode_t)
{
	return 0;

}

void
convert_to_win_path(const char* posix_path, char* win_path)
{
	if (posix_path == NULL)
	{
		win_path[0] = '\0';
		return;
	}

	if (win_path == NULL)
		return;

	g_strncpy(win_path, posix_path, PATH_MAX);
	g_str_replace_all(win_path, "/", "\\");
}

/*****************************************************************************/
int APP_CC
rdpfs_send(struct stream* s){
	int rv;
	int length;

	tc_mutex_lock(send_mutex);
	length = (int)(s->end - s->data);

	rv = vchannel_send(rdpdr_sock, s->data, length);
	if (rv != 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpfs_send]: "
				"Unable to send message");
	}
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[rdpfs_send]: "
				"send message: ");
	log_hexdump(l_config, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)s->data, length );
	tc_mutex_unlock(send_mutex);

	return rv;
}

/*****************************************************************************/
int APP_CC
rdpfs_receive(const char* data, int* length, int* total_length)
{
	return vchannel_receive(rdpdr_sock, data, length, total_length);

}

/*****************************************************************************/
void APP_CC
rdpfs_wait_reply(int completion_id)
{
	barrier_t* barrier = &rdpfs_response[completion_id].barrier;
	tc_barrier_wait(barrier);
}

/*****************************************************************************/
int APP_CC
rdpfs_get_device_count()
{
	return disk_devices_count;
}

/*****************************************************************************/
struct disk_device* APP_CC
rdpfs_get_device_by_index(int device_index)
{
	return &disk_devices[device_index];
}


/************************************************************************/
struct disk_device* APP_CC
rdpfs_get_dir(int device_id)
{
	int i;
	for (i=0 ; i< disk_devices_count ; i++)
	{
		if(device_id == disk_devices[i].device_id)
		{
			return &disk_devices[i];
		}
	}
	return 0;
}

/************************************************************************/
struct disk_device* APP_CC
rdpfs_get_device_from_path(const char* path)
{
	//extract device name
	char *pos;
	int count;
	int i;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[disk_dev_get_device_from_path]: "
				"The path is: %s", path);

	if (rdpfs_is_printer_device(path))
	{
		path += g_strlen(PRINTER_DIR) + 2;
	}
	else
	{
		path++;
	}

	pos = strchr(path, '/');
	if(pos == NULL)
	{
		count = g_strlen(path);
	}
	else
	{
		count = pos-path;
	}

	for (i=0 ; i< disk_devices_count ; i++)
	{
		if(g_strncmp(path, disk_devices[i].dir_name, count) == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[disk_dev_get_device_from_path]: "
						"The drive is: %s", disk_devices[i].dir_name);
			return &disk_devices[i];
		}
	}
	return 0;
}

/************************************************************************/
int APP_CC
rdpfs_is_printer_device(const char* path)
{
	return (g_strncmp(path+1, PRINTER_DIR, g_strlen(PRINTER_DIR)) == 0);
}

/************************************************************************/
int APP_CC
rdpfs_convert_fs_to_stat(struct fs_info* fs, struct stat* st)
{
	st->st_mode = S_IFREG | 0744;
	if( fs->file_attributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		st->st_mode = S_IFDIR | 0744;
	}

	if( fs->file_attributes & FILE_ATTRIBUTE_READONLY )
	{
		st->st_mode |= 0444;
	}

	st->st_size = fs->file_size;
	st->st_atim.tv_sec = fs->last_access_time;
	st->st_ctim.tv_sec = fs->create_access_time;
	st->st_mtim.tv_sec = fs->last_change_time;
	st->st_blocks = fs->allocation_size;
	st->st_uid = g_getuid();
	st->st_gid = g_getuid();

	st->st_nlink = 2;
	return 0;
}

/************************************************************************/
int APP_CC
rdpfs_get_owner_permission(mode_t mode)
{
	int owner_perm = 0;

	if( mode & 0100)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_get_owner_permission]: "
					"Owner can execute");
		owner_perm |= GENERIC_EXECUTE;
	}
	if( mode & 0200)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_get_owner_permission]: "
					"Owner can write");
		owner_perm |= GENERIC_WRITE;
	}
	if( mode & 0400)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_get_owner_permission]: "
					"Owner can read ");
		owner_perm |= GENERIC_READ;
	}
	return owner_perm;
}

/************************************************************************/
int APP_CC
rdpfs_get_other_permission(mode_t mode)
{
	int other_perm = 0;

	if( mode & 0002)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_get_other_permission]: "
					"Other can write");
		other_perm |= FILE_SHARE_WRITE;
	}
	if( mode & 0004)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_get_other_permission]: "
					"Other can read");
		other_perm |= FILE_SHARE_READ;
	}
	return other_perm;
}


/*****************************************************************************/
int APP_CC
rdpfs_open()
{
	int i = 0;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_open]: "
				"Open channel to channel server");
	rdpdr_sock = vchannel_open("rdpdr");
	if(rdpdr_sock == ERROR)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpfs_open]: "
				"Error while connecting to rdpdr provider");
		return 1;
	}
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_open]: "
			"Initialise the rdpfs cache");
	rdpfs_cache_init();
	send_mutex = tc_mutex_create();
	action_index_mutex = tc_mutex_create();

	for (i = 0; i< 128 ; i++)
	{
		tc_barrier_init(&rdpfs_response[i].barrier,2);
	}

	printer_init();

	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_close()
{
	share_desktop_purge();
	share_symlink_purge();
	share_bookmark_purge();
	while (disk_devices_count != 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_close]: "
			"Remove device with id= %i", disk_devices[0].device_id);
		rdpfs_remove(disk_devices[0].device_id);
	}

	vchannel_close(rdpdr_sock);
	rdpfs_cache_dinit();
	printer_dinit();
	//TODO mutex cond realease
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_create(int device_id, int desired_access, int shared_access,
		int creation_disposition, int flags, const char* path)
{
	struct stream* s;
	int completion_id;
	char win_path[PATH_MAX];

	make_stream(s);
	init_stream(s, 2 * PATH_MAX +100);

	completion_id = get_next_action_index();

	actions[completion_id].device = device_id;
	actions[completion_id].file_id = completion_id;
	actions[completion_id].last_req = IRP_MJ_CREATE;


	g_strcpy(actions[completion_id].path, path);
	convert_to_win_path(path, win_path);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_create]:"
  		"Process job[%s]",path);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, device_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, completion_id);                      /* completion id */
	out_uint32_le(s, IRP_MJ_CREATE);                      /* major version */
	out_uint32_le(s, 0);                                  /* minor version */

	out_uint32_le(s, desired_access);                     /* FsInformationClass */
	out_uint64_le(s, 8000);                               /* allocationSizee */
	out_uint32_le(s, 0x80);                               /* FileAttributes */
	out_uint32_le(s, shared_access);                      /* SharedMode */
	out_uint32_le(s, creation_disposition);               /* Disposition */
	out_uint32_le(s, flags);                              /* CreateOptions */
	out_uint32_le(s, (strlen(win_path) + 1) * 2);                     /* PathLength */
	uni_rdp_out_str(s, (char*)win_path, (strlen(win_path) + 1) * 2);  /* Path */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	return completion_id;
}

/*****************************************************************************/
int APP_CC
rdpfs_request_read(int completion_id, int device_id, int length, int offset)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_request_read]: CID: %d, Fileid:%d length_requested:%d, offset:%d pdevid:%d actdevid:%d", completion_id, actions[completion_id].file_id, length, offset, device_id, actions[completion_id].device);
	struct stream* s;
	make_stream(s);
	init_stream(s,1024);

	actions[completion_id].last_req = IRP_MJ_READ;

	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);                        /* completion id */
	out_uint32_le(s, IRP_MJ_READ);                          /* major version */
	out_uint32_le(s, 0);                                    /* minor version */

	out_uint32_le(s, length);                               /* length */
	out_uint64_le(s, offset);                               /* offset */
	out_uint8s(s, 20);                                      /* padding */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
  return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_request_write(int completion_id, int offset, int length)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_request_write]: Begin");
	struct stream* s;
	make_stream(s);
	init_stream(s, length + 56);

	actions[completion_id].last_req = IRP_MJ_WRITE;

	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);                        /* completion id */
	out_uint32_le(s, IRP_MJ_WRITE);                         /* major version */
	out_uint32_le(s, 0);                                    /* minor version */

	out_uint32_le(s, length);                               /* length */
	out_uint64_le(s, offset);                               /* offset */
	out_uint8s(s, 20);                                      /* padding */
	out_uint8p(s, rdpfs_response[completion_id].buffer, length);

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_request_write]: End");

  return 0;
}

/*****************************************************************************/
void APP_CC
rdpfs_request_close(int completion_id, int device_id)
{
	struct stream* s;
	make_stream(s);
	init_stream(s,1024);

	actions[completion_id].last_req = IRP_MJ_CLOSE;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_request_close]:"
			"Close device id %i", actions[completion_id].device);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, IRP_MJ_CLOSE);           /* major version */
	out_uint32_le(s, 0);                      /* minor version */
	out_uint8s(s, 32);                        /* padding */
	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	//remove action
  return ;
}

/*****************************************************************************/
void APP_CC
rdpfs_query_volume_information(int completion_id, int device_id, int information, const char* query )
{
	struct stream* s;
	int win_path_length;
	char win_path[PATH_MAX];

	make_stream(s);
	init_stream(s, 2* PATH_MAX + 100);

	g_strcpy(actions[completion_id].path, query);
	convert_to_win_path(query, win_path);

	actions[completion_id].last_req = IRP_MJ_QUERY_VOLUME_INFORMATION;
	actions[completion_id].request_param = information;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_volume_information]:"
  		"Process job[%s]",query);
	win_path_length = (g_strlen(win_path)+1)*2;
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, IRP_MJ_QUERY_VOLUME_INFORMATION);      /* major version */
	out_uint32_le(s, 0);                                    /* minor version */

	out_uint32_le(s, information);                          /* FsInformationClass */
	out_uint32_le(s, win_path_length);                      /* length */
	out_uint8s(s, 24);                                      /* padding */
	out_uint8p(s, win_path, win_path_length);               /* query */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
  return ;
}

/*****************************************************************************/
void APP_CC
rdpfs_query_information(int completion_id, int device_id, int information, const char* query )
{
	struct stream* s;
	char win_path[PATH_MAX];

	make_stream(s);
	init_stream(s, 2* PATH_MAX + 100);

	g_strcpy(actions[completion_id].path, query);
	convert_to_win_path(query, win_path);

	actions[completion_id].last_req = IRP_MJ_QUERY_INFORMATION;
	actions[completion_id].request_param = information;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_information]:"
  		"Process job[%s]",query);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, IRP_MJ_QUERY_INFORMATION);             /* major version */
	out_uint32_le(s,0);                                     /* minor version */

	out_uint32_le(s, information);                          /* FsInformationClass */
	out_uint32_le(s, g_strlen(win_path));                   /* length */
	out_uint8s(s, 24);                                      /* padding */
	out_uint8p(s, win_path, g_strlen(win_path));            /* query */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
}

/*****************************************************************************/
void APP_CC
rdpfs_query_setinformation(int completion_id, int information, struct fs_info* fs )
{
	struct stream* s;
	uint64_t time;
	int attributes;
	make_stream(s);
	init_stream(s,1024);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: ");

	actions[completion_id].last_req = IRP_MJ_SET_INFORMATION;
	actions[completion_id].request_param = information;

	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, IRP_MJ_SET_INFORMATION);           /* major version */
	out_uint32_le(s, 0);                                /* minor version */

	out_uint32_le(s, information);
	switch(information)
	{
	case FileBasicInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: "
				"set FileBasicInformation");
		out_uint8s(s, 28);                    /* padding */
		time = convert_filetime_to_1970(fs->create_access_time);
		out_uint64_le(s, time);
		time = convert_filetime_to_1970(fs->last_access_time);
		out_uint64_le(s, time);
		time = convert_filetime_to_1970(fs->last_write_time);
		out_uint64_le(s, time);
		time = convert_filetime_to_1970(fs->last_change_time);
		out_uint64_le(s, time);
		attributes = get_attributes_from_mode(fs->file_attributes);
		out_uint32_le(s, attributes);
		out_uint8s(s, 4);
		break;

	case FileDispositionInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: "
				"set FileDispositionInformation");
		out_uint32_le(s, 0);                               /* length */
		out_uint8s(s, 24);                                  /* padding */
		break;

	case FileEndOfFileInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: "
				"set FileEndOfFileInformation to %i", fs->file_size);
		out_uint32_le(s, 4);                               /* length */
		out_uint8s(s, 24);	/* padding */
		out_uint32_le(s, fs->file_size);	/* file size */
		break;

	case FileRenameInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: "
				"set FileRenameInformation");
		out_uint8s(s, 28);                    /* padding */
		out_uint8(s, 0);                      /* replaceIf exist */
		out_uint8(s, 0);                     /* rootDirectory must be set to 0 */
		out_uint32_le(s, (strlen(fs->filename)+1)*2); /* PathLength */
		uni_rdp_out_str(s, (char*)fs->filename, (strlen(fs->filename)+1)*2);			/* Path */
		break;

	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_setinformation]: "
						"IRP_MJ_SET_INFORMATION request : unknow FsInformationClass");
	}

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
}

/*****************************************************************************/
void APP_CC
rdpfs_query_directory(int completion_id, int device_id, int information, const char* query )
{
	struct stream* s;
	int win_path_length = 0;
	int considerPath = 0;
	char win_path[PATH_MAX];
	win_path[0] = '\0';

	make_stream(s);
	init_stream(s, 2* PATH_MAX + 100);

	if (g_strcmp(query, "") != 0)
	{
		considerPath = 1;
		g_strcpy(actions[completion_id].path, query);
		convert_to_win_path(query, win_path);
	}

	win_path_length = (g_strlen(win_path)+1)*2;
	actions[completion_id].last_req = IRP_MN_QUERY_DIRECTORY;
	actions[completion_id].request_param = information;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_query_directory]:"
  		"Process query[%s]",query);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_IOREQUEST);
	out_uint32_le(s, actions[completion_id].device);
	out_uint32_le(s, actions[completion_id].file_id);
	out_uint32_le(s, completion_id);
	out_uint32_le(s, IRP_MJ_DIRECTORY_CONTROL);           /* major version */
	out_uint32_le(s, IRP_MN_QUERY_DIRECTORY);             /* minor version */

	out_uint32_le(s, information);                        /* FsInformationClass */
	out_uint8(s, considerPath);                           /* path is considered ? */
	out_uint32_le(s, win_path_length);                    /* length */
	out_uint8s(s, 23);                                    /* padding */
	uni_rdp_out_str(s, (char*)win_path, win_path_length); /* query */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
}


/*****************************************************************************/
int APP_CC
rdpfs_list_reply(int device_id, int status)
{
	struct stream* s;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_reply]:"
		" reply to the device add");
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_DEVICE_REPLY);
	out_uint32_le(s, device_id);          /* device_id */
	out_uint32_le(s, status);             /* status */

	s_mark_end(s);

	g_sleep(10);
	rdpfs_send(s);
	free_stream(s);
	return 0;
}

/************************************************************************/
int APP_CC
rdpfs_add(struct stream* s, int device_data_length, int device_id, char* device_name, int device_type)
{
	if (device_id < 0)
	{
		return -1;
	}
	if (disk_devices_count >= MAX_SHARE)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpfs_add]: "
				"Failed to add device %s, max number of share reached",
				disk_devices[disk_devices_count].dir_name);
		return -1;
	}
	disk_devices[disk_devices_count].device_id = device_id;
	disk_devices[disk_devices_count].device_type = device_type;
	g_strcpy(disk_devices[disk_devices_count].dir_name, device_name);

	if ( device_type == RDPDR_DTYP_PRINT)
	{
		char* converted_name = printer_convert_name(device_name);
		if (converted_name == NULL)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_add]: "
					"Unable to add printer [%s], the name is invalid", device_name);
			return 0;
		}
		g_strcpy(disk_devices[disk_devices_count].dir_name, converted_name);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_add]: "
				"Succeed to add printer %s", disk_devices[disk_devices_count].dir_name);
		printer_add(username, disk_devices[disk_devices_count].dir_name);

		g_free(converted_name);
		return disk_devices_count++;
	}

	if ( device_type == RDPDR_DTYP_FILESYSTEM)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_add]: "
				"Succedd to disk share %s", disk_devices[disk_devices_count].dir_name);

		share_add_to_symlink(device_name, client_name);
		share_add_to_bookmarks(device_name, client_name);
		return disk_devices_count++;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_add]: "
			"Invalid device %s", disk_devices[disk_devices_count].dir_name);
	return 0;
}

/************************************************************************/
void APP_CC
rdpfs_remove(int device_id)
{
	struct disk_device* device;
	struct disk_device* last_device;

	device = rdpfs_get_dir(device_id);
	last_device = &disk_devices[disk_devices_count-1];
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_remove]: "
				"Removing disk %s with id=%i", device->dir_name, device->device_id);

	if (device->device_type == RDPDR_DTYP_FILESYSTEM)
	{
	//	share_remove_from_desktop(device->dir_name);
		share_remove_from_symlink(device->dir_name, client_name);
		share_remove_from_bookmarks(device->dir_name, client_name);
	}
	if (device->device_type == RDPDR_DTYP_PRINT)
	{
		printer_del(username, device->dir_name);
	}

	if (device->device_id == last_device->device_id)
	{
		device->device_id = -1;
		device->dir_name[0] = 0;
	}
	else
	{
		device->dir_name[0] = 0;
		device->device_id = last_device->device_id;
		device->device_type = last_device->device_type;
		g_strcpy(device->dir_name, last_device->dir_name);
	}
	disk_devices_count--;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_remove]: "
				"Succeed to remove device");
}

/*****************************************************************************/
int APP_CC
rdpfs_list_remove(struct stream* s)
{
	int device_list_count, device_id;
	int i;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_remove]: "
		"new message: PAKID_CORE_DEVICELIST_REMOVE");
	in_uint32_le(s, device_list_count);	/* DeviceCount */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_remove]: "
		"%i device(s) to remove", device_list_count);
	/* device list */
	for( i=0 ; i<device_list_count ; i++)
	{
		in_uint32_le(s, device_id);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_remove]: "
			"device id to remove: %i", device_id);

		rdpfs_remove(device_id);
	}
	return 0;
}


/*****************************************************************************/
int APP_CC
rdpfs_list_announce(struct stream* s)
{
	int device_list_count, device_id, device_type, device_data_length;
	int pnp_name_len, driver_name_len, printer_name_len,cached_field_name;
	int i;
	char device_name[1024] ;
	int handle;
	char* p;
	int str_length;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
		"	new message: PAKID_CORE_DEVICELIST_ANNOUNCE");
	in_uint32_le(s, device_list_count);	/* DeviceCount */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
		"%i device(s) declared", device_list_count);

	/* device list */
	for( i=0 ; i<device_list_count ; i++)
	{
		g_memset(device_name, 0, 1024);
		in_uint32_le(s, device_type);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
			"device type: %i", device_type);
		in_uint32_le(s, device_id);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
			"device id: %i", device_id);

		in_uint8a(s, device_name, 8)
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
			"dos name: '%s'", device_name);
		in_uint32_le(s, device_data_length);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
			"data length: %i", device_data_length);

		p = s->p;
		switch (device_type)
		{
		case RDPDR_DTYP_FILESYSTEM:
			if(device_data_length > 1)
			{
				// RDP specification explains that drive name must be sended in Unicode. (http://msdn.microsoft.com/en-us/library/cc241357)
				// That is wrong, the protocol use CP1252
				str_length = cp1252_rdp_in_str(s, device_name, sizeof(device_name), device_data_length);
			}
			break;

		case RDPDR_DTYP_PRINT:
			in_uint8s(s, 8);
			in_uint32_le(s, pnp_name_len);
			in_uint32_le(s, driver_name_len);
			in_uint32_le(s, printer_name_len);
			in_uint32_le(s, cached_field_name);
			in_uint8s(s, pnp_name_len + driver_name_len);
			if(printer_name_len != 0)
			{
				uni_rdp_in_str(s, device_name, sizeof(device_name), printer_name_len);
				log_message(l_config, LOG_LEVEL_DEBUG, "rdpdr_printer[printer_dev_add]: "
					"Printer name = %s", device_name);
			}
			break;

		default:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
				"The device type %04x is not valid", device_type);
			s->p = p + device_data_length;
			continue;
		}

		handle = rdpfs_add(s, device_data_length, device_id, device_name, device_type);
		s->p = p + device_data_length;
		if (handle < 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_list_announce]: "
				"Unable to add disk device");
			rdpfs_list_reply(device_id, STATUS_INVALID_PARAMETER);
			continue;
		}
		rdpfs_list_reply(device_id, STATUS_SUCCESS);
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_confirm_clientID_request()
{
	struct stream* s;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_capability_request]: Send Server Client ID Confirm Request");
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_CLIENTID_CONFIRM);
	out_uint16_le(s, 0x1);                          /* major version */
	out_uint16_le(s, RDP5);                         /* minor version */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	return 0;
}


/*****************************************************************************/
int APP_CC
rdpfs_process_create_response(int completion_id, struct stream* s)
{
	in_uint32_le(s, actions[completion_id].file_id);			/* client file id */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"IRP_MJ_CREATE response: update file id with %i", actions[completion_id].file_id);

	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_read_response(int completion_id, struct stream* s)
{
	int length;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_read_response]");
	in_uint32_le(s, length);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_read_response] : %i bytes returned ",length );

	g_memcpy(rdpfs_response[completion_id].buffer, s->p, length);
	rdpfs_response[completion_id].buffer_length = length;
	return 0;

}

/*****************************************************************************/
int APP_CC
rdpfs_process_write_response(int completion_id, struct stream* s)
{
	int length;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_write_response]");
	in_uint32_le(s, length);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_write_response] : %i return ",length );

	g_memcpy(rdpfs_response[completion_id].buffer, s->p, length);
	rdpfs_response[completion_id].buffer_length = length;
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_directory_response(int completion_id, struct stream* s)
{
	if(rdpfs_response[completion_id].request_status != 0)
	{
		return 0;
	}
	int is_last_entry;
	int length;
	int filename_length;

	struct fs_info* file_response;
	uint64_t time;
	char path[256];
	struct disk_device* disk;

	file_response = &rdpfs_response[completion_id].fs_inf;
	in_uint32_le(s, length);			/* response length */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_directory_response]: "
			"IRP_MN_QUERY_DIRECTORY response : response length : %i", length);

	if (actions[completion_id].request_param != FileBothDirectoryInformation)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_directory_response]: "
				"IRP_MN_QUERY_DIRECTORY response : bad flag for request parameter : %04x", actions[completion_id].request_param);
		return 1;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"IRP_MJ_QUERY_DIRECTORY response : extract FileBothDirectoryInformation information");

	in_uint32_le(s, is_last_entry );
	in_uint8s(s, 4);

	in_uint64_le(s, time);	/* creation time */
	file_response->create_access_time = convert_1970_to_filetime(time);

	in_uint64_le(s, time);	/* last_access_time */
	file_response->last_access_time = convert_1970_to_filetime(time);

	in_uint64_le(s, time);	/* last_write_time */
	file_response->last_write_time = convert_1970_to_filetime(time);

	in_uint64_le(s, time);	/* last_change_time */
	file_response->last_change_time = convert_1970_to_filetime(time);

	in_uint64_le(s, file_response->file_size);	/* filesize */
	in_uint64_le(s, file_response->allocation_size);   /* allocated filesize  */
	in_uint32_le(s, file_response->file_attributes);   /* attributes  */

	in_uint32_le(s, filename_length);        /* unicode filename length */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_volume_information_response]: "
			"filename length : %i", filename_length);

	in_uint8s(s, 29);               /* we ignore the short file name */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_volume_information_response]: "
			"message : ");
	log_hexdump(l_config, LOG_LEVEL_DEBUG, (unsigned char*)s->p, filename_length);

	uni_rdp_in_str(s, file_response->filename, sizeof(file_response->filename), filename_length);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_volume_information_response]: "
						"IRP_MJ_QUERY_DIRECTORY response : get filename : %s", file_response->filename);

	disk = rdpfs_get_dir(actions[completion_id].device);
	g_sprintf(path, "/%s%s",  disk->dir_name, actions[completion_id].path);
	path[g_strlen(path)-1] = 0;
	g_sprintf(path, "%s%s",  path, file_response->filename);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_volume_information_response]: "
							"IRP_MJ_QUERY_DIRECTORY response : path added in cache : %s", path);

	rdpfs_cache_add_fs(g_strdup(path), file_response);


	return 0;
}



/*****************************************************************************/
int APP_CC
rdpfs_process_volume_information_response(int completion_id, struct stream* s)
{
	int length;
	struct request_response* rep;
	int label_length;
	int object_fs;


	rep = &rdpfs_response[completion_id];

	in_uint32_le(s, length);			/* response length */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"IRP_MJ_QUERY_VOLUME_INFORMATION response : response length : %i", length);
	rep->Request_type = RDPDR_IRP_MJ_QUERY_VOLUME_INFORMATION;

	switch (actions[completion_id].request_param)
	{
	case FileFsVolumeInformation:

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"IRP_MJ_QUERY_VOLUME_INFORMATION response : extract FileFsVolumeInformation information");
		rep->Request_param = actions[completion_id].request_param;

		in_uint64_le(s, rep->fs_inf.create_access_time);    /* volume creation time */
		in_uint8s(s, 4);                                    /* serial (ignored) */
		in_uint32_le(s, label_length);	                        /* length of string */
		in_uint8(s, object_fs);	                                /* support objects? */
		if (object_fs != 0)
		{
			log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
					"IRP_MJ_QUERY_VOLUME_INFORMATION response : Xrdp did not support object file system");
		}
		uni_rdp_in_str(s, rep->fs_inf.filename, sizeof(rep->fs_inf.filename), label_length);
		break;

	case FileFsSizeInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
						"IRP_MJ_QUERY_VOLUME_INFORMATION response : extract FileFsSizeInformation information");

//		in_uint32_le(s, low);                          /* Total allocation units low */
//		in_uint32_le(s, high);                         /* Total allocation high units */
//		in_uint32_le(s, rep->volume_inf.f_bfree);	     /* Available allocation units */
//		in_uint32_le(s, rep->volume_inf.f_blocks);     /* Available allowcation units */
//		in_uint32_le(s, ignored);                      /* Sectors per allocation unit */
//		in_uint32_le(s, ignored);                      /* Bytes per sector */
		break;

	case FileFsAttributeInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
						"IRP_MJ_QUERY_VOLUME_INFORMATION response : extract FileFsAttributeInformation information");

//		in_uint32_le(s, ignored);                      /* fs attributes */
//		in_uint32_le(s, rep->volume_inf.f_namelen);	   /* max length of filename */
//
//		in_uint32_le(s, fs_name_len);                  /* length of fs_type */
//		rdp_in_unistr(s, rep->volume_inf.fs_type, sizeof(rep->volume_inf.fs_type), fs_name_len);
		break;

	case FileFsDeviceInformation:
	case FileFsFullSizeInformation:

	default:

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_volume_information_response]: "
						"IRP_MJ_QUERY_VOLUME_INFORMATION response : unknow response");
		return 1;
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_setinformation_response(int completion_id, struct stream* s)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_setinformation_response]: "
							"IRP_MJ_QUERY_SET_VOLUME_INFORMATION response : %04x", rdpfs_response[completion_id].request_status);
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_information_response(int completion_id, struct stream* s)
{
	int length;
	struct request_response* rep;
	int ignored;
	uint64_t time;

	rep = &rdpfs_response[completion_id];

	in_uint32_le(s, length);			/* response length */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_information_response]: "
			"IRP_MJ_QUERY_INFORMATION response : response length : %i", length);
	rep->Request_type = RDPDR_IRP_MJ_QUERY_INFORMATION;


	switch (actions[completion_id].request_param)
	{
	case FileBasicInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_information_response]: "
				"IRP_MJ_QUERY_INFORMATION response : param response FileBasicInformation");

		in_uint64_le(s, time);	/* create_access_time */
		rep->fs_inf.create_access_time = convert_1970_to_filetime(time);

		in_uint64_le(s, time);	/* last_access_time */
		rep->fs_inf.last_access_time = convert_1970_to_filetime(time);

		in_uint64_le(s, time);	/* last_write_time */
		rep->fs_inf.last_write_time = convert_1970_to_filetime(time);

		in_uint64_le(s, time);	/* last_change_time */
		rep->fs_inf.last_change_time = convert_1970_to_filetime(time);

		in_uint32_le(s, rep->fs_inf.file_attributes);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_information_response]: "
				"IRP_MJ_QUERY_INFORMATION response : param response FileBasicInformation : %04x", rep->fs_inf.file_attributes);
		break;

	case FileStandardInformation:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_information_response]: "
				"IRP_MJ_QUERY_INFORMATION response : param response FileStandardInformation");
		in_uint64_le(s, rep->fs_inf.allocation_size);       /* Allocation size */
		in_uint64_le(s, rep->fs_inf.file_size);             /* End of file */
		in_uint32_le(s, rep->fs_inf.nlink);                 /* Number of links */
		in_uint32_le(s, rep->fs_inf.delele_request);        /* Delete pending */
		in_uint8(s, rep->fs_inf.is_dir);                     /* Directory */
		in_uint32_le(s, ignored);
		break;

	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_information_response]: "
						"IRP_MJ_QUERY_INFORMATION response : unknow response");
		return 0;
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_iocompletion(struct stream* s)
{
	int device;
	int completion_id;
	int io_status;
	int result;
	int size;


	result = 0;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"device reply");
	in_uint32_le(s, device);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"device : %i",device);
	in_uint32_le(s, completion_id);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"completion id : %i", completion_id);
	in_uint32_le(s, io_status);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
			"io_statio : %08x", io_status);

	rdpfs_response[completion_id].request_status = io_status;
	if( io_status != STATUS_SUCCESS)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
  			"Action  %04x failed with the status : %08x", actions[completion_id].last_req, io_status);
  	result = -1;
	}

	switch(actions[completion_id].last_req)
	{
	case IRP_MJ_CREATE:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_CREATE response");
		result = rdpfs_process_create_response(completion_id, s);
		break;

	case IRP_MJ_READ:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_READ response");
		result = rdpfs_process_read_response(completion_id, s);
		break;

	case IRP_MJ_QUERY_VOLUME_INFORMATION:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_QUERY_VOLUME_INFORMATION response");
		result = rdpfs_process_volume_information_response(completion_id, s);
		break;

	case IRP_MJ_QUERY_INFORMATION:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_QUERY_INFORMATION response");
		result = rdpfs_process_information_response(completion_id, s);
		break;

	case IRP_MJ_SET_INFORMATION:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_SET_INFORMATION response");
		result = rdpfs_process_setinformation_response(completion_id, s);
		break;

	case IRP_MN_QUERY_DIRECTORY:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MN_QUERY_DIRECTORY response");
		result = rdpfs_process_directory_response(completion_id, s);
		break;

	case IRP_MJ_WRITE:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_WRITE response");
		in_uint32_le(s, size);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"%i octect written for the jobs %s",size, actions[completion_id].path);
		rdpfs_response[completion_id].buffer_length = size;
		result = 0;
		break;

	case IRP_MJ_CLOSE:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"process IRP_MJ_CLOSE response");
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"file '%s' closed",actions[completion_id].path);

		result = 0;
		break;
	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_process_iocompletion]: "
				"last request %08x is invalid",actions[completion_id].last_req);
		result = -1;
	}

	barrier_t* barrier = &rdpfs_response[completion_id].barrier;
	tc_barrier_wait(barrier);

	return result;
}


/*****************************************************************************/
int APP_CC
rdpfs_send_init(void)
{
	struct stream* s;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_init]: "
		"Send init message");
	/*server announce*/
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_SERVER_ANNOUNCE);
	out_uint16_le(s, 0x1);
	out_uint16_le(s, 0x0c);
	out_uint32_le(s, 0x1);
	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_send_client_id_confirm(void)
{
	struct stream* s;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_init]: "
		"Send client id confirmation");
	/*server announce*/
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_CLIENTID_CONFIRM);
	out_uint16_le(s, 0x1);
	out_uint16_le(s, 0x0c);
	out_uint32_le(s, client_id);
	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	return 0;
}


/*****************************************************************************/
void APP_CC
rdpfs_send_server_capability()
{
	struct stream* s;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_server_capability]: "
		"Send server capabilities");
	/*server announce*/
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_SERVER_CAPABILITY);
	out_uint16_le(s, 5);                               /* Number of capabilities */
	out_uint16_le(s, 0x0);                             /* Padding */
	/* general capability */
	out_uint16_le(s, CAP_GENERAL_TYPE);                /* type */
	out_uint16_le(s, 0x002c);                          /* length */
	out_uint32_le(s, GENERAL_CAPABILITY_VERSION_02);   /* version */
	out_uint32_le(s, OS_TYPE_WINNT);
	out_uint32_le(s, 0x00);
	out_uint16_le(s, 0x0001);                          /* major */
	out_uint16_le(s, 0x000c);                          /* minor */
	out_uint32_le(s, 0x0000ffff);
	out_uint32_le(s, 0x00000000);
	out_uint32_le(s, 0x00000007);
	out_uint32_le(s, 0x00000000);
	out_uint32_le(s, 0x00000000);
	out_uint32_le(s, 0x00000002);

	/* printer capabilities */
	out_uint16_le(s, CAP_PRINTER_TYPE);
	out_uint16_le(s, 0x0008);
	out_uint32_le(s, PRINT_CAPABILITY_VERSION_01);

	/* port capabilities */
	out_uint16_le(s, CAP_PORT_TYPE);
	out_uint16_le(s, 0x0008);
	out_uint32_le(s, PORT_CAPABILITY_VERSION_01);

	/* drive capabilities */
	out_uint16_le(s, CAP_DRIVE_TYPE);
	out_uint16_le(s, 0x0008);
	out_uint32_le(s, DRIVE_CAPABILITY_VERSION_02);

	/* smartcard capabilities */
	out_uint16_le(s, CAP_SMARTCARD_TYPE);
	out_uint16_le(s, 0x0008);
	out_uint32_le(s, SMARTCARD_CAPABILITY_VERSION_01);


	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
}

/*****************************************************************************/
void APP_CC
rdpfs_send_server_logon()
{
	struct stream* s;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_server_capability]: "
		"Send init message");

	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_USER_LOGGEDON);

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
}

/*****************************************************************************/
int APP_CC
rdpdr_confirm_clientID_request()
{
	struct stream* s;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_send_capability_request]: "
		"Send Server Client ID Confirm Request");
	make_stream(s);
	init_stream(s, 256);
	out_uint16_le(s, RDPDR_CTYP_CORE);
	out_uint16_le(s, PAKID_CORE_CLIENTID_CONFIRM);
	out_uint16_le(s, 0x1);              /* major version */
	out_uint16_le(s, RDP5);             /* minor version */

	s_mark_end(s);
	rdpfs_send(s);
	free_stream(s);
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpdr_clientID_confirm(struct stream* s)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_clientID_confirm]: "
		"new message: PAKID_CORE_CLIENTID_CONFIRM");
	in_uint16_le(s, vers_major);
	in_uint32_le(s, vers_minor);
	in_uint32_le(s, client_id);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_clientID_confirm]: "
		"version : %i:%i, client_id : %i", vers_major, vers_minor, client_id);
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_client_name(struct stream* s)
{
	int hostname_size;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_client_name]: ");

	in_uint8s(s, 4);
	in_uint32_le(s, hostname_size);   /* flag not use */
	in_uint32_le(s, hostname_size);
	if (hostname_size < 1)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpfs_client_name]: ");
		return 1;
	}

	uni_rdp_in_str(s, client_name, sizeof(client_name), hostname_size);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_client_name]: "
		"hostname : '%s'",client_name);

	return 0;
}


/*****************************************************************************/
int APP_CC
rdpdr_client_capability(struct stream* s)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
		"new message: PAKID_CORE_CLIENT_CAPABILITY");
	int capability_count, ignored, temp, general_capability_version, rdp_version, i;

	in_uint16_le(s, capability_count);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
		"Capability number : %i", capability_count);
	if (capability_count == 0 )
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
			"No capability ");
		return 1;
	}
	in_uint16_le(s, ignored);
	/* GENERAL_CAPS_SET */
	in_uint16_le(s, temp);		/* capability type */
	if (temp != CAP_GENERAL_TYPE )
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
			"Malformed message (normaly  CAP_GENERAL_TYPE)");
		return 1;
	}
	in_uint16_le(s, ignored);		/* capability length */
	in_uint32_le(s, general_capability_version);		/* general capability version */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
		"general capability version = %i ",general_capability_version);
	if (general_capability_version != GENERAL_CAPABILITY_VERSION_01 && general_capability_version != GENERAL_CAPABILITY_VERSION_02)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
			"Malformed message (normaly general_capability_version = [GENERAL_CAPABILITY_VERSION_01|GENERAL_CAPABILITY_VERSION_02])");
		return 1;
	}
	/* Capability message */
	in_uint32_le(s, ignored);               /* OS type */
	in_uint32_le(s, ignored);               /* OS version */
	in_uint16_le(s, ignored);               /* major version */
	in_uint16_le(s, rdp_version);           /* minor version */
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
		"RDP version = %i ",rdp_version);
	if(rdp_version == RDP6)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
			"only RDP5 is supported");
	}
	in_uint32_le(s, temp);	/* oiCode1 */
	if (temp != RDPDR_IRP_MJ_ALL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
			"client did not support all the IRP operation");
		return 1;
	}
	in_uint32_le(s, ignored);                /* oiCode2(unused) */
	in_uint32_le(s, ignored);                /* extendedPDU(unused) */
	in_uint32_le(s, ignored);                /* extraFlags1 */
	in_uint32_le(s, ignored);                /* extraFlags2 */
	if (general_capability_version == GENERAL_CAPABILITY_VERSION_02)
	{
		in_uint32_le(s, ignored);        /* SpecialTypeDeviceCap (device redirected before logon (smartcard/com port */
	}

	for( i=1 ; i<capability_count ; i++ )
	{
		in_uint16_le(s, temp);           /* capability type */
		switch (temp)
		{
		case CAP_DRIVE_TYPE:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
				"CAP_DRIVE_TYPE supported by the client");
			supported_operation[temp]=1;
			break;
		case CAP_PORT_TYPE:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
				"CAP_PORT_TYPE supported by the client but not by the server");
			supported_operation[temp]=1;
			break;
		case CAP_PRINTER_TYPE:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
				"CAP_PRINTER_TYPE supported by the client");
			supported_operation[temp]=1;
			break;
		case CAP_SMARTCARD_TYPE:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_client_capability]: "
				"CAP_SMARTCARD_TYPE supported by the client but not by the server");
			supported_operation[temp]=1;
			break;
		default:
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpdr_client_capability]: "
				"invalid capability type : %i", temp);
			break;
		}
		in_uint16_le(s, ignored);       /* capability length */
		in_uint32_le(s, ignored);       /* capability version */
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
rdpfs_process_message(struct stream* s, int length, int total_length)
{
	int component;
	int packetId;
	int result = 0;
	struct stream* packet;

	if(length != total_length)
	{
		log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[rdpfs_process_message]: "
			"Packet is fragmented");
		if(is_fragmented_packet == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr[rdpfs_process_message]: "
				"Packet is fragmented : first part");
			is_fragmented_packet = 1;
			fragment_size = length;
			make_stream(splitted_packet);
			init_stream(splitted_packet, total_length);
			g_memcpy(splitted_packet->p,s->p, length );
			log_hexdump(l_config, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)s->p, length);
			return 0;
		}
		else
		{
			g_memcpy(splitted_packet->p+fragment_size, s->p, length );
			fragment_size += length;
			if (fragment_size == total_length )
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpdr_process_message]: "
					"Packet is fragmented : last part");
				packet = splitted_packet;
			}
			else
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpdr_process_message]: "
					"Packet is fragmented : next part");
				return 0;
			}
		}
	}
	else
	{
		packet = s;
	}
	log_message(l_config, LOG_LEVEL_DEBUG_PLUS, "vchannel_rdpdr_channel[rdpdr_process_message]: "
		"Data received:");
	log_hexdump(l_config, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)packet->p, total_length);
	in_uint16_le(packet, component);
	in_uint16_le(packet, packetId);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr_channel[rdpdr_process_message]: "
		"component=0x%04x packetId=0x%04x", component, packetId);
	if ( component == RDPDR_CTYP_CORE )
	{
		switch (packetId)
		{
		case PAKID_CORE_CLIENTID_CONFIRM :
			result = rdpdr_clientID_confirm(packet);
			break;

		case PAKID_CORE_CLIENT_NAME :
			result =  rdpfs_client_name(packet);
			rdpfs_send_server_capability();
			rdpfs_send_client_id_confirm();
			break;

		case PAKID_CORE_CLIENT_CAPABILITY:
			result = rdpdr_client_capability(packet);
			rdpfs_send_server_logon();
			break;

		case PAKID_CORE_DEVICELIST_ANNOUNCE:
		case PAKID_CORE_DEVICELIST_REMOVE:
			rdpfs_list_announce(packet);
			break;

		case PAKID_CORE_DEVICE_IOCOMPLETION:
			rdpfs_process_iocompletion(packet);
			break;

		default:
			log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr_channel[rdpdr_process_message]: "
				"Unknown message %02x",packetId);
			result = 1;
		}

		if(is_fragmented_packet == 1)
		{
			is_fragmented_packet = 0;
			fragment_size = 0;
			free_stream(packet);
		}
	}
	return result;
}


/*****************************************************************************/
void *thread_vchannel_process (void * arg)
{
	struct stream* s = NULL;
	int rv;
	int length;
	int total_length;

	rdpfs_send_init();
	while(1){
		make_stream(s);
		init_stream(s, 1605);
		rv = rdpfs_receive(s->data, &length, &total_length);

		switch(rv)
		{
		case ERROR:
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[thread_vchannel_process]: "
					"Invalid message");
			free_stream(s);
			pthread_exit ((void*) 1);
			break;
		case STATUS_CONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[thread_vchannel_process]: "
					"Status connected");
			break;
		case STATUS_DISCONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[thread_vchannel_process]: "
					"Status disconnected");
			disk_deinit();
			break;
		default:
			if (length == 0)
			{
				disk_deinit();
				pthread_exit (0);
			}
			rdpfs_process_message(s, length, total_length);
			break;
		}
		free_stream(s);
	}
	pthread_exit (0);
}
