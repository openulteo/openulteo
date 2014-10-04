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

#ifndef RDPFS_H_
#define RDPFS_H_

#include "rdpdr.h"
#include "log.h"
#include "vchannel.h"
#include "shares.h"
#include "printers.h"
#include "rdpfs_cache.h"
#include "thread_calls.h"
#include "uni_rdp.h"
#include "cp1252_rdp.h"
#include <sys/stat.h>

#define MAX_SHARE 128
#define MAX_SIZE	6000

#define RDPDRIVE_NAME    ".rdp_drive"
#define PRINTER_DIR      ".rdp_printer"


struct disk_device
{
	int device_id;
	int device_type;
	char dir_name[1024];
	int open;
	int completion_id;
};

struct fs_info
{
	uint64_t create_access_time;
	uint64_t last_access_time;
	uint64_t last_write_time;
	uint64_t last_change_time;
	int file_attributes;
	size_t file_size;
	size_t allocation_size;    /* number of occupied cluster */
	int nlink;
	int delele_request;
	int is_dir;
	char filename[256];
	char key[256];
	/* for cache */
	int time_stamp;
};


struct request_response
{
	int Request_type;
	int Request_param;
	struct fs_info fs_inf;
	unsigned char *buffer;
	int buffer_length;
	int request_status;
	barrier_t barrier;
} ;


int APP_CC
rdpfs_close();
int APP_CC
rdpfs_process_message(struct stream* s, int length, int total_length);
int APP_CC
rdpfs_receive(const char* data, int* length, int* total_length);
int APP_CC
rdpfs_open();
void APP_CC
rdpfs_wait_reply(int completion_id);
int APP_CC
rdpfs_get_device_count();
struct disk_device* APP_CC
rdpfs_get_device_by_index(int device_index);
struct disk_device* APP_CC
rdpfs_get_dir(int device_id);
int APP_CC
rdpfs_add(struct stream* s, int device_data_length, int device_id, char* device_name, int device_type);
void APP_CC
rdpfs_remove(int device_id);
struct disk_device* APP_CC
rdpfs_get_device_from_path(const char* path);
int APP_CC
rdpfs_is_printer_device(const char* path);
int APP_CC
rdpfs_send(struct stream* s);
void APP_CC
rdpfs_query_volume_information(int completion_id, int device_id, int information, const char* query );
void APP_CC
rdpfs_query_information(int completion_id, int device_id, int information, const char* query );
void APP_CC
rdpfs_query_setinformation(int completion_id, int information, struct fs_info* fs );
void APP_CC
rdpfs_query_directory(int completion_id, int device_id, int information, const char* query );
int APP_CC
rdpfs_create(int device_id, int desired_access, int shared_access, int creation_disposition, int flags, const char* path);
int APP_CC
rdpfs_request_read(int completion_id, int device_id, int length, int offset);
int APP_CC
rdpfs_request_write(int completion_id, int offset, int length);
void APP_CC
rdpfs_query_volume_information(int completion_id, int device_id, int information, const char* query );
void APP_CC
rdpfs_request_close(int completion_id, int device_id);
int APP_CC
rdpfs_convert_fs_to_stat(struct fs_info* fs, struct stat* st);
int APP_CC
rdpfs_get_other_permission(mode_t mode);
int APP_CC
rdpfs_get_owner_permission(mode_t mode);

#endif /* RDPFS_H_ */
