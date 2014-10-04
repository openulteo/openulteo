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

#ifndef CLIPRDR_H_
#define CLIPRDR_H_

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <stdio.h>


/* config constant */
#define CLIPRDR_CFG_GLOBAL						"Globals"
#define CLIPRDR_CFG_NAME							"Name"
#define CLIPRDR_CFG_LOGGING						"Logging"
#define CLIPRDR_CFG_LOG_DIR						"LogDir"
#define CLIPRDR_CFG_LOG_LEVEL					"LogLevel"
#define CLIPRDR_CFG_LOG_ENABLE_SYSLOG	"EnableSyslog"
#define CLIPRDR_CFG_LOG_SYSLOG_LEVEL	"SyslogLevel"

#define MAX_TARGETS 8


/* socket constant */
#define ERROR	-1
#define	SOCKET_ERRNO	errno
#define	ERRNO		errno


/* cliprdr constant */
/* msg type */
#define CB_MONITOR_READY            0x0001
#define CB_FORMAT_LIST              0x0002
#define CB_FORMAT_LIST_RESPONSE     0x0003
#define CB_FORMAT_DATA_REQUEST      0x0004
#define CB_FORMAT_DATA_RESPONSE     0x0005
#define CB_TEMP_DIRECTORY           0x0006
#define CB_CLIP_CAPS                0x0007
#define CB_FILECONTENTS_REQUEST     0x0008
#define CB_FILECONTENTS_RESPONSE    0x0009
#define CB_LOCK_CLIPDATA            0x000A
#define CB_UNLOCK_CLIPDATA          0x000B

/* msg flags */
#define CB_RESPONSE_OK              0x0001
#define CB_RESPONSE_FAIL            0x0002
#define CB_ASCII_NAMES              0x0004

#define SHORT_FORMAT_NAME_SIZE      32

/* clipboard type */
#define CB_CAPSTYPE_GENERAL         0x0001

/* clipboard version */
#define CB_CAPS_VERSION_1           0x00000001
#define CB_CAPS_VERSION_2           0x00000002


/* clipboard general flags */
#define CB_USE_LONG_FORMAT_NAMES    0x00000002
#define CB_STREAM_FILECLIP_ENABLED  0x00000004
#define CB_FILECLIP_NO_FILE_PATHS   0x00000008
#define CB_CAN_LOCK_CLIPDATA        0x00000010


#ifndef CF_TEXT
#define CF_TEXT         1
#define CF_BITMAP       2
#define CF_METAFILEPICT 3
#define CF_SYLK         4
#define CF_DIF          5
#define CF_TIFF         6
#define CF_OEMTEXT      7
#define CF_DIB          8
#define CF_PALETTE      9
#define CF_PENDATA      10
#define CF_RIFF         11
#define CF_WAVE         12
#define CF_UNICODETEXT  13
#define CF_ENHMETAFILE  14
#define CF_HDROP        15
#define CF_LOCALE       16
#define CF_MAX          17
#endif


#endif /* CLIPRDR_H_ */
