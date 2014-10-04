/**
 * Copyright (C) 2010-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010, 2012
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

#include "chansrv.h"
#include <os_calls.h>
#include <list.h>
#include <file.h>
#include <errno.h>
#include "thread_calls.h"


static int g_chansrv_up = 0;
struct log_config log_conf;
static struct channel user_channels[15];
static int channel_count = 0;


/*****************************************************************************/
int APP_CC
chansrv_cleanup(){
	return 0;
}

/*****************************************************************************/
char* APP_CC
chansrv_get_channel_app_property(const char* channel_file_conf, const char* property)
{
  char* name;
  char* value;
  struct list* names;
  struct list* values;
  int index;

  names = list_create();
  names->auto_free = 1;
  values = list_create();
  values->auto_free = 1;
  if (file_by_name_read_section(channel_file_conf, CHANNEL_GLOBAL_CONF, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, property))
      {
        if( g_strlen(value) > 1)
        {
          char* ret = g_strdup(value);
          list_delete(names);
          list_delete(values);
          return ret;
        }
      }
    }
  }
  return NULL;
}

/*****************************************************************************/
int APP_CC
chansrv_transmit(int socket, int type, char* mess, int length, int total_length )
{
  struct stream* header;
  int rv;
  make_stream(header);
  init_stream(header, 9);
  out_uint8(header, type);
  out_uint32_be(header, length);
  out_uint32_be(header, total_length);
  s_mark_end(header);
  rv = g_tcp_send(socket, header->data, 9, 0);
  log_message(&log_conf, LOG_LEVEL_DEBUG_PLUS, "chansrv[chansrv_transmit]: "
  		"Header sended:");
  log_hexdump(&log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)header->data, 9);
  if (rv != 9)
  {
    log_message(&log_conf, LOG_LEVEL_ERROR, "chansrv[chansrv_transmit]: "
    		"Error while sending the header");
    free_stream(header);
    return rv;
  }
  free_stream(header);
  rv = g_tcp_send(socket, mess, length, 0);
  log_message(&log_conf, LOG_LEVEL_DEBUG_PLUS, "chansrv[chansrv_transmit]: "
  		"Message sended:");
  log_hexdump(&log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)mess, length);
  if (rv != length)
  {
    log_message(&log_conf, LOG_LEVEL_ERROR, "chansrv[chansrv_transmit]: "
    		"Error while sending the message: %s", mess);
  }
	return rv;
}

/*****************************************************************************/
int APP_CC
chansrv_launch_server_channel(vchannel* v, int display, char* channel_name)
{
	char channel_file_conf[256];
	char* channel_program_name;
	char channel_program_path[256];
	char* channel_program_arguments;
	char* channel_type;
	struct list* channel_params;
	int pid = 0;

	g_sprintf(channel_file_conf, "%s/%s.conf", XRDP_CFG_PATH, channel_name);
	if (!g_file_exist(channel_file_conf))
	{
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_launch_server_channel]: "
				"Channel %s is not registered ", channel_name);
		return 0;
	}
	channel_program_name = chansrv_get_channel_app_property(channel_file_conf, CHANNEL_APP_NAME_PROP);
	channel_type = chansrv_get_channel_app_property(channel_file_conf, CHANNEL_TYPE_PROP);
	channel_program_arguments = chansrv_get_channel_app_property(channel_file_conf, CHANNEL_APP_ARGS_PROP);

	if (channel_program_name == NULL || channel_type == NULL)
	{
		log_message(&log_conf, LOG_LEVEL_WARNING, "chansrv[chansrv_launch_server_channel]: "
				"Channel conf file for %s is not correct", channel_name);
		return 1;
	}

	if (g_strcmp(channel_type, CHANNEL_TYPE_CUSTOM) == 0)
	{
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_launch_server_channel]: "
				"The custom channel %s must be start by the user", channel_name);
		return 1;
	}

	g_snprintf(channel_program_path, 256, "%s/%s", XRDP_SBIN_PATH, channel_program_name);
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_launch_server_channel]: "
			"Channel app name for %s: %s", channel_name, channel_program_name);
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_launch_server_channel]: "
			"Channel app path for %s: %s", channel_name, channel_program_path);
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_launch_server_channel]: "
				"Channel type for %s: %s", channel_name, channel_type);

	channel_params = list_create();
	channel_params->auto_free = 1;

	/* building parameters */
	list_add_item(channel_params, (long)g_strdup(channel_program_path));
	list_add_item(channel_params, (long)g_strdup(v->username));
	list_add_item(channel_params, 0);

	if( g_strcmp(channel_type, CHANNEL_TYPE_ROOT) == 0)
	{
		pid = g_launch_process(display, channel_params, 0);
	}
	else
	{
		pid = g_su(v->username, display, channel_params, 0);
	}
	if (pid == 0)
	{
		log_message(&log_conf, LOG_LEVEL_WARNING, "chansrv[chansrv_launch_server_channel]: "
					"Unable to launch the channel application %s ", channel_program_path);
		return 1;
	}

	g_free(channel_type);
	g_free(channel_program_name);
	g_free(channel_program_arguments);

	list_delete(channel_params);

	return 0;
}

/*****************************************************************************/
int APP_CC
chansrv_do_up(vchannel* v, char* chan_name)
{
  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_do_up]: "
  		"Activate the channel '%s'", chan_name);
	char socket_filename[256];
	int sock = 0;
	int display;

	display = g_get_display_num_from_display(g_getenv("DISPLAY"));
	g_sprintf(socket_filename, "/var/spool/xrdp/%i/vchannel_%s", display, chan_name);
	sock = g_create_unix_socket(socket_filename);
	g_chown(socket_filename, v->username);

  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_do_up]: "
  		"Channel socket '%s' is created", socket_filename);
  chansrv_launch_server_channel(v, display, chan_name);

	return sock;
}


/*****************************************************************************/
int APP_CC
chansrv_get_channel_from_name(char* channel_name)
{
	int i;
	for(i=0 ; i<channel_count ; i++)
	{
		if(g_strcmp(user_channels[i].channel_name, channel_name) == 0)
		{
			return i;
		}
	}
	return -1;
}


/*****************************************************************************/
int APP_CC
chansrv_get_data_descriptor(vchannel* vc, tbus* robjs, int* rc, tbus* wobjs, int* wc, int* timeout)
{
	int i;
	int j;
	struct channel* channel;

	for(i=0 ; i<channel_count; i++)
	{
		channel = &user_channels[i];
		for(j=0 ; j<channel->client_channel_count; j++)
		{
			robjs[(*rc)++] = channel->client_channel_socket[j];
		}
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
chansrv_has_data(vchannel* v, int channel_id)
{
	struct stream* header;
	struct channel* channel;
	int data_length = 0;
	int size;
	int j;
	int test;
	int type;
	int sock;

	if (channel_id > channel_count)
	{
		return 0;
	}

	channel = &user_channels[channel_id];

	for( j=0 ; j<channel->client_channel_count; j++)
	{
		sock = channel->client_channel_socket[j];
		test = g_is_wait_obj_set(sock);
		if (test)
		{
			log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
					"New data from channel '%s'",channel->channel_name);
			make_stream(header);
			init_stream(header, 5);

			size = g_tcp_recv(sock, header->data, 5, MSG_PEEK);
			if ( size != 5)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
						"Channel %s closed : [%s]", channel->channel_name, g_get_strerror());
				g_tcp_close(sock);
				channel->client_channel_count--;
				channel->client_channel_socket[j] = user_channels[channel_id].client_channel_socket[channel->client_channel_count];
				channel->client_channel_socket[channel->client_channel_count] = 0;

				free_stream(header);
				continue;
			}
			in_uint8(header, type);
			if(type != DATA_MESSAGE)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
						"Invalid operation type");
				free_stream(header);
				return 0;
			}
			in_uint32_be(header, data_length);
			log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
					"Data_length : %i\n", data_length);
			free_stream(header);
		}
	}
	return data_length;
}



/*****************************************************************************/
int APP_CC
chansrv_get_data(vchannel* v, int chanid, struct stream* s)
{
	struct stream* header;
	struct channel* channel;
	int data_length;
	int size;
	int j;
	int test;
	int type;
	int sock;

	channel = &user_channels[chanid];
	for( j=0 ; j < channel->client_channel_count; j++)
	{
		sock = channel->client_channel_socket[j];
		test = g_is_wait_obj_set(sock);
		if (test)
		{
			log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
					"New data from channel '%s'", channel->channel_name);
			make_stream(header);
			init_stream(header, 5);

			size = g_tcp_recv(sock, header->data, 5, 0);
			if ( size != 5)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
						"Channel %s closed : [%s]", channel->channel_name, g_get_strerror());
				g_tcp_close(sock);
				channel->client_channel_count--;
				channel->client_channel_socket[j] = channel->client_channel_socket[channel->client_channel_count];
				channel->client_channel_socket[channel->client_channel_count] = 0;

				free_stream(header);
				continue;
			}
			in_uint8(header, type);
			if(type != DATA_MESSAGE)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
						"Invalid operation type");
				free_stream(header);
				return 0;
			}
			in_uint32_be(header, data_length);
			log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
					"Data_length : %i\n", data_length);
			free_stream(header);

			size = g_tcp_recv(sock, s->data, data_length, 0);
			if ( size != data_length)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
						"Unable to read data message");
				free_stream(s);
				continue;
			}
			//s->data[data_length] = 0;
			log_message(&log_conf, LOG_LEVEL_DEBUG_PLUS, "chansrv[chansrv_check_wait_objs]: "
					"Data:");
			log_hexdump(&log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)s->data, data_length);
			if( channel->channel_id == -1)
			{
				log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
						"Client channel is not opened");
				free_stream(s);
				return 0 ;
			}
		}
	}
	return 0;
}


/*****************************************************************************/
bool APP_CC
chansrv_add(vchannel* v, char* channel_name, int channel_id, int chan_flags)
{
	int sock = 0;
	char status[1];

	if (g_chansrv_up == 0)
	{
		g_chansrv_up = 1;
	}

	if (user_channels[channel_id].client_channel_socket[0] != 0 )
	{
		status[0] = STATUS_CONNECTED;
		chansrv_transmit(sock, SETUP_MESSAGE, status, 1, 1);
		return true;
	}
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_add]: "
			"new client connection for channel '%s' with id= %i ", channel_name, channel_id);
	sock = chansrv_do_up(v, channel_name);
	if (sock < 0)
	{
		log_message(&log_conf, LOG_LEVEL_ERROR, "chansrv[chansrv_add]: "
				"Unable to open channel %s", channel_name);
		return false;
	}

	user_channels[channel_id].channel_id = channel_id;
	g_strcpy(user_channels[channel_id].channel_name, channel_name);
	user_channels[channel_id].server_channel_socket = sock;
	channel_count++;
	return true;
}

/*****************************************************************************/
int APP_CC
chansrv_deinit(void)
{
	char status[1];
	int i;
	int j;
	int socket;
	status[0] = STATUS_DISCONNECTED;

	for (i=0 ; i<channel_count ; i++ )
	{
		for (j=0 ; j<user_channels[i].client_channel_count ; j++ )
		{
			socket = user_channels[i].client_channel_socket[j];
			chansrv_transmit(socket, SETUP_MESSAGE, status, 1, 1 );
		}
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
chansrv_send_data(vchannel* v, unsigned char* data, int chan_id, int chan_flags, int length, int total_length)
{
	int i;

	for(i=0; i<channel_count; i++)
	{
		if( user_channels[i].channel_id == chan_id )
		{
		  log_message(&log_conf, LOG_LEVEL_DEBUG_PLUS, "chansrv[chansrv_data_in]: "
		  		"new client message for channel %s ",user_channels[i].channel_name);
			log_hexdump(&log_conf, LOG_LEVEL_DEBUG_PLUS, data, length);
			//user_channels[i].client_channel_socket[0] -> the main client socket
			if(user_channels[i].client_channel_socket[0] == 0 )
			{
			  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_data_in]: "
			  		"server side channel is not opened");
				return 0;
			}
			chansrv_transmit(user_channels[i].client_channel_socket[0], DATA_MESSAGE, (char*)data, length, total_length);
			return 0;
		}
	}

  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_data_in]: "
  		"the channel id %i is invalid",chan_id);

	return 0;
}

/*****************************************************************************/
int APP_CC
chansrv_get_wait_objs(tbus* objs, int* count)
{
  int lcount;
  int i;

  if ((!g_chansrv_up) || (objs == 0))
  {
    return 0;
  }
  lcount = *count;

  for (i=0 ; i<channel_count ; i++)
  {
  	if( user_channels[i].server_channel_socket != 0)
  	{
  	  objs[lcount] = user_channels[i].server_channel_socket;
  	  lcount++;
  	}
  }
  *count = lcount;
  return 0;
}

/************************************************************************/
int DEFAULT_CC
chansrv_process_channel_opening(int channel_index, int client)
{
	struct stream* s;
  int data_length;
  int size;
  int type;
  int count;
  char status[1];

	make_stream(s);
	init_stream(s, 1024);
	size = g_tcp_recv(client, s->data, sizeof(int)+1, 0);

	if ( size < 1)
	{
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
				"Unable to get information on the opening channel");
		g_tcp_close(client);
		free_stream(s);
		return 0;
	}
	in_uint8(s, type);
	if(type != CHANNEL_OPEN)
	{
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
				"Invalid operation type");
		free_stream(s);
		return 0;
	}
	in_uint32_be(s, data_length);
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
			"Data_length : %i", data_length);
	size = g_tcp_recv(client, s->data, data_length, 0);
	s->data[data_length] = 0;
	log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
			"Channel name : %s",s->data);

	if(g_strcmp(user_channels[channel_index].channel_name, s->data) == 0)
	{
		printf("adding new user channel\n");
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
				"New server connection for channel %s ", user_channels[channel_index].channel_name);
		count = user_channels[channel_index].client_channel_count;
		user_channels[channel_index].client_channel_socket[count] = client;
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
				"Socket : %i", client);
		user_channels[channel_index].client_channel_count++;

		status[0] = STATUS_CONNECTED;
		chansrv_transmit(client, SETUP_MESSAGE, status, 1, 1);
	}
	else
	{
		log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_process_channel_opening]: "
				"Unable to open a channel without client");
	}
	free_stream(s);
	return 0;
}


/*****************************************************************************/
int APP_CC
chansrv_check_wait_objs(void)
{
  int i;
  int new_client;
  int test;

  for( i=0 ; i<channel_count; i++)
  {
    test = g_is_wait_obj_set(user_channels[i].server_channel_socket);
    if (test)
    {
      log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[chansrv_check_wait_objs]: "
           "New server side channel connection for channel %s : ",user_channels[i].channel_name);
      new_client = g_wait_connection(user_channels[i].server_channel_socket);
      chansrv_process_channel_opening(i, new_client);
      return 0;
    }
  }
  return 0;
}

/*****************************************************************************/
THREAD_RV THREAD_CC
channel_thread_loop(void* in_val)
{
  tbus objs[32];
  int num_objs = 0;
  int timeout = 1000;
  THREAD_RV rv = 0;
  vchannel* vc = (vchannel*)in_val;
  vc->stop = false;

  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[channel_thread_loop]: "
  		"channel_thread_loop: thread start");

  while (vc->stop == false)
  {
	if (num_objs > 0)
	{
	  g_obj_wait(objs, num_objs, 0, 0, timeout);
	}

    chansrv_check_wait_objs();
    num_objs = 0;
    chansrv_get_wait_objs(objs, &num_objs);
  }
  chansrv_deinit();

  log_message(&log_conf, LOG_LEVEL_DEBUG, "chansrv[channel_thread_loop]: "
				"channel_thread_loop: thread stop");
  return rv;
}

void APP_CC
chansrv_launch(vchannel* v)
{
  tc_thread_create(channel_thread_loop, v);
}

/*****************************************************************************/
bool chansrv_init(vchannel* vc)
{
  vc->add_channel = chansrv_add;
  vc->has_data = chansrv_has_data;
  vc->get_data = chansrv_get_data;
  vc->send_data = chansrv_send_data;
  vc->get_data_descriptor = chansrv_get_data_descriptor;
  vc->thread_launch = chansrv_launch;

  return true;
}

/*****************************************************************************/
void chansrv_end(vchannel* vc)
{
  vc->stop = true;
  chansrv_deinit();
}


