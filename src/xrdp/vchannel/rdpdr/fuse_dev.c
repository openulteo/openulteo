/**
 * Copyright (C) 2010-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010-2011, 2013
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

#define FUSE_USE_VERSION 26



#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "fuse_dev.h"
#include "rdpfs.h"
#include "rdpfs_cache.h"


extern pthread_t vchannel_thread;
extern void *thread_vchannel_process (void * arg);
extern struct log_config *l_config;
extern char mount_point[256];
extern int rdpdr_sock;
extern int disk_up;
extern struct request_response rdpfs_response[128];

int pouet = 0;


/************************************************************************
	Begin of fuse operation
 */

/************************************************************************/
static struct fs_info* fuse_dev_file_getattr(struct disk_device* disk, const char* path, struct stat *stbuf, int *error)
{
	int completion_id = 0;
	int status;

	completion_id = rdpfs_create(disk->device_id, GENERIC_READ|FILE_EXECUTE_ATTRIBUTES,	FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,	FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, path);
	rdpfs_wait_reply(completion_id);
	status = rdpfs_response[completion_id].request_status;
	switch(status)
	{
	case STATUS_SUCCESS:
		rdpfs_query_information(completion_id, disk->device_id, FileBasicInformation, path);
		rdpfs_wait_reply(completion_id);
		rdpfs_query_information(completion_id, disk->device_id, FileStandardInformation,path);
		rdpfs_wait_reply(completion_id);
		rdpfs_request_close(completion_id, disk->device_id);
		rdpfs_wait_reply(completion_id);
		return &rdpfs_response[completion_id].fs_inf;

	case STATUS_NO_SUCH_FILE:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_file_getattr]: "
					"file did not exist");
		*error = -ENOENT;
		return NULL;
	default:
		*error = -EIO;
		return NULL;
	}
}

/************************************************************************/
static int fuse_dev_getattr(const char *path, struct stat *stbuf)
{
	struct disk_device *disk = NULL;
	char* rdp_path = NULL;
	struct fs_info* fs = NULL;
	int error = 0;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
				"getattr on %s", path);

	g_memset(stbuf, 0, sizeof(struct stat));
	if (disk_up == 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
				"get_attr return");
		return -1;
	}

	if (rdpfs_is_printer_device(path))
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
				"Device is a printer");

		if (g_str_end_with((char*)path, ".pdf") == 0)
		{
			stbuf->st_mode = S_IFREG | 0744;
			return 0;
		}
		stbuf->st_nlink = 2;
		stbuf->st_mode = S_IFDIR | 0755;
		return 0;
	}

	rdp_path = g_strdup(path);

	if (strcmp(rdp_path, "/") == 0) {
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
				"%i device mounted ", rdpfs_get_device_count());
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = rdpfs_get_device_count() + 2;
		return 0;
	}

	fs = rdpfs_cache_get_fs(rdp_path);
	if (fs == NULL)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
				"%s not in cache", rdp_path);
		disk = rdpfs_get_device_from_path(rdp_path);
		if ( disk == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
					"Device from path(%s) did not exist", rdp_path);
			return 0;
		}
		g_str_replace_first(rdp_path, disk->dir_name, "");


		if (strcmp(rdp_path, "/") == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_getattr]: "
					"request (fuse_dev_volume_getattr)");
			fs = fuse_dev_file_getattr(disk, "", stbuf, &error);
		}
		else
		{
			fs = fuse_dev_file_getattr(disk, rdp_path+1, stbuf, &error);
		}
		if (fs == NULL)
		{
			return error;
		}
		rdpfs_cache_add_fs(g_strdup(path), fs);
	}

	return rdpfs_convert_fs_to_stat(fs, stbuf);
}

/************************************************************************/
static int fuse_dev_access(const char *path, int mask)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_access]: "
				"access on %s", path);

	return F_OK;
}

/************************************************************************/
static int fuse_dev_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	int i;
	(void) fi;
	(void) offset;
	struct disk_device *disk;
	char rdp_path[256];
	int completion_id;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
				"readdir on %s", path);

	if (strcmp(path, "/") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		for (i=0 ; i<rdpfs_get_device_count() ; i++)
		{
			disk = rdpfs_get_device_by_index(i);
			if (disk->device_type == RDPDR_DTYP_FILESYSTEM)
			{
				filler(buf, disk->dir_name , NULL, 0);
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
						"the device name is : %s", disk->dir_name);
			}
		}
		return 0;
	}

	disk = rdpfs_get_device_from_path(path);
	if(disk == 0)
	{
		return 0;
	}

	g_sprintf(rdp_path, "%s/", path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
				"Request rdp path %s", rdp_path );

	completion_id = rdpfs_create(disk->device_id,
                               GENERIC_ALL|DELETE,
                               FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT|FILE_DIRECTORY_FILE,
                               rdp_path);
	rdpfs_wait_reply(completion_id);
	g_sprintf(rdp_path, "%s*", rdp_path);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
				"Request rdp path %s", rdp_path );


	rdpfs_query_directory(completion_id, disk->device_id, FileBothDirectoryInformation, rdp_path);
	rdpfs_wait_reply(completion_id);
	if (rdpfs_response[completion_id].request_status == 0)
	{
		if (filler(buf, rdpfs_response[completion_id].fs_inf.filename, NULL, 0))
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
						"Failed to add the file %s", rdpfs_response[completion_id].fs_inf.filename);
		}
	}
	while( rdpfs_response[completion_id].request_status == 0 )
	{
		rdpfs_query_directory(completion_id, disk->device_id, FileBothDirectoryInformation, "");
		rdpfs_wait_reply(completion_id);
		if (rdpfs_response[completion_id].request_status == 0)
		{
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
						"Add %s in filler", rdpfs_response[completion_id].fs_inf.filename);
			if (filler(buf, rdpfs_response[completion_id].fs_inf.filename, NULL, 0))
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
							"Failed to add the file %s", rdpfs_response[completion_id].fs_inf.filename);
			}
		}
	}

	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_readdir]: "
				"End of %s", rdp_path);

	return 0;
}

/************************************************************************/
static int fuse_dev_mknod(const char *path, mode_t mode, dev_t rdev)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mknod] ");
	int completion_id = 0;
	struct disk_device* disk;
	int owner_perm = 0;
	int other_perm = 0;
	int attributes = 0;
	char* rdp_path;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mknod]: rdev : %i", rdev);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mknod]: rdev : %i", mode);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mknod]: rdev : %s", path);

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_mknod]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}


	owner_perm = rdpfs_get_owner_permission(mode);
	other_perm = rdpfs_get_other_permission(mode);

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	attributes = FILE_SYNCHRONOUS_IO_NONALERT |FILE_NON_DIRECTORY_FILE;
	completion_id = rdpfs_create(disk->device_id, owner_perm,	other_perm,	FILE_CREATE, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		rdpfs_request_close(completion_id, disk->device_id);
		rdpfs_wait_reply(completion_id);
		return -errno;
	}
	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return 0;
}

/************************************************************************/
static int fuse_dev_mkdir(const char *path, mode_t mode)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mkdir] ");
	int completion_id = 0;
	struct disk_device* disk;
	int owner_perm = 0;
	int other_perm = 0;
	int attributes = 0;
	char* rdp_path;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mkdir]: rdev : %i", mode);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_mkdir]: rdev : %s", path);

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_mknod]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	owner_perm = rdpfs_get_owner_permission(mode);
	other_perm = rdpfs_get_other_permission(mode);

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	attributes = FILE_SYNCHRONOUS_IO_NONALERT |FILE_DIRECTORY_FILE;
	completion_id = rdpfs_create(disk->device_id, owner_perm,	other_perm,	FILE_CREATE, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		rdpfs_request_close(completion_id, disk->device_id);
		rdpfs_wait_reply(completion_id);
		return -errno;
	}
	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return 0;
}

/************************************************************************/
static int fuse_dev_unlink(const char *path)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_unlink] ");
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info fs;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_unlink]: rdev : %s", path);

	if (rdpfs_is_printer_device(path))
	{
		return 0;
	}

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_unlink]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	fs.delele_request = 1;
	rdpfs_cache_remove_fs(path);

	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		return -errno;
	}
	rdpfs_query_setinformation(completion_id, FileDispositionInformation, &fs);
	rdpfs_wait_reply(completion_id);
	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return 0;
}

/************************************************************************/
static int fuse_dev_rmdir(const char *path)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_unlink] ");
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info fs;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_rmdir]: rdev : %s", path);

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_rmdir]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	fs.delele_request = 1;
	rdpfs_cache_remove_fs(path);

	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		return -errno;
	}
	rdpfs_query_setinformation(completion_id, FileDispositionInformation, &fs);
	rdpfs_wait_reply(completion_id);
	if( rdpfs_response[completion_id].request_status != 0 )
	{
		log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_unlink]: "
				"FileDispositionInformation fails 0x%x", rdpfs_response[completion_id].request_status);
		return -errno;
	}

	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);
	if( rdpfs_response[completion_id].request_status != 0 )
	{
		log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_unlink]: "
				"rdpfs_request_close fails 0x%x", rdpfs_response[completion_id].request_status);
		return -errno;
	}

	return 0;
}

/************************************************************************/
static int fuse_dev_rename(const char *from, const char *to)
{
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info fs;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_rename]: "
			"change from %s to %s", from, to);

	disk = rdpfs_get_device_from_path(to);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_rename]:"
					"Unable to get device from path : %s", from);
 		return -errno;
	}

	rdp_path = g_strdup(from);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	g_strcpy(fs.filename, rdp_path);

	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		rdpfs_request_close(completion_id, disk->device_id);
		rdpfs_wait_reply(completion_id);
 		return -errno;
	}

	rdp_path = g_strdup(to);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	g_strcpy(fs.filename, rdp_path);

	rdpfs_query_setinformation(completion_id, FileRenameInformation, &fs);
	rdpfs_wait_reply(completion_id);
	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);
 	return 0;
 }

/************************************************************************/
static int fuse_dev_chmod(const char *path, mode_t mode)
 {
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path = NULL;
	struct fs_info* fs = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_chmod]: "
			"Path : %s", path);

	if (rdpfs_is_printer_device(path))
	{
		return 0;
	}

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_chmod]:"
					"Unable to get device from path : %s", path);
 		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	//fs.file_size = size;

	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
 		return -errno;
	}
	fs = rdpfs_cache_get_fs(path);
	if (fs == NULL)
	{
		rdpfs_query_information(completion_id, disk->device_id, FileBasicInformation, rdp_path);
		rdpfs_wait_reply(completion_id);
		fs = &rdpfs_response[completion_id].fs_inf;
	}
	fs->file_attributes = mode;

	rdpfs_query_setinformation(completion_id, FileBasicInformation, fs);
	rdpfs_wait_reply(completion_id);
	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);
	return 0;
}

/************************************************************************/
static int fuse_dev_chown(const char *path, uid_t uid, gid_t gid)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_chown]: "
				"Chown operation is not supported");
	return 0;
}

/************************************************************************/
static int fuse_dev_truncate(const char *path, off_t size)
{
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info fs;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_truncate]: "
			"path : %s", path);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_truncate]: "
			"size : %i", size);

	if (rdpfs_is_printer_device(path))
	{
		return 0;
	}

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_truncate]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	fs.file_size = size;

	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		return -errno;
	}
	rdpfs_query_setinformation(completion_id, FileEndOfFileInformation, &fs);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status == 0 )
	{
		rdpfs_cache_update_size(path, size);
	}

	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return 0;
}

/************************************************************************/
static int fuse_dev_utimens(const char *path, const struct timespec ts[2])
{
	log_message(l_config, LOG_LEVEL_DEBUG, "fuse_dev_utimens");
	return 0;
}

/************************************************************************/
static int fuse_dev_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_read]: "
			"Path to read : %s", path);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_read]: "
			"Size to read : %i", size);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_read]: "
			"Read from : %i", offset);

	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info* fs;
	int current_offset = offset;
	int size_read = 1;
	int chunk_size_to_read;
	int size_to_read = size;

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_read]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");


	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_READ|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access, FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_read]:"
					"Bad response : %s : 0x%x", rdp_path, rdpfs_response[completion_id].request_status);
		rdpfs_request_close(completion_id, disk->device_id);
		rdpfs_wait_reply(completion_id);
		return -errno;
	}

	rdpfs_query_information(completion_id, disk->device_id, FileStandardInformation,rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_read]: "
			"Read fileinfo on %s fails:%d", rdp_path, rdpfs_response[completion_id].request_status);
	}

	fs = &rdpfs_response[completion_id].fs_inf;

	rdpfs_response[completion_id].buffer = (unsigned char*)buf;
	while (size_to_read != 0)
	{
		rdpfs_response[completion_id].request_status = 0;
		chunk_size_to_read = size_to_read > MAX_SIZE ? MAX_SIZE : size_to_read;

		rdpfs_request_read(completion_id, disk->device_id, chunk_size_to_read, current_offset);
		rdpfs_wait_reply(completion_id);

		if( rdpfs_response[completion_id].request_status != 0 )
		{
			log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_read]: "
					"Read request fails:0x%x, CID:%d, File:%s", rdpfs_response[completion_id].request_status, completion_id, rdp_path);
			if (rdpfs_response[completion_id].request_status == 0xc0000022)
			{
				break;
			}
		}

		size_read = rdpfs_response[completion_id].buffer_length;
		current_offset += size_read;
		rdpfs_response[completion_id].buffer += size_read;
		size_to_read -= size_read;
		if (size_read != chunk_size_to_read)
		{
			//file end
			break;
		}
	}

	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return size - size_to_read;
}


/************************************************************************/
static int fuse_dev_printer_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	int current_offset = offset;
	int size_write = 1;
	int chunk_size_to_write;
	int size_to_write = size;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_printer_write]: "
			" Path to write : %s", path);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_printer_write]: "
			" Size to write : %i", size);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_printer_write]: "
			" Write from : %i", offset);

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_write]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");

	if (offset == 0)
	{
		attributes = FILE_SYNCHRONOUS_IO_NONALERT;
		desired_access = GENERIC_WRITE|FILE_EXECUTE_ATTRIBUTES;
		shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
		completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
		disk->completion_id = completion_id;
		rdpfs_wait_reply(completion_id);

		if( rdpfs_response[completion_id].request_status != 0 )
		{
			return -errno;
		}
	}
	else
	{
		completion_id = disk->completion_id;
	}

	rdpfs_response[completion_id].buffer = (unsigned char*)buf;
	rdpfs_response[completion_id].buffer_length = size;


	while (size_to_write != 0)
	{
		chunk_size_to_write = size_to_write > MAX_SIZE ? MAX_SIZE : size_to_write;

		rdpfs_request_write(completion_id, current_offset, chunk_size_to_write);
		rdpfs_wait_reply(completion_id);

		size_write = rdpfs_response[completion_id].buffer_length;
		current_offset += size_write;
		rdpfs_response[completion_id].buffer += size_write;
		size_to_write -= size_write;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_printer_dev_write]:"
				"try to write %i -> really write : %i", size, rdpfs_response[completion_id].buffer_length);

	return rdpfs_response[completion_id].buffer_length;
}

/************************************************************************/
static int fuse_dev_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int completion_id = 0;
	struct disk_device* disk;
	int attributes = 0;
	int desired_access = 0;
	int shared_access = 0;
	char* rdp_path;
	struct fs_info* fs;
	int current_offset = offset;
	int size_write = 1;
	int chunk_size_to_write;
	int size_to_write = size;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_write]: "
			" Path to write : %s", path);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_write]: "
			" Size to write : %i", size);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_write]: "
			" Write from : %i", offset);

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_printer_dev_write]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	if (rdpfs_is_printer_device(path) && disk->open == 0)
	{
		return fuse_dev_printer_write(path, buf, size, offset, fi);
	}

	rdp_path = g_strdup(path);
	g_str_replace_first(rdp_path, disk->dir_name, "");
	g_str_replace_first(rdp_path, "//", "/");


	attributes = FILE_SYNCHRONOUS_IO_NONALERT;
	desired_access = GENERIC_WRITE|FILE_EXECUTE_ATTRIBUTES;
	shared_access = FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE;
	completion_id = rdpfs_create(disk->device_id, desired_access , shared_access,	FILE_OPEN, attributes, rdp_path);
	rdpfs_wait_reply(completion_id);

	if( rdpfs_response[completion_id].request_status != 0 )
	{
		return -errno;
	}

	rdpfs_query_information(completion_id, disk->device_id, FileStandardInformation,path);
	rdpfs_wait_reply(completion_id);

	fs = &rdpfs_response[completion_id].fs_inf;


	rdpfs_response[completion_id].buffer = (unsigned char*)buf;
	rdpfs_response[completion_id].buffer_length = size;


	while (size_to_write != 0)
	{
		chunk_size_to_write = size_to_write > MAX_SIZE ? MAX_SIZE : size_to_write;

		rdpfs_request_write(completion_id, current_offset, chunk_size_to_write);
		rdpfs_wait_reply(completion_id);
		if( rdpfs_response[completion_id].request_status != 0 )
		{
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_printer_dev_write]:"
					"write fails to: %s with rc 0x%x", path, rdpfs_response[completion_id].request_status);
			return -errno;
		}

		size_write = rdpfs_response[completion_id].buffer_length;
		current_offset += size_write;
		rdpfs_response[completion_id].buffer += size_write;
		size_to_write -= size_write;

		if( rdpfs_response[completion_id].request_status != 0 )
		{
			log_message(l_config, LOG_LEVEL_INFO, "vchannel_rdpdr[fuse_dev_write]: "
						"rdpfs_write fails 0x%x", rdpfs_response[completion_id].request_status);
			rdpfs_request_close(completion_id, disk->device_id);
			rdpfs_wait_reply(completion_id);
			return -errno;
		}

	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_write]:"
				"try to write %i -> really write : %i", size, rdpfs_response[completion_id].buffer_length);

	rdpfs_request_close(completion_id, disk->device_id);
	rdpfs_wait_reply(completion_id);

	return rdpfs_response[completion_id].buffer_length;
}

/************************************************************************/
static int fuse_dev_statfs(const char *path, struct statvfs *stbuf)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "fuse_dev_statfs");
	return 0;
}

/************************************************************************/
static int fuse_dev_release(const char *path, struct fuse_file_info *fi)
{
	struct disk_device* disk;

	log_message(l_config, LOG_LEVEL_DEBUG, "fuse_dev_release");

	disk = rdpfs_get_device_from_path(path);
	if (disk == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_write]:"
					"Unable to get device from path : %s", path);
		return -errno;
	}

	if (rdpfs_is_printer_device(path))
	{
		rdpfs_request_close(disk->completion_id, disk->device_id);
		rdpfs_wait_reply(disk->completion_id);
		disk->completion_id = 0;
	}
	(void) path;
	(void) fi;
	return 0;
}

/************************************************************************/
static void* fuse_dev_init()
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_init]: ");
	if (pthread_create (&vchannel_thread, NULL, thread_vchannel_process, (void*)0) < 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_dev_init]: "
				"Pthread_create error for thread : vchannel_thread");
		return NULL;
	}
	disk_up = 1;
	return NULL;
}

/************************************************************************/
static void fuse_dev_destroy(void *private_data)
{
	(void)private_data;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_dev_destroy]");
	pthread_cancel(vchannel_thread);
	pthread_join(vchannel_thread, NULL);
	g_exit(0);

}

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
/************************************************************************/
static int fuse_dev_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

/************************************************************************/
static int fuse_dev_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

/************************************************************************/
static int fuse_dev_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

/************************************************************************/
static int fuse_dev_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */


/*
 * End of fuse operation
 */


static struct fuse_operations fuse_dev_oper = {
	.getattr	= fuse_dev_getattr,
	.access		= fuse_dev_access,
	.readlink	= NULL,
	.readdir	= fuse_dev_readdir,
	.mknod		= fuse_dev_mknod,
	.mkdir		= fuse_dev_mkdir,
	.symlink	= NULL,
	.unlink		= fuse_dev_unlink,
	.rmdir		= fuse_dev_rmdir,
	.rename		= fuse_dev_rename,
	.link			= NULL,
	.chmod		= fuse_dev_chmod,
	.chown		= fuse_dev_chown,
	.truncate	= fuse_dev_truncate,
	.utimens	= fuse_dev_utimens,
	.read			= fuse_dev_read,
	.write		= fuse_dev_write,
	.statfs		= fuse_dev_statfs,
	.release	= fuse_dev_release,
	.fsync		= NULL,
	.init			= fuse_dev_init,
	.destroy	= fuse_dev_destroy,
#ifdef HAVE_SETXATTR
	.setxattr	= fuse_dev_setxattr,
	.getxattr	= fuse_dev_getxattr,
	.listxattr	= fuse_dev_listxattr,
	.removexattr	= fuse_dev_removexattr,
#endif
};

int DEFAULT_CC
fuse_run()
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	g_mkdir(mount_point);
	if( g_directory_exist(mount_point) == 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[fuse_run]: "
				"Unable to initialize the mount point : %s", mount_point);
	}

//	umask(0);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_run]: "
			"Configuration of fuse");
	fuse_opt_add_arg(&args, "");
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[fuse_run]: "
			"Setup of the main mount point: %s", mount_point);
	fuse_opt_add_arg(&args, "-f");
	fuse_opt_add_arg(&args, "-o");
	fuse_opt_add_arg(&args, "allow_other");
	fuse_opt_add_arg(&args, mount_point);

	ret = fuse_main(args.argc, args.argv, &fuse_dev_oper, NULL);
	fuse_opt_free_args(&args);

	return ret;
}
