/**
 * Copyright (C) 2010-2011 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010-2011
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


#ifndef RDPFS_CACHE_H_
#define RDPFS_CACHE_H_

#include "rdpfs.h"
#include "hashmap.h"


#define RDPFS_CACHE_VOLUME_SIZE 30
#define RDPFS_CACHE_FS_SIZE 256
/* time after the cache became invalid in second*/
#define RDPFS_CACHE_FS_TIME_EXPIRE 5

struct rdpfs_direntry_cache
{
	struct fs_info *fs_list;
	struct HashMap* fs_map;
	int index;
};


void rdpfs_cache_init();
void rdpfs_cache_dinit();

void APP_CC
rdpfs_cache_add_fs(const char* path, struct fs_info* fs);
struct fs_info* APP_CC
rdpfs_cache_get_fs(const char* fs_name);
void APP_CC
rdpfs_cache_remove_fs(const char* fs_name);
int APP_CC
rdpfs_cache_contain_fs(const char* fs_name);
void APP_CC
rdpfs_cache_update_size(const char* path, size_t size);
void APP_CC
rdpfs_cache_dump();

#endif /* RDPFS_CACHE_H_ */
