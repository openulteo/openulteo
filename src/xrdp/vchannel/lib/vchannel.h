/**
 * Copyright (C) 2009 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com>
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


#ifndef VCHANNEL_H_
#define VCHANNEL_H_

#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>
#include <os_calls.h>
#include "log.h"
#include "parse.h"
#include "list.h"

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define VCHANNEL_SOCKET_DIR	"/var/spool/xrdp"
#define ERROR		-1
#define VCHANNEL_OPEN_RETRY_ATTEMPT		12

#define VCHAN_CFG_LOGGING						"Logging"
#define VCHAN_CFG_LOG_FILE					"LogDir"
#define VCHAN_CFG_LOG_LEVEL					"LogLevel"
#define VCHAN_CFG_LOG_ENABLE_SYSLOG	"EnableSyslog"
#define VCHAN_CFG_LOG_SYSLOG_LEVEL	"SyslogLevel"

#define CHANNEL_PDU_LENGTH			1600
/* message type */
#define SETUP_MESSAGE			0x01
#define DATA_MESSAGE			0x02
#define CHANNEL_OPEN			0x03

/* status flags */
#define STATUS_NORMAL						0x00
#define STATUS_DISCONNECTED			0x01
#define STATUS_CONNECTED				0x02


typedef struct{
	char name[9];
	int sock;
} Vchannel;

int APP_CC
vchannel_open(const char* name);
int APP_CC
vchannel_send(int sock, const char* data, int length);
int APP_CC
vchannel_receive(int sock, const char* data, int* length, int* total_length);
int APP_CC
vchannel_close(int sock);
int APP_CC
vchannel_init();
int
vchannel_get_channel_from_socket(int sock);
int
vchannel_try_open(const char* name);

#endif /* VCHANNEL_H_ */
