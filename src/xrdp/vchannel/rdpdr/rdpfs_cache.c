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


#include "rdpfs_cache.h"
#include "rdpfs.h"
#include "log.h"


static struct rdpfs_direntry_cache direntry_cache;
extern struct log_config *l_config;
static tbus cache_mutex;


static void rdpfs_destroyEntry(void *arg, char *key, void *value)
{
  free(key);
//  free(value);
}

/************************************************************************/
static void rdpfs_update_fs_cache_index()
{
	struct fs_info* fs;
	direntry_cache.index++;

	if( direntry_cache.index == RDPFS_CACHE_FS_SIZE )
	{
		direntry_cache.index = 0;
	}
	fs = &direntry_cache.fs_list[direntry_cache.index];
	fs->allocation_size     = 0;
	fs->create_access_time  = 0;
	fs->delele_request      = 0;
	fs->file_attributes     = 0;
	fs->file_size           = 0;
	fs->is_dir              = 0;
	fs->last_access_time    = 0;
	fs->last_change_time    = 0;
	fs->last_write_time     = 0;
	fs->nlink               = 0;
	if (fs->key && fs->key[0])
	{
		deleteFromHashMap(direntry_cache.fs_map, fs->key);
	}
	fs->filename[0] = 0;
	fs->key[0] = 0;
}

/************************************************************************/
void rdpfs_cache_init()
{
	direntry_cache.fs_list = g_malloc(RDPFS_CACHE_FS_SIZE * sizeof(struct fs_info), 1);
	direntry_cache.index = 0;
	direntry_cache.fs_map = newHashMap(rdpfs_destroyEntry, NULL);
	direntry_cache.fs_map->mapSize = 0;
	cache_mutex = tc_mutex_create();
}

/************************************************************************/
void rdpfs_cache_dinit()
{
	g_free(direntry_cache.fs_list);
	deleteHashMap(direntry_cache.fs_map);
}




/************************************************************************/
void APP_CC
rdpfs_cache_add_fs(const char* path, struct fs_info* fs_inf)
{
	struct fs_info* fs;
	tc_mutex_lock(cache_mutex);

	/* test if present */
	fs = (struct fs_info*)getFromHashMap(direntry_cache.fs_map, path);

	if( fs == NULL)
	{
		fs = &direntry_cache.fs_list[direntry_cache.index];
	}

	fs->allocation_size = fs_inf->allocation_size;
	fs->create_access_time = fs_inf->create_access_time;
	fs->delele_request = fs_inf->delele_request;
	fs->file_attributes = fs_inf->file_attributes;
	fs->file_size = fs_inf->file_size;
	fs->is_dir = fs_inf->is_dir;
	fs->last_access_time = fs_inf->last_access_time;
	fs->last_change_time = fs_inf->last_change_time;
	fs->last_write_time = fs_inf->last_write_time;
	fs->nlink = fs_inf->nlink;
	strncpy(fs->filename, fs_inf->filename, 256);
	strncpy(fs->key, path, 256);
	fs->time_stamp = g_time1();

	addToHashMap(direntry_cache.fs_map, path, fs);
	rdpfs_update_fs_cache_index();

	tc_mutex_unlock(cache_mutex);
	return;
}

/************************************************************************/
void APP_CC
rdpfs_cache_update_size(const char* path, size_t size)
{
	struct fs_info* fs;
	tc_mutex_lock(cache_mutex);

	/* Test if present */
	fs = (struct fs_info*)getFromHashMap(direntry_cache.fs_map, path);

	if( fs == NULL)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_cache_update_size]: "
		    		"Unable to find %s", path);
		tc_mutex_unlock(cache_mutex);
		return;
	}

	fs->allocation_size = size;
	fs->file_size = size;

	tc_mutex_unlock(cache_mutex);
	return;
}

/************************************************************************/
struct fs_info*
rdpfs_cache_get_fs(const char* fs_name)
{
	struct fs_info* temp = NULL;
	int time = 0;
	tc_mutex_lock(cache_mutex);

	temp = (struct fs_info*)getFromHashMap(direntry_cache.fs_map, fs_name);
	if (temp == NULL)
	{
		tc_mutex_unlock(cache_mutex);
		return NULL;
	}
	time = g_time1()-temp->time_stamp;
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_cache_get_fs]: "
	    		"Cached value have been updated %i second ago", time);
	if (time > RDPFS_CACHE_FS_TIME_EXPIRE)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[rdpfs_cache_get_fs]: "
				"Cached value expired");
		tc_mutex_unlock(cache_mutex);
		return NULL;
	}

	tc_mutex_unlock(cache_mutex);
	return temp;
}

/************************************************************************/
void APP_CC
rdpfs_cache_remove_fs(const char* fs_name)
{
	deleteFromHashMap(direntry_cache.fs_map, fs_name);
}

/************************************************************************/
int APP_CC
rdpfs_cache_contain_fs(const char* fs_name)
{
	return 1;
}

/************************************************************************/
static int dump(void *arg, const char *key, void *value)
{
	struct fs_info* temp = value;
	 log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[rdpfs_send]: "
			 "\t key:%s -> value:%s , key2 : %s", key, temp->filename, temp->key);
	return 1;
}

/************************************************************************/
void APP_CC
rdpfs_cache_dump()
{
	iterateOverHashMap(direntry_cache.fs_map, dump, 0);
}
