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

#ifndef SOUND_CHANNEL_H_
#define SOUND_CHANNEL_H_
#include <parse.h>
#include <arch.h>

void* thread_vchannel_process (void * arg);
int init_channel();

#define ERROR -1

/* config constant */
#define SND_CFG_GLOBAL						"Globals"
#define SND_CFG_NAME							"Name"
#define SND_CFG_LOGGING						"Logging"
#define SND_CFG_LOG_DIR						"LogDir"
#define SND_CFG_LOG_LEVEL					"LogLevel"
#define SND_CFG_LOG_ENABLE_SYSLOG	"EnableSyslog"
#define SND_CFG_LOG_SYSLOG_LEVEL	"SyslogLevel"
#define SND_CFG_FORMAT						"SoundFormat"
#define SND_CFG_NUMBER_CHANNEL		"NumberOfChannel"
#define SND_CFG_BIT_PER_SAMPLE		"BitsPerSample"
#define SND_CFG_RATE							"Rate"

#define MAX_SOUND_SENDED      5
#define MAX_BLOCK_SIZE        16000

#define SOUND_FORMAT_NUMBER		3
/* Msg type */
#define SNDC_CLOSE						0x01
#define SNDC_WAVE							0x02
#define SNDC_SETVOLUME				0x03
#define SNDC_SETPITCH					0x04
#define SNDC_WAVECONFIRM			0x05
#define SNDC_TRAINING					0x06
#define SNDC_FORMATS					0x07
#define SNDC_CRYPTKEY					0x08
#define SNDC_WAVEENCRYPT			0x09
#define SNDC_UDPWAVE					0x0A
#define SNDC_UDPWAVELAST			0x0B
#define SNDC_QUALITYMODE			0x0C


/* sound version */
#define	RDP5						0x02
#define	RDP51						0x05
#define	RDP7						0x06

/* sound format */
#define WAVE_FORMAT_PCM      0x0001
#define WAVE_FORMAT_ADPC     0x0002
#define WAVE_FORMAT_ALAW     0x0006
#define WAVE_FORMAT_MSG723   0x0042
#define WAVE_FORMAT_GSM610   0x0031
#define WAVE_FORMAT_MULAW    0x0007


/* sound possible control */
#define TSSNDCAPS_ALIVE		0x00000001
#define TSSNDCAPS_VOLUME	0x00000002
#define TSSNDCAPS_PITCH		0x00000004



#define TRAINING_VALUE		0x99




#define MAX_FORMATS			10
#define MAX_CBSIZE 256


typedef struct _RD_WAVEFORMATEX
{
	uint16 wFormatTag;
	uint16 nChannels;
	uint32 nSamplesPerSec;
	uint32 nAvgBytesPerSec;
	uint16 nBlockAlign;
	uint16 wBitsPerSample;
	uint16 cbSize;
	uint8 cb[MAX_CBSIZE];
} RD_WAVEFORMATEX;


#endif /* SOUND_CHANNEL_H_ */
