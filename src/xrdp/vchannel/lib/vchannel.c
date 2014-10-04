/**
 * Copyright (C) 2009-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Vincent ROULLIER <v.roullier@ulteo.com> 2013
 * Author David LECHEVALIER <david@ulteo.com> 2009, 2012, 2013
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

#include "vchannel.h"
#include <file.h>



static Vchannel channel_list[25];
static struct log_config* log_conf;
static int channel_count = 0;

int
_vchannel_send(int sock, int type, const char* data, int length);


/*****************************************************************************/
int
vchannel_add_channel(int sock, const char* name)
{
	strncpy(channel_list[channel_count].name, name, 9);
	channel_list[channel_count].sock = sock;
	channel_count++;
	return 0;
}

/*****************************************************************************/
int
vchannel_remove_channel(int sock)
{
	Vchannel* chan;
	int index = vchannel_get_channel_from_socket(sock);
	if(index == ERROR)
	{
		log_message(log_conf, LOG_LEVEL_WARNING ,"vchannel[vchannel_remove_channel]: "
			"Unable to remove channel");
		return 1;
	}
	chan = &channel_list[index];
	strncpy(chan->name, "         ", 9);
	chan->sock = 0;
	channel_count --;

	if(index != channel_count && channel_count != 0)
	{
		strncpy(channel_list[index].name, channel_list[channel_count].name, 9);
		strncpy(channel_list[channel_count].name, "         ", 9);
		channel_list[index].sock = channel_list[channel_count].sock;
		channel_list[channel_count].sock = 0;
	}
	return 0;
}


/*****************************************************************************/
int
vchannel_try_open(const char* name)
{
	int sock;
	int len;
	struct sockaddr_un saun;
	char socket_filename[256];
	int display_num;

	display_num = g_get_display_num_from_display(getenv("DISPLAY"));
	if(display_num == 0)
	{
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_try_open]: "
				"Display must be different of 0", name);
		return ERROR;
	}

	/* Create socket */
	log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_try_open]: "
			"Creating socket", name);
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_try_open]: "
				"Unable to create socket: %s", name, strerror(errno));
		return ERROR;
	}

	sprintf(socket_filename, "%s/%i/vchannel_%s", VCHANNEL_SOCKET_DIR, display_num, name);
	log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_try_open]: "
			"Socket name : %s", name, socket_filename);
	/* Connect to server */
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, socket_filename);
	len = sizeof(saun.sun_family) + strlen(saun.sun_path);
	if (connect(sock, (struct sockaddr *) &saun, len) < 0)
	{
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_try_open]: "
				"Unable to connect to vchannel server: %s",name, strerror(errno));
		close(sock);
		return ERROR;
	}
	struct stream* s;
	make_stream(s);
	init_stream(s, 256);
	out_uint8p(s, name, strlen(name));
	s_mark_end(s);
	vchannel_add_channel(sock, name);
	if (_vchannel_send(sock,CHANNEL_OPEN, s->data, strlen(name)) == ERROR)
	{
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_try_open]: "
				"Unable to send data", name);
		free_stream(s);
		vchannel_close(sock);
		return ERROR;
	}
	free_stream(s);
	return sock;
}

/*****************************************************************************/
int
vchannel_get_channel_from_socket(int sock)
{
	int i;
	for(i=0 ; i<channel_count ; i++)
	{
		if (channel_list[i].sock == sock)
		{
			return i;
		}
	}
	return ERROR;
}

/*****************************************************************************/
int
vchannel_open(const char* name)
{
	int count = 0;
	int sock;
	log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_open]: "
			"Try to open channel", name);
	while( count < VCHANNEL_OPEN_RETRY_ATTEMPT)
	{
		sock = vchannel_try_open(name);
		if(sock == ERROR)
		{
			log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_open]: "
					"Attempt %i failed on channel", name, count);
			sleep(1);
			count++;
		}
		else
		{
			log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_open]: "
					"Channel openned", name);
			return sock;
		}
	}
	log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_open]: "
			"Failed to open channel [timeout reached]", name);
	return ERROR;
}

/*****************************************************************************/
int APP_CC
vchannel_send(int sock, const char* data, int length)
{
	return _vchannel_send(sock, DATA_MESSAGE, data, length);
}


/*****************************************************************************/
int
_vchannel_send(int sock, int type, const char* data, int length)
{
	struct stream* header;
	int index = vchannel_get_channel_from_socket(sock);
	Vchannel* channel;

	if(index == ERROR)
	{
		log_message(log_conf, LOG_LEVEL_WARNING ,"vchannel[vchannel_send]: "
				"Enable to get channel from socket %i", sock);
		return ERROR;
	}
	channel = &channel_list[index];
	if(sock < 0)
	{
		log_message(log_conf, LOG_LEVEL_WARNING ,"vchannel[vchannel_send]: "
				"The channel %s is not opened ", channel->name);
		return ERROR;
	}
	make_stream(header);
	init_stream(header, 5);
	out_uint8(header, type);
	out_uint32_be(header, length);
	s_mark_end(header);
	log_message(log_conf, LOG_LEVEL_DEBUG_PLUS ,"vchannel{%s}[vchannel_send]: "
			"Header send: ", channel->name);
	log_hexdump(log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)header->data, 5);
	if(g_tcp_send(sock, header->data, 5, 0) < 5)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_send]: "
				"Failed to send message [%s]", channel->name, strerror(errno));
		free_stream(header);
		return ERROR;
	}
	free_stream(header);
	if(g_tcp_send(sock, data, length, 0) < length)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_send]: "
				"Failed to send message data to channel [%s]", channel->name, strerror(errno));
		return ERROR;
	}
	log_message(log_conf, LOG_LEVEL_DEBUG_PLUS ,"vchannel{%s}[vchannel_send]: "
			"Message sended:", channel->name);
	log_hexdump(log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)data, length);

	return 0;
}

/*****************************************************************************/
int
vchannel_receive(int sock, const char* data, int* length, int* total_length)
{
	struct stream* header;
	int nb_read = 0;
	int type;
	int rv;
	Vchannel* channel;
	int index = vchannel_get_channel_from_socket(sock);

	if(index == ERROR)
	{
		log_message(log_conf, LOG_LEVEL_WARNING ,"vchannel[vchannel_receive]: "
				"Enable to get channel from socket %i", sock);
		return ERROR;
	}
	channel = &channel_list[index];
	if(channel->sock < 0)
	{
		log_message(log_conf, LOG_LEVEL_WARNING ,"vchannel{%s}[vchannel_receive]: "
				"The channel is not opened ", channel->name);
		return ERROR;
	}
	make_stream(header);
	init_stream(header, 9);
	nb_read = g_tcp_recv(channel->sock, header->data, 9, 0);
	if (nb_read == 0) // socket is closed
	{
		*length = 0;
		*total_length = 0;
		return 0;
	}

	if( nb_read != 9)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_receive]: "
				"Error while receiving data lenght: %s",channel->name, strerror(errno));
		free_stream(header);
		return ERROR;
	}
	log_message(log_conf, LOG_LEVEL_DEBUG_PLUS ,"vchannel{%s}[vchannel_receive]: "
			"Header received: ", channel->name);
	log_hexdump(log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)header->data, *length);
	in_uint8(header, type);
	in_uint32_be(header, *length);
	in_uint32_be(header, *total_length);
	free_stream(header);

	switch(type)
	{
	case CHANNEL_OPEN:
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_receive]: "
				"CHANNEL OPEN message is invalid message", channel->name);
		rv = ERROR;
		break;
	case SETUP_MESSAGE:
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_receive]: "
				"Status change message", channel->name);
		rv = 0;
		break;
	case DATA_MESSAGE:
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_receive]: "
				"Data message", channel->name);
		rv = 0;
		break;
	default:
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_receive]: "
				"Invalid message '%02x'", channel->name, type);
		rv = ERROR;
		break;
	}
	if( rv == ERROR)
	{
		return ERROR;
	}
	nb_read = g_tcp_recv(channel->sock, (void*)data, *length, 0);
	if (nb_read == 0) // socket is closed
	{
		*length = 0;
		*total_length = 0;
		return 0;
	}

	if (nb_read == -1)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_receive]: "
			"Error while receiving data [%s]", channel->name, strerror(errno));
		return ERROR;
	}
	if(nb_read != *length)
	{
		log_message(log_conf, LOG_LEVEL_ERROR ,"vchannel{%s}[vchannel_receive]: "
			"Data length is invalid", channel->name);
		return ERROR;
	}
	log_message(log_conf, LOG_LEVEL_DEBUG_PLUS ,"vchannel{%s}[vchannel_receive]: "
			"Message received: ", channel->name);
	log_hexdump(log_conf, LOG_LEVEL_DEBUG_PLUS, (unsigned char*)data, *length);
	if (type == SETUP_MESSAGE)
	{
		return data[0];
	}
	return 0;
}

/*****************************************************************************/
int
vchannel_close(int sock)
{
	Vchannel* channel;
	shutdown(sock, SHUT_RDWR);
	int index = vchannel_get_channel_from_socket(sock);
	if(index == ERROR)
	{
		log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel[vchannel_close]: "
				"Enable to get channel from socket %i", sock);
		return ERROR;
	}
	channel = &channel_list[index];
	log_message(log_conf, LOG_LEVEL_DEBUG ,"vchannel{%s}[vchannel_receive]: "
			"Channel closed", channel->name);

	return vchannel_remove_channel(sock);
}

/*****************************************************************************/
int
vchannel_init()
{
  char filename[256];
  char log_filename[256];
  struct list* names;
  struct list* values;
  char* name;
  char* value;
  int index;
  int display_num;

  display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	if(display_num == 0)
	{
		g_printf("vchannel[vchannel_init]: Display must be different of 0\n");
		return ERROR;
	}
	log_conf = g_malloc(sizeof(struct log_config), 1);
  log_conf->program_name = "vchannel";
  log_conf->log_file = 0;
  log_conf->fd = 0;
  log_conf->log_level = LOG_LEVEL_DEBUG;
  log_conf->enable_syslog = 0;
  log_conf->syslog_level = LOG_LEVEL_DEBUG;

  names = list_create();
  names->auto_free = 1;
  values = list_create();
  values->auto_free = 1;
  g_snprintf(filename, 255, "%s/vchannel.ini", XRDP_CFG_PATH);
  if (file_by_name_read_section(filename, VCHAN_CFG_LOGGING, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, VCHAN_CFG_LOG_FILE))
      {
        log_conf->log_file = (char*)g_strdup(value);
      }
      if (0 == g_strcasecmp(name, VCHAN_CFG_LOG_LEVEL))
      {
        log_conf->log_level = log_text2level(value);
      }
      if (0 == g_strcasecmp(name, VCHAN_CFG_LOG_ENABLE_SYSLOG))
      {
        log_conf->enable_syslog = log_text2bool(value);
      }
      if (0 == g_strcasecmp(name, VCHAN_CFG_LOG_SYSLOG_LEVEL))
      {
        log_conf->syslog_level = log_text2level(value);
      }
    }
    if( g_strlen(log_conf->log_file) > 1)
    {
    	g_sprintf(log_filename, "%s/%i/vchannels.log",
    			log_conf->log_file,	display_num);
    	g_free(log_conf->log_file);
    	log_conf->log_file = (char*)g_strdup(log_filename);
    }
  	if(log_start(log_conf) != LOG_STARTUP_OK)
  	{
  		g_printf("vchannel[vchannel_init]: Unable to start log system\n");
  	}
  }
  else
  {
  	g_printf("vchannel[vchannel_init]: Invalid channel configuration file : %s\n", filename);
  	return 1;
  }
  list_delete(names);
  list_delete(values);
  return 0;
}
