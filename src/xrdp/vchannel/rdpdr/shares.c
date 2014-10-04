/**
 * Copyright (C) 2008-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010, 2014
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


#include <shares.h>

extern struct log_config *l_config;


/* A table of the ASCII chars from space (32) to DEL (127) */
static const char ACCEPTABLE_URI_CHARS[96] = {
  /*      !    "    #    $    %    &    '    (    )    *    +    ,    -    .    / */
  0x00,0x3F,0x20,0x20,0x28,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x2A,0x28,0x3F,0x3F,0x1C,
  /* 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x20,
  /* @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
  0x38,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _ */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F,
  /* `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
  0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~  DEL */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20
};

static const char HEX_CHARS[16] = "0123456789ABCDEF";


/*****************************************************************************/
static char*
share_convert_path(char* path)
{
	char* buffer = NULL;
	int i = 0;
	int buffer_len = 0;
	int path_len = 0;
	unsigned char c = 0;
	char* t = 0;

	path_len = g_strlen(path);
	buffer_len = path_len * 3 + 1;
	buffer = g_malloc(buffer_len, 1);

	t = buffer;

	/* copy the path component name */
	for (i = 0 ; i< path_len ; i++)
	{
		c = path[i];
		if (!ACCEPTABLE_URI_CHAR (c) && (c != '\n'))
		{
			*t++ = '%';
			*t++ = HEX_CHARS[c >> 4];
			*t++ = HEX_CHARS[c & 15];
		}
		else
		{
			*t++ = path[i];
		}
	}
	*t = '\0';

	return buffer;
}

/*****************************************************************************/
static char*
share_get_share_path(const char* sharename)
{
	char* share_path = NULL;
	char* home_dir = g_getenv("HOME");

	share_path = g_malloc(1024, 1);

	if (sharename && sharename[0])
	{
		g_snprintf(share_path, 1024, "%s/%s/%s", home_dir, RDPDRIVE_NAME, sharename);
	}
	return share_path;
}

/*****************************************************************************/
static char*
share_get_share_link(const char* sharename, const char* client_name)
{
	char* share_link = NULL;
	char* home_dir = g_getenv("HOME");

	share_link = g_malloc(1024, 1);

	if (sharename && sharename[0])
	{
		g_snprintf(share_link, 1024, "%s/%s on (%s)", home_dir, sharename, client_name);
	}
	return share_link;
}



/*****************************************************************************/
struct list*
share_get_bookmarks_list()
{
	int file_size = 0;
	char* home_dir = g_getenv("HOME");
	char* buffer = NULL;
	char* pos = NULL;
	char* pos2 = NULL;
	char* item = NULL;
	int fd = 0;
	struct list* bookmarks = NULL;
	char bookmark_file_path[1024] = {0};

	g_snprintf((char*)bookmark_file_path, sizeof(bookmark_file_path), "%s/%s", home_dir, BOOKMARK_FILENAME);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_get_bookmarks_list]: "
			"Bookmark file: %s", bookmark_file_path);


	if (g_file_exist(bookmark_file_path) == 0){
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Bookmark file do not exist");
		goto fail;
	}

	file_size = g_file_size(bookmark_file_path);
	if (file_size < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Unable to get size of file %s", bookmark_file_path);
		goto fail;
	}

	buffer = g_malloc(file_size+1, 1);
	fd = g_file_open(bookmark_file_path);
	if (fd < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Unable to open file %s", bookmark_file_path);
		goto fail;
	}

	if (g_file_read(fd, buffer, file_size) < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Unable to read the file %s [%s]", bookmark_file_path, strerror(errno));
		goto fail;
	}

	bookmarks = list_create();
	bookmarks->auto_free = 1;

	pos = buffer;
	while(pos != NULL)
	{
		pos2 = g_strchr(pos, '\n');
		if (pos2 == NULL)
		{
			if (g_strlen(pos) > 1)
			{
				item = g_strdup(pos);
				g_strtrim(item, 3);
				list_add_item(bookmarks, (long)item);
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_get_bookmarks_list]: "
						"Add bookmark %s to bookmarks list", pos);
			}
			pos = pos2;
			continue;
		}
		*pos2 = 0;
		item = g_strdup(pos);
		g_strtrim(item, 3);
		list_add_item(bookmarks, (long)item);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Add bookmark %s to bookmarks list", pos);

		pos = pos2 + 1;
	}
	if (buffer)
	{
		g_free(buffer);
	}
	return bookmarks;

fail:
	if (buffer)
	{
		g_free(buffer);
	}
	bookmarks = list_create();
	bookmarks->auto_free = 1;

	return bookmarks;
}

/*****************************************************************************/
int
share_save_bookmark(struct list* bookmarks)
{
	int fd = 0;
	int i = 0;
	char* bookmark = NULL;
	char* home_dir = g_getenv("HOME");
	char bookmark_file_path[1024] = {0};

	g_snprintf((char*)bookmark_file_path, sizeof(bookmark_file_path), "%s/%s", home_dir, BOOKMARK_FILENAME);

	if (g_file_exist(bookmark_file_path)){
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_get_bookmarks_list]: "
				"Bookmark already exist, delete it");
		g_file_delete(bookmark_file_path);
	}

	fd = g_file_open(bookmark_file_path);
	if (fd < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_save_bookmark]: "
				"Unable to open file %s", bookmark_file_path);
		return 1;
	}

  for (i=0; i<(bookmarks->count); i++)
  {
  	bookmark = (char*)list_get_item(bookmarks, i);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_save_bookmark]: "
				"Test item %s", bookmark);
  	g_file_write(fd, bookmark, g_strlen(bookmark));
  	g_file_write(fd, "\n", 1);
  }

  g_file_close(fd);
  return 0;
}


/*****************************************************************************/
static int file_contain(char* filename, char* pattern)
{
	char* buffer = NULL;
	char* pos = NULL;
	int res = 0;
	int file_size;
	int fd = 0;

	if (! g_file_exist(filename))
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[file_contain]: "
				"%s do not exist", filename);
		return 1;
	}
	file_size = g_file_size(filename);
	if (file_size < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[file_contain]: "
				"Unable to get size of file %s", filename);
		return 1;
	}
	buffer = g_malloc(file_size, 1);
	fd = g_file_open(filename);
	if (fd < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[file_contain]: "
		        		"Unable to open file %s [%s]", filename, strerror(errno));
		goto fail;
	}
	if (g_file_read(fd, buffer, file_size) < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[file_contain]: "
				"Unable to read the file %s [%s]", filename, strerror(errno));
		goto fail;
	}
	pos = g_strstr(buffer, pattern);
	if (pos == NULL)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[file_contain]: "
				"Unable to find the pattern %s in the file %s", pattern, filename);
		res = 1;
	}

	if (buffer)
	{
		g_free(buffer);
	}	return res;

fail:
	if (buffer)
	{
		g_free(buffer);
	}
	return 1;
}


/*****************************************************************************/
int
share_desktop_purge()
{
	char* home_dir = g_getenv("HOME");
	char path[1024] = {0};
	char share_preffix[1024] = {0};
	char desktop_file_path[1024] = {0};
	struct dirent *dir_entry = NULL;
	DIR *dir;

	g_snprintf((char*)desktop_file_path, sizeof(desktop_file_path), "%s/Desktop", home_dir);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_purge]: "
	        		"Desktop file path : %s", desktop_file_path);

	g_snprintf(share_preffix, sizeof(desktop_file_path), "%s/%s", home_dir, RDPDRIVE_NAME);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_purge]: "
		        		"Share path : %s", share_preffix);

	dir = opendir(desktop_file_path);
	if( dir == NULL)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_purge]: "
		        		"Unable to open the directory: %s", desktop_file_path);
		return 1;
	}
	while ((dir_entry = readdir(dir)) != NULL)
	{
		if((g_strcmp(dir_entry->d_name, ".") == 0)
				|| (g_strcmp(dir_entry->d_name, "..") == 0)
				|| ((dir_entry->d_type == DT_DIR) == 0))
		{
			continue;
		}
		g_snprintf(path, sizeof(path), "%s/%s", desktop_file_path, dir_entry->d_name);

		if (g_str_end_with(path, ".desktop") == 0)
		{
			if (file_contain(path, share_preffix) == 0)
			{
				log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_purge]: "
					        		"Desktop file : %s", path);
				g_file_delete(path);
			}
		}
	}

	closedir(dir);
	return 0;
}

/*****************************************************************************/
int
share_add_to_desktop(const char* share_name)
{
	char* home_dir = g_getenv("HOME");
	char desktop_file_path[1024] = {0};
	char share_path[256] = {0};
	char file_content[1024] = {0};
	int handle = 0;

	g_snprintf((char*)desktop_file_path, 256, "%s/Desktop", home_dir);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_desktop]: "
	        		"Desktop file path : %s", desktop_file_path);


	g_mkdir(desktop_file_path);
	if (g_file_exist(desktop_file_path) == 0){
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_add_to_desktop]: "
		        		"Desktop already exist");
		return 1;
	}

	g_snprintf((char*)desktop_file_path, sizeof(desktop_file_path), "%s/Desktop/%s.desktop", home_dir, share_name);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_desktop]: "
	        		"Desktop file path : %s", desktop_file_path);

	if (g_file_exist(desktop_file_path) != 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_add_to_desktop]: "
		        		"Share already exist");
		return 0;
	}

	g_snprintf(share_path, sizeof(desktop_file_path), "%s/%s/%s", home_dir, RDPDRIVE_NAME, share_name);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_desktop]: "
		        		"Share path : %s", share_path);

	g_strcpy(file_content, DESKTOP_FILE_TEMPLATE);
	g_str_replace_first(file_content, "%ShareName%", (char*)share_name);
	g_str_replace_first(file_content, "%SharePath%", share_path);

	handle = g_file_open(desktop_file_path);
	if(handle < 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_desktop]: "
		        		"Unable to create : %s", desktop_file_path);
		return 1;
	}
	g_file_write(handle, file_content, g_strlen(file_content));
	g_file_close(handle);

	return 0;
}

/*****************************************************************************/
int share_remove_from_desktop(const char* share_name){
	char* home_dir = g_getenv("HOME");
	char desktop_file_path[1024] = {0};

	g_snprintf((char*)desktop_file_path, sizeof(desktop_file_path), "%s/Desktop", home_dir);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_desktop]: "
	        		"Desktop file path : %s", desktop_file_path);

	g_snprintf((char*)desktop_file_path, sizeof(desktop_file_path), "%s/Desktop/%s.desktop", home_dir, share_name);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_desktop]: "
	        		"Desktop file path : %s", desktop_file_path);

	if (g_file_exist(desktop_file_path) != 0)
	{
		g_file_delete(desktop_file_path);
		return 0;
	}

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_desktop]: "
			"Desktop file did not exixt");

	return 1;
}

/*****************************************************************************/
int share_add_to_bookmarks(const char* share_name, const char* client_name){
	char* home_dir = g_getenv("HOME");
	char bookmark_file_content[1024] = {0};
	char *escaped_bookmark_file_content;
	struct list* bookmarks = NULL;

	bookmarks = share_get_bookmarks_list();

	g_snprintf(bookmark_file_content, sizeof(bookmark_file_content), "%s%s/%s on (%s)", FILE_PREFFIX, home_dir, share_name, client_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_bookmark]: "
			"Entry to add: %s", bookmark_file_content);

	escaped_bookmark_file_content = share_convert_path(bookmark_file_content);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_bookmark]: "
			"Entry escaped added: %s", escaped_bookmark_file_content);
	list_add_item(bookmarks, (long)g_strdup(escaped_bookmark_file_content));

	share_save_bookmark(bookmarks);
	list_delete(bookmarks);
	return 0;
}



/*****************************************************************************/
int share_remove_from_bookmarks(const char* share_name, const char* client_name){
	int i = 0;
	char* home_dir = g_getenv("HOME");
	char bookmark_file_content[1024] = {0};
	char *escaped_bookmark_file_content = NULL;
	char* bookmark = NULL;
	struct list* bookmarks = NULL;
	struct list* new_bookmarks = NULL;


	bookmarks = share_get_bookmarks_list();
	if (share_name == NULL)
	{
		g_snprintf(bookmark_file_content, sizeof(bookmark_file_content), "on (", FILE_PREFFIX, home_dir, RDPDRIVE_NAME);
	}
	else
	{
		g_snprintf(bookmark_file_content, sizeof(bookmark_file_content), "%s%s/%s on (%s)", FILE_PREFFIX, home_dir, share_name, client_name);
	}


	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_bookmarks]: "
			"Entry to remove: %s", bookmark_file_content);

	escaped_bookmark_file_content = share_convert_path(bookmark_file_content);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_bookmarks]: "
			"Entry escaped to remove: %s", escaped_bookmark_file_content);

	new_bookmarks = list_create();
	new_bookmarks->auto_free = 1;
	for (i=0; i<(bookmarks->count); i++)
	{
		bookmark = (char*)list_get_item(bookmarks, i);
		if (bookmark == NULL)
		{
			continue;
		}
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_bookmarks]: "
				"Test item %s", bookmark);
  	if (g_strstr(bookmark, escaped_bookmark_file_content) == 0)
		{
  		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_remove_from_bookmarks]: "
  				"keep the item %s", bookmark);
			list_add_item(new_bookmarks, (long)g_strdup(bookmark));
			continue;
		}
	}
	list_delete(bookmarks);
  list_dump_items(new_bookmarks);

	share_save_bookmark(new_bookmarks);
	list_delete(new_bookmarks);
	if (escaped_bookmark_file_content)
	{
		g_free(escaped_bookmark_file_content);
	}
	return 0;
}

/*****************************************************************************/
int
share_bookmark_purge()
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_bookmark_purge]: "
			"Purge bookmarks");
	return share_remove_from_bookmarks(NULL, NULL);
}

/*****************************************************************************/
int share_add_to_symlink(const char* share_name, const char* client_name){
	char* rdpdr_path = NULL;
	char* symlink_path = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_symlink]: "
			"Create symlink for share %s", share_name);

	rdpdr_path = share_get_share_path(share_name);
	symlink_path = share_get_share_link(share_name, client_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_symlink]: "
			"Creating the symlink: %s -> %s", symlink_path, rdpdr_path);
	if (g_create_symlink(rdpdr_path, symlink_path) < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_add_to_symlink]: "
				"Unable to create the symlink: %s -> %s [%s]", symlink_path, rdpdr_path, strerror(errno));
		goto fail;
	}

	if (rdpdr_path)
	{
		g_free(rdpdr_path);
	}
	if (symlink_path)
	{
		g_free(symlink_path);
	}
	return 0;


fail:
	if (rdpdr_path)
	{
		g_free(rdpdr_path);
	}
	if (symlink_path)
	{
		g_free(symlink_path);
	}
	return 1;
}



/*****************************************************************************/
int share_remove_from_symlink(const char* share_name, const char* client_name){
	char* symlink_path = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_symlink]: "
			"Remove symlink for share %s", share_name);

	symlink_path = share_get_share_link(share_name, client_name);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_add_to_symlink]: "
			"Removing the symlink: %s", symlink_path);
	if (g_file_delete(symlink_path) < 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpdr[share_add_to_symlink]: "
				"Unable to remove the symlink: %s [%s]", symlink_path, strerror(errno));
		goto fail;
	}

	if (symlink_path)
	{
		g_free(symlink_path);
	}
	return 0;


fail:
	if (symlink_path)
	{
		g_free(symlink_path);
	}
	return 1;


}

/*****************************************************************************/
int
share_symlink_purge()
{
	char* home_dir = g_getenv("HOME");
	char path[1024] = {0};
	char link[1024] = {0};
	char path_preffix[1024] = {0};
	DIR *dir;
	struct dirent *dir_entry = NULL;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpdr[share_symlink_purge]: "
			"Purge symlink\n");

	g_snprintf(path_preffix, sizeof(path_preffix), "%s/%s", home_dir, RDPDRIVE_NAME);

	dir = opendir(home_dir);
	if( dir == NULL)
	{
		return 0;
	}
	while ((dir_entry = readdir(dir)) != NULL)
	{
		if( 	 (g_strcmp(dir_entry->d_name, ".") == 0)
				|| (dir_entry->d_name[0] == '.')
				|| (g_strcmp(dir_entry->d_name, "..") == 0)
				|| (dir_entry->d_type != DT_LNK))
		{
			continue;
		}
		g_snprintf(path, sizeof(path), "%s/%s", home_dir, dir_entry->d_name);


		if (readlink(path, link, sizeof(link)) < 0)
		{
			log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpdr[share_symlink_purge]: "
					"Unable to get symlink for %s", path);
			continue;
		}
		if (link[0] == '\0')
		{
			continue;
		}
  	if (g_strstr(link, path_preffix) != 0)
  	{
  		g_file_delete(path);
  	}
	}
	closedir(dir);

	return 0;
}
