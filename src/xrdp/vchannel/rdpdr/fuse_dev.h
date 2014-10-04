/**
 * Copyright (C) 2008 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010
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

#ifndef FUSE_DEV_H_
#define FUSE_DEV_H_

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <vchannel.h>
#include <os_calls.h>
#include <xrdp_constants.h>
#include <file.h>
#include "log.h"
#include "arch.h"
#include "os_calls.h"
#include "parse.h"


int DEFAULT_CC
fuse_run();
int DEFAULT_CC
fuse_dev_add(struct stream* s, int device_data_length, int device_id, char* dos_name);


#endif /* FUSE_DEV_H_ */



