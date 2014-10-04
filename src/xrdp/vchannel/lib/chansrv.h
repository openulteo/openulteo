/**
 * Copyright (C) 2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com>
 * Author Vincent Roullier <v.roullier@ulteo.com> 2014
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

#ifndef USER_CHANNEL_H
#define USER_CHANNEL_H

#include "arch.h"
#include "parse.h"
#include "log.h"


#define SETUP_MESSAGE			0x01
#define DATA_MESSAGE			0x02
#define CHANNEL_OPEN			0x03

#define STATUS_DISCONNECTED			0x01
#define STATUS_CONNECTED				0x02


/* configuration file section and param */
#define CHANNEL_GLOBAL_CONF			"Globals"
#define CHANNEL_APP_NAME_PROP		"ApplicationName"
#define CHANNEL_APP_PATH_PROP		"ApplicationPath"
#define CHANNEL_APP_ARGS_PROP		"ApplicationArguments"
#define CHANNEL_TYPE_PROP				"ChannelType"
#define CHANNEL_TYPE_ROOT				"RootChannel"
#define CHANNEL_TYPE_USER				"UserChannel"
#define CHANNEL_TYPE_CUSTOM				"CustomChannel"

#define CHANSRV_LIBRARY  "libxrdp_chansrv.so"

typedef struct _vchannel
{
	bool stop;
	char* username;
	tbus handle;
	tbus session;
	tbus thread_handle;

	bool (*init)(struct _vchannel*);
	void (*exit)(struct _vchannel*);

	bool (*add_channel)(struct _vchannel*, char*, int, int);
	int (*has_data)(struct _vchannel*, int);
	int (*thread_launch)(struct _vchannel*);

	int (*get_data)(struct _vchannel*, int, struct stream* s);
	int (*send_data)(struct _vchannel*, unsigned char*, int, int, int, int);
	int (*get_data_descriptor)(struct _vchannel*, tbus*, int*, tbus*, int*, int*);

} vchannel;

struct channel
{
	int channel_id;
	char channel_name[9];
	int client_channel_socket[5];
	int client_channel_count;
	int server_channel_socket;
	struct list* psooled_packet;
};

int APP_CC
chansrv_do_up(vchannel* v, char* chan_name);

bool APP_CC
chansrv_add(vchannel* v, char* channel_name, int channel_id, int chan_flags);
void APP_CC
chansrv_launch(vchannel* v);
int APP_CC
chansrv_deinit(void);
int APP_CC
chansrv_send_data(vchannel* v, unsigned char* data, int chan_id, int chan_flags, int length, int total_length);
int APP_CC
chansrv_get_wait_objs(tbus* objs, int* count);
int APP_CC
chansrv_check_wait_objs(void);
int APP_CC
chansrv_has_data(vchannel* v, int channel_id);
int APP_CC
chansrv_get_data(vchannel* v, int chanid, struct stream* s);

#endif
