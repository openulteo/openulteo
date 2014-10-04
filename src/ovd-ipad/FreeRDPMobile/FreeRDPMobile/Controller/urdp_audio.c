/*
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Clément Bizeau <cbizeau@ulteo.com> 2011
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <freerdp/types.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/dsp.h>
#include <freerdp/utils/svc_plugin.h>

#include <rdpsnd_main.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES_Platform.h>

#include "log.h"

#define SLES_OPENED 1
#define SLES_CLOSED 0
#define BUFFERQUEUE_FULL_WAITINGTIME 100
#define BEFORECLOSE_WAITINGTIME 1

//necessary data for sles
typedef struct rdpsnd_sles_plugin rdpsndSlesPlugin;
struct rdpsnd_sles_plugin {
	rdpsndDevicePlugin device;

	SLObjectItf engineObjectItf;
	SLEngineItf engineItf;
	SLObjectItf  playerObjectItf;
	SLPlayItf playItf;
	SLObjectItf outputmixObjectItf;
	SLOutputMixItf outputmixItf;
	SLDataSource audioSource;
	SLDataFormat_PCM format;
	SLDataSink sink;
	SLDataLocator_AndroidSimpleBufferQueue bufferQueue;
	SLAndroidSimpleBufferQueueItf bufferQueueItf;
	SLDataLocator_OutputMix outputmixLoc;
	ADPCM adpcm;
	char *data_play;
	char *data_next;
	int next_size;
	int data_max;
	pthread_mutex_t mutex;
	short opened;
	short playing;
	int adpcmBlockAlign;
	int formatTag;
};

//check error state of sles function
static int checkError(int errorCode, char *msg) {
	log_debug(msg);

	if (errorCode != SL_RESULT_SUCCESS)
		log_error(msg);

	switch (errorCode) {
		case SL_RESULT_SUCCESS:
			log_debug("SL_SUCESS");
			break;
		case SL_RESULT_PRECONDITIONS_VIOLATED:
			log_error("SL_RESULT_PRECONDITIONS_VIOLATED");
			break;
		case SL_RESULT_PARAMETER_INVALID:
			log_error("SL_RESULT_PARAMETER_INVALID");
			break;
		case SL_RESULT_MEMORY_FAILURE:
			log_error("SL_RESULT_MEMORY_FAILURE");
			break;
		case SL_RESULT_RESOURCE_ERROR:
			log_error("SL_RESULT_RESOURCE_ERROR");
			break;
		case SL_RESULT_RESOURCE_LOST:
			log_error("SL_RESULT_RESOURCE_LOST");
			break;
		case SL_RESULT_IO_ERROR:
			log_error("SL_RESULT_IO_ERROR");
			break;
		case SL_RESULT_BUFFER_INSUFFICIENT:
			log_error("SL_RESULT_BUFFER_INSUFFICIENT");
			break;
		case SL_RESULT_CONTENT_CORRUPTED:
			log_error("SL_RESULT_CONTENT_CORRUPTED");
			break;
		case SL_RESULT_CONTENT_UNSUPPORTED:
			log_error("SL_RESULT_CONTENT_UNSUPPORTED");
			break;
		case SL_RESULT_CONTENT_NOT_FOUND:
			log_error("SL_RESULT_CONTENT_NOT_FOUND");
			break;
		case SL_RESULT_PERMISSION_DENIED:
			log_error("SL_RESULT_PERMISSION_DENIED");
			break;
		case SL_RESULT_FEATURE_UNSUPPORTED:
			log_error("SL_RESULT_FEATURE_UNSUPPORTED");
			break;
		case SL_RESULT_INTERNAL_ERROR:
			log_error("SL_RESULT_INTERNAL_ERROR");
			break;
		case SL_RESULT_UNKNOWN_ERROR:
			log_error("SL_RESULT_UNKNOWN_ERROR");
			break;
		case SL_RESULT_OPERATION_ABORTED:
			log_error("SL_RESULT_OPERATION_ABORTED");
			break;
		case SL_RESULT_CONTROL_LOST:
			log_error("SL_RESULT_CONTROL_LOST");
			break;
		default:
			log_error("unknown error");
			break;
	}
	return errorCode == SL_RESULT_SUCCESS;
}

void swap_streams_and_play(rdpsndSlesPlugin *sles) {
	SLresult result;
	char* bkp;
	if (sles->next_size > 0) {
		bkp = sles->data_play;
		sles->data_play = sles->data_next;
		sles->data_next = bkp;
		sles->playing = 1;
		for(;;)
		{
			//enqueue data in the sles buffer queue
			result = (*sles->bufferQueueItf)->Enqueue(sles->bufferQueueItf, sles->data_play, sles->next_size);
			//wait if sles buffer queue is full
			if(result == SL_RESULT_BUFFER_INSUFFICIENT) {
				log_debug("buffer queue is full");
				usleep(BUFFERQUEUE_FULL_WAITINGTIME);
			} else
				break;
		}
		sles->next_size = 0;
	} else {
		sles->playing = 0;
	}
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
	rdpsndSlesPlugin *sles = (rdpsndSlesPlugin *)context;

	log_debug("free buffer played buffer");
	pthread_mutex_lock(&sles->mutex);
	swap_streams_and_play(sles);
	pthread_mutex_unlock(&sles->mutex);
}

static void rdpsnd_sles_close(rdpsndDevicePlugin* device) {
	SLresult result;
	rdpsndSlesPlugin* sles = (rdpsndSlesPlugin*)device;

	log_debug("rdpsnd_sles_close");

	if(!sles->opened)
		return;

	//wait before stopping sound playing because of latency
	sleep(BEFORECLOSE_WAITINGTIME);

	//pause the player
	result = (*sles->playItf)->SetPlayState(sles->playItf, SL_PLAYSTATE_PAUSED);
	if (!checkError(result, "set playstate stop"))
		return;

	//player is not open anymore
	sles->opened = SLES_CLOSED;
	sles->playing = 0;

	//clear the buffer queue of sles
	result = (*sles->bufferQueueItf)->Clear(sles->bufferQueueItf);
	if (!checkError(result, "clear buffer"))
		return;

	//free audio player and all interface associated
	if(sles->playerObjectItf != NULL) {
		(*sles->playerObjectItf)->Destroy(sles->playerObjectItf);
		sles->playerObjectItf = NULL;
		sles->playItf = NULL;
		sles->bufferQueueItf = NULL;
	}
}

//set the sles sound format to this one
static void rdpsnd_sles_set_format(rdpsndDevicePlugin* device, rdpsndFormat* format, int latency) {
	SLresult result;

	rdpsnd_sles_close(device);

	log_debug("rdpsnd_sles_set_format: wFormatTag=0x%X nChannels=%d nSamplesPerSec=%d wBitsPerSample=%d cbSize=%d", format->wFormatTag, format->nChannels, format->nSamplesPerSec, format->wBitsPerSample, format->cbSize);

	rdpsndSlesPlugin* sles = (rdpsndSlesPlugin*)device;
	sles->formatTag = format->wFormatTag;

	switch (format->wFormatTag) {
		case 0x11: /* IMA ADPCM */
			format->wBitsPerSample = 16;
			sles->adpcmBlockAlign = format->nBlockAlign;
		case 0x1: /* PCM */
			sles->format.formatType = SL_DATAFORMAT_PCM;
			switch (format->wBitsPerSample) {
				case 8:
					sles->format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
					sles->format.containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
					break;
				case 16:
					sles->format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
					sles->format.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
					break;
			}
			sles->format.numChannels = (SLuint32) format->nChannels;
			sles->format.endianness = SL_BYTEORDER_LITTLEENDIAN;
			sles->format.samplesPerSec = (SLuint32) format->nSamplesPerSec*1000;
			if (format->nChannels == 2)
				sles->format.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
			else if (format->nChannels == 1)
				sles->format.channelMask = SL_SPEAKER_FRONT_CENTER;
			break;
	}

	//set audio format properties
	sles->audioSource.pFormat = &sles->format;
	sles->audioSource.pLocator = &sles->bufferQueue;

	//set sink to output
	sles->sink.pLocator = (void *)&sles->outputmixLoc;
	sles->sink.pFormat = NULL;

	//set required interface for audio player
	const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};

	//creation of audio player
	result = (*sles->engineItf)->CreateAudioPlayer(sles->engineItf, &sles->playerObjectItf, &sles->audioSource, &sles->sink, 1, ids, req);
	if (!checkError(result, "create audio player"))
		return;

	result = (*sles->playerObjectItf)->Realize(sles->playerObjectItf, SL_BOOLEAN_FALSE);
	if (!checkError(result, "realize audio player"))
		return;

	result = (*sles->playerObjectItf)->GetInterface(sles->playerObjectItf, SL_IID_PLAY, (void *)&sles->playItf);
	if (!checkError(result, "get interface player"))
		return;

	//get buffer queue of the player
	result = (*sles->playerObjectItf)->GetInterface(sles->playerObjectItf, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, (void *)&sles->bufferQueueItf);
	if (!checkError(result, "get interface buffer queue"))
		return;

	//set callback for buffer queue
	result = (*sles->bufferQueueItf)->RegisterCallback(sles->bufferQueueItf, bqPlayerCallback, sles);
	log_debug("open success");
	
	//set player state to play
	result = (*sles->playItf)->SetPlayState(sles->playItf, SL_PLAYSTATE_PLAYING);
	if (!checkError(result, "set play state play"))
		return;
}

static void rdpsnd_sles_open(rdpsndDevicePlugin* device, rdpsndFormat* format, int latency) {
	rdpsndSlesPlugin *sles = (rdpsndSlesPlugin *)device;

	log_debug("rdpsnd_sles_open");

	rdpsnd_sles_set_format(device, format, latency);

	//sles is opened
	sles->opened = SLES_OPENED;
}

static void rdpsnd_sles_free(rdpsndDevicePlugin* device) {
	rdpsndSlesPlugin* sles = (rdpsndSlesPlugin*)device;

	log_debug("rdpsnd_sles_free");

	//free outmix object
	if(sles->outputmixObjectItf != NULL) {
		(*sles->outputmixObjectItf)->Destroy(sles->outputmixObjectItf);
		sles->outputmixObjectItf = NULL;
	}

	//free engine and all interface associated
	if(sles->engineObjectItf != NULL) {
		(*sles->engineObjectItf)->Destroy(sles->engineObjectItf);
		sles->engineObjectItf = NULL;
		sles->engineItf = NULL;
	}

	pthread_mutex_destroy(&sles->mutex);

	if (sles->data_next)
		free(sles->data_next);
	if (sles->data_play)
		free(sles->data_play);

	free(sles);
}

static boolean rdpsnd_sles_format_supported(rdpsndDevicePlugin* device, rdpsndFormat* format) {
	//get format supported by the server
	log_debug("rdpsnd_sles_format_supported: wFormatTag=0x%X nChannels=%d nSamplesPerSec=%d wBitsPerSample=%d cbSize=%d", format->wFormatTag, format->nChannels, format->nSamplesPerSec, format->wBitsPerSample, format->cbSize);

	//android sles supports only pcm
	switch (format->wFormatTag) {
		case 0x01: /* PCM */
			if (format->cbSize == 0 && format->nSamplesPerSec <= 48000 && (format->wBitsPerSample == 8 || format->wBitsPerSample == 16) && (format->nChannels == 1 || format->nChannels == 2)) {
				log_debug("rdpsnd_sles_format_supported : pcm supported");
				return true;
			}
			break;
		case 0x11: /* IMA ADPCM */
			if (format->nSamplesPerSec <= 48000 && format->wBitsPerSample == 4 && (format->nChannels == 1 || format->nChannels == 2)) {
				log_debug("rdpsnd_sles_format_supported : ima adpcm supported");
				return true;
			}
			break;
	}
	return false;
}

static void rdpsnd_sles_set_volume(rdpsndDevicePlugin* device, uint32 value) {
	log_debug("rdpsnd_sles_set_volume: %8.8x", value);
}

static void rdpsnd_sles_play(rdpsndDevicePlugin* device, uint8* data, int size) {
	log_debug("rdpsnd_sles_play : data = %X - size = %d", (unsigned int)data, size);

	rdpsndSlesPlugin* sles = (rdpsndSlesPlugin*)device;

	if(!sles->bufferQueueItf)
		return;

	if(!data)
		return;

	pthread_mutex_lock(&sles->mutex);

	if (sles->formatTag == 0x11) {
		int decoded_size;
		//log_debug("rdpsnd_sles_play : decode ImaAdpcm");
		uint8* decoded_data = dsp_decode_ima_adpcm(&sles->adpcm, (uint8 *) data, size, sles->format.numChannels, sles->adpcmBlockAlign, &decoded_size);
		sles->next_size = decoded_size;
		if (decoded_size > sles->data_max) {
			log_debug("rdpsnd_sles_play : realloc %d => %d", sles->data_max, decoded_size);
			sles->data_max = decoded_size;
			sles->data_next = realloc(sles->data_next, decoded_size);
			sles->data_play = realloc(sles->data_play, decoded_size);
		}
		memcpy(sles->data_next, decoded_data, decoded_size);
		free(decoded_data);
	} else {
		if (size > sles->data_max) {
			log_debug("rdpsnd_sles_play : realloc %d => %d", sles->data_max, size);
			sles->data_max = size;
			sles->data_next = realloc(sles->data_next, size);
			sles->data_play = realloc(sles->data_play, size);
		}
		memcpy(sles->data_next, data, size);
		sles->next_size = size;
	}
	if (!sles->playing) {
		swap_streams_and_play(sles);
	}
	pthread_mutex_unlock(&sles->mutex);
}

static void rdpsnd_sles_start(rdpsndDevicePlugin* device) {
	log_debug("rdpsnd_sles_start, called just before close ???");
}

int FreeRDPRdpsndDeviceEntry(PFREERDP_RDPSND_DEVICE_ENTRY_POINTS pEntryPoints) {
	rdpsndSlesPlugin* sles;

	sles = xnew(rdpsndSlesPlugin);

	//set callbacks
	sles->device.Open = rdpsnd_sles_open;
	sles->device.FormatSupported = rdpsnd_sles_format_supported;
	sles->device.SetFormat = rdpsnd_sles_set_format;
	sles->device.SetVolume = rdpsnd_sles_set_volume;
	sles->device.Play = rdpsnd_sles_play;
	sles->device.Start = rdpsnd_sles_start;
	sles->device.Close = rdpsnd_sles_close;
	sles->device.Free = rdpsnd_sles_free;

	pthread_mutex_init(&sles->mutex, NULL);

	SLresult result;
	//create sles engine to access all sles functions
	result = slCreateEngine(&sles->engineObjectItf, 0, NULL, 0, NULL, NULL);
	if (!checkError(result, "slCreateEngine")) return 1;
	result =(*sles->engineObjectItf)->Realize(sles->engineObjectItf, SL_BOOLEAN_FALSE);
	if (!checkError(result, "realize engine")) return 1;
	result = (*sles->engineObjectItf)->GetInterface(sles->engineObjectItf, SL_IID_ENGINE, &sles->engineItf);
	if (!checkError(result, "get interface engine")) return 1;
	//create outputmix to play on output of the device
	result = (*sles->engineItf)->CreateOutputMix(sles->engineItf, &sles->outputmixObjectItf, 0, NULL, NULL);
	if (!checkError(result, "create output mix")) return 1;
	result = (*sles->outputmixObjectItf)->Realize(sles->outputmixObjectItf, SL_BOOLEAN_FALSE);
	if (!checkError(result, "realize output mix")) return 1;
	//set output property to outpoutmix
	sles->outputmixLoc.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	sles->outputmixLoc.outputMix = sles->outputmixObjectItf;
	//set input property to bufferqueue
	sles->bufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
	sles->bufferQueue.numBuffers = (SLuint32)128;

	//register sles rdpsnd device
	pEntryPoints->pRegisterRdpsndDevice(pEntryPoints->rdpsnd, (rdpsndDevicePlugin*)sles);

	return 0;
}

