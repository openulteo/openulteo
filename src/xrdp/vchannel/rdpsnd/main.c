/* $Id: module-rdp-sink.c 2043 2007-11-09 18:25:40Z lennart $ */

/***
  This file is part of PulseAudio.

  Copyright 2004-2006 Lennart Poettering

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

/**
 * Copyright (C) 2010-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechavalier <david@ulteo.com> 2010-2013
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <log.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <os_calls.h>
#include <list.h>
#include "sound_channel.h"

extern int completion_count;
extern RD_WAVEFORMATEX server_format;
struct log_config* l_config;
int pulseaudio_pid;
int display_num;

void sound_process(const void *data, size_t size) {
  	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[sound_process]: "
  			"Process sound trame of size %i", size);

    int stamp = 0;
    vchannel_sound_send_wave_info(stamp, size, data);
}

/*****************************************************************************/
void *thread_sound_process (void * arg)
{
	char* buffer;
  int error;
  int block_size;
  int factor;
  int i = 0;

  pa_sample_spec ss;
  if (server_format.wBitsPerSample == 16)
  {
    ss.format = PA_SAMPLE_S16LE;
  }
  else
  {
    ss.format = PA_SAMPLE_U8;
  }
  ss.channels = server_format.nChannels;
  ss.rate = server_format.nSamplesPerSec;
  pa_simple *s = NULL;
//  block_size = pa_bytes_per_second(&ss) / 20; /* 50 ms */
//  if (block_size <= 0)
//      block_size = pa_frame_size(&ss);
  server_format.nAvgBytesPerSec = server_format.wBitsPerSample * server_format.nChannels * server_format.nSamplesPerSec / 8;
  factor = (int) server_format.nAvgBytesPerSec / MAX_BLOCK_SIZE;
  if (factor == 0)
  {
    factor = 1;
  }
  block_size =  server_format.nAvgBytesPerSec /factor;
  log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_sound_process]: "
			" Block size : %i", block_size);

  buffer = g_malloc(block_size, 0);

  /* Create the recording stream */
  s = pa_simple_new(NULL, "vchannel_rdpsnd", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error);
  while (s == 0) {
  	g_sleep(1000);
  	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_sound_process]: "
  			"Unable to connect to pulseaudio server, next retry in one second");
    s = pa_simple_new(NULL, "vchannel_rdpsnd", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error);
  }


	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_sound_process]: "
			"Beginning the main loop");
  for (;;) {
      /* Record some data ... */
      if (pa_simple_read(s, buffer, block_size, &error) < 0) {
        if (error == PA_ERR_CONNECTIONTERMINATED)
        {
          goto finish;
        }
      	log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[thread_sound_process]: "
      			"pa_simple_read() failed: %s", pa_strerror(error));
      	goto finish;
      }
      for (i = 0 ; i < block_size ; i++) {
        if (buffer[i]) {
          sound_process(buffer , block_size);
          break;
        }
      }
  }


  finish:
      if (s)
          pa_simple_free(s);
  g_free(buffer);
  vchannel_sound_deinit();

	pthread_exit (0);
}

/*****************************************************************************/
int make_pa_from_config(char* client_pa_file)
{
	char pa_config_path[256] = {0};
	char opt_line[256] = {0};
	int opt_line_len = 0;
	int file_size = 0;
	int fd = 0;
	char* buffer = NULL;


	g_snprintf(pa_config_path, 256, "%s/%s", XRDP_CFG_PATH, "rdpsnd.pa");

	if (server_format.wBitsPerSample == 8)
	{
		g_snprintf(opt_line, 256, "load-module module-null-sink channels=%i rate=%i format=u8",
				server_format.nChannels, server_format.nSamplesPerSec);
	}
	else
	{
		g_snprintf(opt_line, 256, "load-module module-null-sink channels=%i rate=%i format=s%ile",
				server_format.nChannels, server_format.nSamplesPerSec, server_format.wBitsPerSample);
	}


	file_size = g_file_size(pa_config_path);
	if (file_size < 0 )
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to get the size of the file %s [%s]", pa_config_path, strerror(errno));
		goto fail;
	}

	fd = g_file_open(pa_config_path);
	if (fd < 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to open the file %s [%s]", pa_config_path, strerror(errno));
		goto fail;
	}
	buffer = g_malloc(file_size + 1, 1);
	if (buffer == NULL)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to allocate memory to store the file", pa_config_path, strerror(errno));
		goto fail;
	}
	if (g_file_read(fd, buffer, file_size) != file_size)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to read the file", pa_config_path, strerror(errno));

		goto fail;
	}
	g_file_close(fd);
	fd = 0;

	fd = g_file_open(client_pa_file);
	if (fd < 0)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to open the file %s [%s]", client_pa_file, strerror(errno));
		goto fail;
	}

	if (g_file_write(fd, buffer, file_size) != file_size )
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to write buffer to the file %s [%s]", client_pa_file, strerror(errno));
		goto fail;
	}

	if (g_file_write(fd, "\n", 1) != 1)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to write buffer to the file %s [%s]", client_pa_file, strerror(errno));
		goto fail;
	}

	opt_line_len = g_strlen(opt_line);
	if (g_file_write(fd, opt_line, opt_line_len) != opt_line_len)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[make_pa_from_config]: "
				"Unable to write buffer to the file %s [%s]", client_pa_file, strerror(errno));
		goto fail;
	}

	g_file_close(fd);
	if (buffer)
	{
		g_free(buffer);
	}
	return 0;

fail:
	if (buffer)
	{
		g_free(buffer);
	}
	if (fd)
	{
		g_file_close(fd);
	}
	return 1;
}




/*****************************************************************************/
int start_pulseaudio()
{
	struct list *args;
	int status = 0;
	char client_pa_config_path[256] = {0};
	char home_pulse_dir[256] = {0};
	char* home = g_getenv("HOME");

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[start_pulseaudio]: "
			"Starting pulseaudio");

	args = list_create();
	args->auto_free = 1;

	g_snprintf(home_pulse_dir, 256, "%s/%s", home, ".pulse");
	g_remove_dirs(home_pulse_dir);
	g_mkdir(home_pulse_dir);

	g_snprintf(client_pa_config_path, 256, "%s/%s", home_pulse_dir, "rdpsnd.pa");
	make_pa_from_config(client_pa_config_path);

	list_add_item(args, (tbus)strdup("/usr/bin/pulseaudio"));
	list_add_item(args, (tbus)strdup("-nF"));
	list_add_item(args, (tbus)strdup((const char*)client_pa_config_path));

	pulseaudio_pid = g_launch_process(display_num, args, 1);
	if ( pulseaudio_pid < 0 )
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[start_pulseaudio]: "
				"Failed to fork [%s]",strerror(errno));
		g_exit(-1);
	}
	if ( pulseaudio_pid > 0 )
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[start_pulseaudio]: "
				"Child pid : %i", pulseaudio_pid);
		return 0;
	}
	list_delete(args);
}

int
sndchannel_init()
{
  char filename[256];
  char log_filename[256];
  struct list* names;
  struct list* values;
  char* name;
  char* value;
  int index;
  int res;

  display_num = g_get_display_num_from_display(g_getenv("DISPLAY"));
	if(display_num == 0)
	{
		g_printf( "vchannel_rdpsnd[sndchannel_init]: "
				"Display must be different of 0");
		return ERROR;
	}
	l_config = g_malloc(sizeof(struct log_config), 1);
	l_config->program_name = "vchannel_rdpsnd";
	l_config->log_file = 0;
	l_config->fd = 0;
	l_config->log_level = LOG_LEVEL_DEBUG;
	l_config->enable_syslog = 0;
	l_config->syslog_level = LOG_LEVEL_DEBUG;

  names = list_create();
  names->auto_free = 1;
  values = list_create();
  values->auto_free = 1;
  g_snprintf(filename, 255, "%s/rdpsnd.conf", XRDP_CFG_PATH);
  if (file_by_name_read_section(filename, SND_CFG_GLOBAL, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, SND_CFG_NAME))
      {
        if( g_strlen(value) > 1)
        {
        	l_config->program_name = (char*)g_strdup(value);
        }
      }
    }
  }
  if (file_by_name_read_section(filename, SND_CFG_LOGGING, names, values) == 0)
  {
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, SND_CFG_LOG_DIR))
      {
      	l_config->log_file = (char*)g_strdup(value);
      }
      if (0 == g_strcasecmp(name, SND_CFG_LOG_LEVEL))
      {
      	l_config->log_level = log_text2level(value);
      }
      if (0 == g_strcasecmp(name, SND_CFG_LOG_ENABLE_SYSLOG))
      {
      	l_config->enable_syslog = log_text2bool(value);
      }
      if (0 == g_strcasecmp(name, SND_CFG_LOG_SYSLOG_LEVEL))
      {
      	l_config->syslog_level = log_text2level(value);
      }
    }
  }
  if (file_by_name_read_section(filename, SND_CFG_FORMAT, names, values) == 0)
  {
  	server_format.nChannels = 2;
  	server_format.nSamplesPerSec = 44100;
  	server_format.wBitsPerSample = 16;
    for (index = 0; index < names->count; index++)
    {
      name = (char*)list_get_item(names, index);
      value = (char*)list_get_item(values, index);
      if (0 == g_strcasecmp(name, SND_CFG_NUMBER_CHANNEL))
      {
      	server_format.nChannels = g_atoi(value);
      }
      if (0 == g_strcasecmp(name, SND_CFG_RATE))
      {
      	server_format.nSamplesPerSec = g_atoi(value);
      }
      if (0 == g_strcasecmp(name, SND_CFG_BIT_PER_SAMPLE))
      {
      	server_format.wBitsPerSample = g_atoi(value);
      }
      server_format.nAvgBytesPerSec = 2 * server_format.nSamplesPerSec;
      server_format.nBlockAlign = server_format.wBitsPerSample/4;
    }
  }
  if( g_strlen(l_config->log_file) > 1 && g_strlen(l_config->program_name) > 1)
  {
  	g_snprintf(log_filename, 256, "%s/%i/%s.log",
  			l_config->log_file,	display_num, l_config->program_name);
  	l_config->log_file = (char*)g_strdup(log_filename);
  }
  list_delete(names);
  list_delete(values);
  res = log_start(l_config);

	if( res != LOG_STARTUP_OK)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[sndchannel_init]: "
				"Unable to start log system[%i]", res);
		return res;
	}
  else
  {
  	return LOG_STARTUP_OK;
  }
}

/*****************************************************************************/
int main(int argc, char*argv[]) {
    /* The sample type to use */
		pthread_t vchannel_thread;
		pthread_t sound_thread;
    int ret = 1;

  	l_config = g_malloc(sizeof(struct log_config), 1);
  	if (sndchannel_init() != LOG_STARTUP_OK)
  	{
  		g_printf("vchannel_rdpsnd[main]: Unable to init log system\n");
  		g_free(l_config);
  		return 1;
  	}
  	if (vchannel_init() == ERROR)
  	{
  		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[main]: "
  				"Unable to init channel system\n");
  		g_free(l_config);
  		return 1;
  	}
    init_channel();
    start_pulseaudio();

		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[main]: "
    		"Create virtual channel thread\n");
  	if (pthread_create (&sound_thread, NULL, thread_sound_process, (void*)0) < 0)
  	{
  		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[main]: "
  				"Pthread_create error for thread : spool_thread");
  		return 1;
  	}

    if (pthread_create (&vchannel_thread, NULL, thread_vchannel_process, (void*)0) < 0)
  	{
  		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[main]: "
  				"Pthread_create error for thread : vchannel_thread\n");
  		return 1;
  	}

  	(void)pthread_join (vchannel_thread, (void*)&ret);
  	(void)pthread_join (sound_thread, (void*)&ret);


  	return 0;
}
