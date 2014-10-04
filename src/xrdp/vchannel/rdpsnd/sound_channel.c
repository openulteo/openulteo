/**
 * Copyright (C) 2010 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechavalier <david@ulteo.com>
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

#include <vchannel.h>
#include "sound_channel.h"

static int sndrdp_channel;
static int g_rdpsnd_chan_id = 0;

static int is_fragmented_packet = 0;
static int fragment_size;
static struct stream* splitted_packet = 0;
static RD_WAVEFORMATEX formats[MAX_FORMATS];
static int format_index = 0;
static block_count = 0;
int completion_count=0;
extern int pulseaudio_pid;
extern struct log_config* l_config;
static pthread_cond_t reply_cond;
static pthread_mutex_t mutex;
RD_WAVEFORMATEX server_format;


/*****************************************************************************/
int vchannel_sound_send(struct stream* s, int update_size){
  int rv;
  int size = (int)(s->end - s->data);
  /* set the body size */
  if(update_size == 1)
  {
  	*(s->data+2) = (uint16)size-4;
  }

  rv = vchannel_send(sndrdp_channel, s->data, size);
  if (rv != 0)
  {
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[vchannel_sound_send]: "
    		"Unable to send message");
  }
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_send]: "
    		"Send message of size : %i",size);
  return rv;
}

/*****************************************************************************/
void APP_CC
vchannel_sound_wait_reply()
{
	completion_count++;
	if (completion_count < MAX_SOUND_SENDED )
	{
		return;
	}
  if (pthread_cond_wait(&reply_cond, &mutex) != 0) {
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[vchannel_sound_wait_reply]: "
    "pthread_cond_timedwait() error [%s]", strerror(errno));
    return;
  }
}

/*****************************************************************************/
int APP_CC
vchannel_sound_send_training()
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_send_training]: "
			"Send sound training");
	struct stream* s;
	make_stream(s);
	init_stream(s, 1024);
	/* RDPSND PDU Header */
	out_uint8(s, SNDC_TRAINING);
	out_uint8(s, 0);												/* unused */
	out_uint16_le(s, 0);										/* data size: unused */
	/* Body */
	out_uint16_le(s, TRAINING_VALUE);				/* wTimeStamp: arbitrary */
	out_uint16_le(s, 0); 										/* wPackSize */
	s_mark_end(s);
	vchannel_sound_send(s, 1);
	free_stream(s);
}

/*****************************************************************************/
int APP_CC
vchannel_sound_send_wave_info(int timestamp, int len, char* data)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_send_wave_info]: "
			"Send wave information");
	char *p = data;
	struct stream* s;
	make_stream(s);
	init_stream(s, 16);
	/* RDPSND PDU Header */
	out_uint8(s, SNDC_WAVE);

	out_uint8(s, 0);												/* padding */
	out_uint16_le(s, len+12);								/* data size*/
	/* Body */
	out_uint16_le(s, timestamp);						/* wTimeStamp */
	out_uint16_le(s, 0); 										/* wFormatNo */
	out_uint8(s, block_count);							/* cBlockNo */
	block_count++;
	out_uint8s(s,3);		 										/* bPad */
	out_uint8a(s, p, 4); 										/* first 4 data bytes */

	s_mark_end(s);
	vchannel_sound_send(s, 0);
	free_stream(s);

	make_stream(s);
	init_stream(s, len + 4);

	//p+=4;
	out_uint8s(s,4);		 										/* bPad */
	out_uint8a(s, p, len);

	s_mark_end(s);
	vchannel_sound_send(s, 0);
	free_stream(s);
	//vchannel_sound_wait_reply();

}

/*****************************************************************************/
int APP_CC
vchannel_sound_send_next_wave_info(int len, char* data)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_send_next_wave_info]: "
			"Send wave information");
	char *p = data;
	struct stream* s;

	make_stream(s);
	init_stream(s, len);
	out_uint8a(s, p, len);
	s_mark_end(s);
	vchannel_sound_send(s, 0);
	free_stream(s);

}

/*****************************************************************************/
int APP_CC
vchannel_sound_send_format_and_version(void)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[main]: "
			"Send available server format and version");
	struct stream* s;
	make_stream(s);
	init_stream(s, 1024);
	/* RDPSND PDU Header */
	out_uint8(s, SNDC_FORMATS);
	out_uint8(s, 0);												/* unused */
	out_uint16_le(s, 0);										/* data size: unused */
	/* Body */
	out_uint32_le(s, 0); 										/* dwFlags: unused */
	out_uint32_le(s, 0); 										/* dwVolume: unused */
	out_uint32_le(s, 0); 										/* dwPitch: unused */
	out_uint16_le(s, 0); 										/* wDGramPort: unused */
	out_uint16_le(s, 1);									 	/* wNumberOfFormats */
	out_uint8(s, 0);												/* cLastBlockConfirmed */
	out_uint16_le(s, RDP51); 								/* wVersion */
	out_uint8(s, 0);												/* bPad: unused */
	/* sound formats table */
	/* format 1 */
	out_uint16_le(s, WAVE_FORMAT_PCM); 			          /* wFormatTag */
	out_uint16_le(s, server_format.nChannels);        /* nChannels */
	out_uint32_le(s, server_format.nSamplesPerSec); 	/* nSamplesPerSec */
	out_uint32_le(s, server_format.nAvgBytesPerSec); 	/* nAvgBytesPerSec */
	out_uint16_le(s, server_format.nBlockAlign);		 	/* nBlockAlign */
	out_uint16_le(s, server_format.wBitsPerSample);		/* wBitsPerSample */
	out_uint16_le(s, 0);		 								          /* cbSize */
	//out_uint16_le(s, RDP51); 								        /* data */
	s_mark_end(s);
	vchannel_sound_send(s, 1);
	free_stream(s);
}


/*****************************************************************************/
int APP_CC
vchannel_sound_process_client_format(struct stream* s)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
			"Register client format");
	RD_WAVEFORMATEX *format;
	int dwFlags;
	int dwVolume;
	int dwPitch;
	int port;
	int format_number_announced;
	int unused;
	int version;
	int wDGramPort;
	int i;

	in_uint32_le(s, dwFlags);
	in_uint32_le(s, dwVolume);
	in_uint32_le(s, dwPitch);
	in_uint16_be(s, wDGramPort);
	in_uint16_le(s, format_number_announced);
	in_uint8(s, unused);
	in_uint16_le(s, version);
	in_uint8(s, unused);

	if (dwFlags & TSSNDCAPS_ALIVE == 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
				"The client did not to use audio channel");
	}
	if (dwFlags & TSSNDCAPS_VOLUME == 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
				"The client can not control volume");
		dwVolume = 0;
	}
	if (dwFlags & TSSNDCAPS_PITCH == 0)
	{
		log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
				"The client can not control pitch");
		/* TODO interprete the pitch value */
		dwPitch = 0;
	}
	/* sound format list */
	for ( i=0; i<format_number_announced ; i++)
	{
		format = &formats[i];
		in_uint16_le(s, format->wFormatTag); 					/* wFormatTag */
		in_uint16_le(s, format->nChannels); 					/* nChannels */
		in_uint32_le(s, format->nSamplesPerSec); 		/* nSamplesPerSec */
		in_uint32_le(s, format->nAvgBytesPerSec); 		/* nAvgBytesPerSec */
		in_uint16_le(s, format->nBlockAlign);				/* nBlockAlign */
		in_uint16_le(s, format->wBitsPerSample);			/* wBitsPerSample */
		in_uint16_le(s, format->cbSize);		 					/* cbSize */
		if(format->cbSize != 0)
		{
			in_uint8a(s, format->cb, format->cbSize);
		}
	}
	if( wDGramPort == 0)
	{
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
				"Client do not use UDP transport layer");
		vchannel_sound_send_training();
	}
	else
	{
		/* if datagram is present, add Quality Mode PDU */
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[vchannel_sound_process_client_format]: "
				"UDP transfert is not supported");
	}
	return 0;
}

/*****************************************************************************/
int APP_CC
vchannel_sound_process_training_message(struct stream* s)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_training_message]: ");
	int training_value;
	in_uint16_le(s, training_value);

	if (training_value != TRAINING_VALUE)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[vchannel_sound_process_training_message]: "
				"The training value returned is invalid : %02x\n",training_value);
	}
	return 0;
}



/*****************************************************************************/
int APP_CC
vchannel_sound_process_message(struct stream* s, int length, int total_length)
{
  int msg_type;
  int result;
  struct stream* packet;
  int unused;
  int msg_size;

  if(length != total_length)
  {
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
  			"Packet is fragmented");
  	if(is_fragmented_packet == 0)
  	{
  		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
  				"packet is fragmented : first part");
  		is_fragmented_packet = 1;
  		fragment_size = length;
  		make_stream(splitted_packet);
  		init_stream(splitted_packet, total_length);
  		g_memcpy(splitted_packet->p,s->p, length );
  		return 0;
  	}
  	else
  	{
  		g_memcpy(splitted_packet->p+fragment_size, s->p, length );
  		fragment_size += length;
  		if (fragment_size == total_length )
  		{
    		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
    				"Packet is fragmented : last part");
  			packet = splitted_packet;
  		}
  		else
  		{
    		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
    				"Packet is fragmented : next part");
  			return 0;
  		}
  	}
  }
  else
  {
  	packet = s;
  }
  in_uint8(packet, msg_type);
  in_uint8(packet, unused);
  in_uint16_le(s, msg_size);

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
  		"Msg_type=0x%01x", msg_type);
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
	  		"msg_size=%i", msg_size);

	switch (msg_type)
	{
	case SNDC_FORMATS:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
	  		"SNDC_FORMATS message");
		result = vchannel_sound_process_client_format(s);
		break;

	case SNDC_TRAINING:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
	  		"SNDC_TRANING message");
		result = vchannel_sound_process_training_message(s);
		break;

	case SNDC_WAVECONFIRM:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
  			"SNDC_WAVECONFIRM");
  	completion_count--;
  	pthread_cond_signal(&reply_cond);

		break;

	default:
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_process_message]: "
				"unknown message %02x", msg_type);
		result = 1;
	}
	if(is_fragmented_packet == 1)
	{
		is_fragmented_packet = 0;
		fragment_size = 0;
		if(splitted_packet != 0)
		{
			free_stream(splitted_packet);
		}
		splitted_packet = 0;
	}

  return result;
}


/*****************************************************************************/
int APP_CC
vchannel_sound_init(void)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_init]: "
			"Init sound channel");
	g_rdpsnd_chan_id = 1;
	vchannel_sound_send_format_and_version();
	pthread_cond_init(&reply_cond, NULL);
	pthread_mutex_init(&mutex, NULL);

  return 0;
}

/*****************************************************************************/
int APP_CC
vchannel_sound_deinit(void)
{
	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[vchannel_sound_deinit]: ");
	pthread_cond_destroy(&reply_cond);
	pthread_mutex_destroy(&mutex);

	g_sigterm(pulseaudio_pid);
	g_exit(0);
	return 0;
}

/*****************************************************************************/
void *thread_vchannel_process (void * arg)
{
	struct stream* s = NULL;
	int rv;
	int length;
	int total_length;

	log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_vchannel_process]: "
			"Init vchannel main loop thread");
	vchannel_sound_init();

	while(1){
		make_stream(s);
		init_stream(s, 1600);
		log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_vchannel_process]: "
				"Prepare to receive ");

		rv = vchannel_receive(sndrdp_channel, s->data, &length, &total_length);
		switch(rv)
		{
		case ERROR:
			log_message(l_config, LOG_LEVEL_WARNING, "vchannel_rdpsnd[thread_vchannel_process]: "
					"Invalid message");
			free_stream(s);
			vchannel_close(sndrdp_channel);
			pthread_exit ((void*) 1);
			break;
		case STATUS_CONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_vchannel_process]: "
					"Status connected");
			break;
		case STATUS_DISCONNECTED:
			log_message(l_config, LOG_LEVEL_DEBUG, "vchannel_rdpsnd[thread_vchannel_process]: "
					"Status disconnected");
			vchannel_sound_deinit();
			break;
		default:
			if (length == 0)
			{
				pthread_exit (0);
			}
			s->data[length]=0;
			vchannel_sound_process_message(s, length, total_length);
			break;
		}

		free_stream(s);
	}
	pthread_exit (0);
}


/*****************************************************************************/
int init_channel()
{
	pthread_t Vchannel_thread;
	void *ret;

	if (vchannel_init() == ERROR)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[init_channel]: "
				"Unable to init channel system");
		return 1;
	}

	sndrdp_channel = vchannel_open("rdpsnd");
	if( sndrdp_channel == ERROR)
	{
		log_message(l_config, LOG_LEVEL_ERROR, "vchannel_rdpsnd[init_channel]: "
				"Error while connecting to vchannel provider");
		return 1;
	}
}

