/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
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
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <config.h>
#include <log.h>

#include <freerdp/types.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/dsp.h>
#include <freerdp/utils/svc_plugin.h>

#include "../../../FreeRDP/FreeRDP/FreeRDP/channels/rdpsnd/rdpsnd_main.h"

//#define kFormatPCM8    1
//#define kFormatPCM16   2
//#define kFormatPCM16LE 3

#define BUFFER_SIZE 32768
//#define BUFFER_COUNT 1

typedef struct rdpsnd_ios_plugin rdpsndIosPlugin;
struct rdpsnd_ios_plugin {
	rdpsndDevicePlugin devplugin;
	ADPCM adpcm;
	
	pthread_mutex_t mutex;
	
  AudioFileStreamID stream;
	AudioQueueRef audioQueue;
	AudioQueueBufferRef buffer; //s[BUFFER_COUNT];

  int wformat;
	int adpcmBlockAlign;
	int numChannels;
	
	char *data_play;
	char *data_next;
	int next_size;
	int data_max;
	
	boolean queueStarted;
};


void printErrorMessage(OSStatus code) {
  switch(code) {
    case kAudioFileStreamError_UnsupportedFileType      : log_warning("kAudioFileStreamError_UnsupportedFileType"); break;
    case kAudioFileStreamError_UnsupportedDataFormat    : log_warning("kAudioFileStreamError_UnsupportedDataFormat"); break;
    case kAudioFileStreamError_UnsupportedProperty      : log_warning("kAudioFileStreamError_UnsupportedProperty"); break;
    case kAudioFileStreamError_BadPropertySize          : log_warning("kAudioFileStreamError_BadPropertySize"); break;
    case kAudioFileStreamError_NotOptimized             : log_warning("kAudioFileStreamError_NotOptimized"); break;
    case kAudioFileStreamError_InvalidPacketOffset      : log_warning("kAudioFileStreamError_InvalidPacketOffset"); break;
    case kAudioFileStreamError_InvalidFile              : log_warning("kAudioFileStreamError_InvalidFile"); break;
    case kAudioFileStreamError_ValueUnknown             : log_warning("kAudioFileStreamError_ValueUnknown"); break;
    case kAudioFileStreamError_DataUnavailable          : log_warning("kAudioFileStreamError_DataUnavailable"); break;
    case kAudioFileStreamError_IllegalOperation         : log_warning("kAudioFileStreamError_IllegalOperation"); break;
    case kAudioFileStreamError_UnspecifiedError         : log_warning("kAudioFileStreamError_UnspecifiedError"); break;
    case kAudioFileStreamError_DiscontinuityCantRecover : log_warning("kAudioFileStreamError_DiscontinuityCantRecover"); break;
      //    default: DDLogCVerbose(@"no error");
  }
}


void swap_streams_and_play(rdpsndIosPlugin* iosplugin, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
  OSStatus err;

	//log_verbose("AudioQueueCallback");

	char* bkp;
	if (iosplugin->next_size > 0) {
		bkp = iosplugin->data_play;
		iosplugin->data_play = iosplugin->data_next;
		iosplugin->data_next = bkp;
		iosplugin->queueStarted = true;
		//enqueue data in the sles buffer queue
		
		if (iosplugin->next_size > inBuffer->mAudioDataBytesCapacity)
			iosplugin->next_size = inBuffer->mAudioDataBytesCapacity;
		
		memcpy(inBuffer->mAudioData, iosplugin->data_play, iosplugin->next_size);
		
		inBuffer->mAudioDataByteSize = iosplugin->next_size;
		err = AudioQueueEnqueueBuffer(iosplugin->audioQueue, inBuffer, 0, NULL);
		printErrorMessage(err);
		
		iosplugin->next_size = 0;
	} else {
		iosplugin->queueStarted = false;
	}
}


static void AudioQueueCallback(void* inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
	rdpsndIosPlugin* iosplugin = (rdpsndIosPlugin*)inUserData;
	pthread_mutex_lock(&iosplugin->mutex);
	swap_streams_and_play(iosplugin, inAQ, inBuffer);
	pthread_mutex_unlock(&iosplugin->mutex);
}
		
/*
void MyAudioQueueIsRunningCallback(void *inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID) {
  //    DDLogCVerbose(@"audio queue property listener callback");
  if (inID == kAudioQueueProperty_IsRunning) {
    //        DDLogCVerbose(@"audio queue is running");
  }
}


void PropertyListener(void *inClientData,
                      AudioFileStreamID inAudioFileStream,
                      AudioFileStreamPropertyID inPropertyID,
                      UInt32 *ioFlags) {
  log_verbose("PropertyListener");
}


void PacketsProc(void *inClientData,
                 UInt32 inNumberBytes,
                 UInt32 inNumberPackets,
                 const void *inInputData,
                 AudioStreamPacketDescription *inPacketDescriptions) {
  log_verbose("PacketsProc");
}*/


// returns boolean
static boolean rdpsnd_ios_format_supported(rdpsndDevicePlugin * devplugin, rdpsndFormat* format) {
  //log_verbose("rdpsnd_ios_format_supported");
  //    SoundInfo *soundInfo;
  
  //    soundInfo = (SoundInfo *)devplugin->device_data;
  
	log_debug("rdpsnd_ios_format_supported: wFormatTag=%d "
						"nChannels=%d nSamplesPerSec=%d wBitsPerSample=%d cbSize=%d",
						format->wFormatTag, format->nChannels, format->nSamplesPerSec, format->wBitsPerSample, format->cbSize);
  
  //    printf("rdpsnd_ios_format_supported: wFormatTag=%d "
  //                "nChannels=%d nSamplesPerSec=%d wBitsPerSample=%d cbSize=%d\n",
  //           wFormatTag, nChannels, nSamplesPerSec, wBitsPerSample, cbSize);
	switch (format->wFormatTag)	{
		case 1: // PCM 
      //			if (cbSize == 0 &&
      //				(nSamplesPerSec <= 48000) &&
      //				(wBitsPerSample == 8 || wBitsPerSample == 16) &&
      //				(nChannels == 1 || nChannels == 2))
      if (format->cbSize == 0 &&
          (format->nSamplesPerSec <= 48000) &&
          (format->wBitsPerSample == 8 || format->wBitsPerSample == 16) &&
					(format->nChannels == 1 || format->nChannels == 2))	{
				log_verbose("rdpsnd_ios_format_supported: PCM supported");
				return true;
			}
			break;
      
		case 0x11: // IMA ADPCM 
			if ((format->nSamplesPerSec <= 48000) &&
          (format->wBitsPerSample == 4) &&
          (format->nChannels == 1 || format->nChannels == 2)) {
				log_verbose("rdpsnd_ios_format_supported: IMA ADPCM supported");
        //				DDLogCVerbose(@"rdpsnd_ios_format_supported: ok");
				return true; //FIXME
			}
			break;
	}
	return false;
}


static void rdpsnd_ios_close(rdpsndDevicePlugin * devplugin) {
	rdpsndIosPlugin *soundInfo = (rdpsndIosPlugin *)devplugin;
  log_verbose("Dispose queue");
	if (soundInfo->audioQueue == NULL)
		return;
	
  OSStatus ret = AudioQueueDispose(soundInfo->audioQueue, NO);
  printErrorMessage(ret);
	soundInfo->audioQueue = NULL;
  soundInfo->queueStarted = false;
}


static void rdpsnd_ios_set_format(rdpsndDevicePlugin * devplugin, rdpsndFormat* format, int latency) {
  OSStatus err = noErr;
	rdpsndIosPlugin *soundInfo = (rdpsndIosPlugin *)devplugin;
	AudioStreamBasicDescription deviceFormat;
	
	rdpsnd_ios_close(devplugin);
  
	log_debug("rdpsnd_ios_set_format: wFormatTag=%d nChannels=%d "
						"nSamplesPerSec=%d wBitsPerSample=%d nBlockAlign=%d",
						format->wFormatTag, format->nChannels, format->nSamplesPerSec, format->wBitsPerSample, format->nBlockAlign);
  
	soundInfo->wformat = format->wFormatTag;
	soundInfo->numChannels = format->nChannels;
	switch (format->wFormatTag) {
		case 0x11: /* IMA ADPCM */
			format->wBitsPerSample = 16;
			soundInfo->adpcmBlockAlign = format->nBlockAlign;
			break;
	}
	deviceFormat.mFormatID = kAudioFormatLinearPCM;
  deviceFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
  deviceFormat.mSampleRate = format->nSamplesPerSec;
  deviceFormat.mChannelsPerFrame = format->nChannels;
  deviceFormat.mFramesPerPacket  = 1;
	deviceFormat.mBytesPerPacket   = format->nChannels * (format->wBitsPerSample / 8);
  deviceFormat.mBytesPerFrame    = deviceFormat.mFramesPerPacket * deviceFormat.mBytesPerPacket;
	deviceFormat.mBitsPerChannel = format->wBitsPerSample;
	deviceFormat.mReserved         = 0;
  
  err = AudioQueueNewOutput(&deviceFormat, AudioQueueCallback, (void*)soundInfo, CFRunLoopGetMain(), kCFRunLoopCommonModes, 0, &soundInfo->audioQueue);
  //    DDLogCVerbose(@"AudioQueueNewOutput");
  printErrorMessage(err);
  
  // Allocate buffers for the AudioQueue, and pre-fill them.
//	for (int i = 0; i < BUFFER_COUNT; ++i) {
		err = AudioQueueAllocateBuffer(soundInfo->audioQueue, BUFFER_SIZE, &soundInfo->buffer);
	//AudioQueueCallback(NULL, iosplugin->audioQueue, iosplugin->buffers[i]);
//	}
  //    DDLogCVerbose(@"audioqueueproperty listener");
  //  err = AudioQueueAddPropertyListener(audioQueue, kAudioQueueProperty_IsRunning, &MyAudioQueueIsRunningCallback, NULL);
  //  printErrorMessage(err);
  
  //  if (err == noErr) {
  AudioQueueSetParameter(soundInfo->audioQueue, kAudioQueueParam_Volume, 1.0);
  //  err = AudioQueueStart(audioQueue, NULL);
  //  printErrorMessage(err);
  //  }
  
  //    if (err == noErr) {DDLogCVerbose(@"CFRunLoopRun() before"); CFRunLoopRun(); DDLogCVerbose(@"CFRunLoopRun()");}
  //    printErrorMessage(err);
  //    DDLogCVerbose(@"before run loop");
  //    do {CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);} while(1);
  //    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
  //    DDLogCVerbose(@"after runloop");

  
  //printFormat(soundInfo);
}


static void rdpsnd_ios_start(rdpsndDevicePlugin* device) {
	log_debug("rdpsnd_ios_start, called just before close ???");
}


static void rdpsnd_ios_open(rdpsndDevicePlugin * devplugin, rdpsndFormat* format, int latency)
{
  log_verbose("rdpsnd_ios_open");
	
	rdpsnd_ios_set_format(devplugin, format, latency);
  
  
  
  //  SetupAudioQueue();
  
  
  //    SoundInfo *soundInfo = devplugin->device_data;
  
  //    OSStatus ret = AudioFileStreamOpen(NULL,
  //                                       &PropertyListener,
  //                                       &PacketsProc,
  //                                       kAudioFileWAVEType,
  //                                       &(soundInfo->stream));
  //    printErrorMessage(ret);
  
  //	struct alsa_device_data * alsa_data;
  //	int error;
  //    
  //	alsa_data = (struct alsa_device_data *) devplugin->device_data;
  //    
  //	if (alsa_data->out_handle != 0)
  //	{
  //		return 0;
  //	}
  //	LLOGLN(10, ("rdpsnd_ios_open:"));
  //	error = snd_pcm_open(&alsa_data->out_handle, alsa_data->device_name,
  //                         SND_PCM_STREAM_PLAYBACK, 0);
  //	if (error < 0)
  //	{
  //		LLOGLN(0, ("rdpsnd_ios_open: snd_pcm_open failed"));
  //		return 1;
  //	}
  //	memset(&alsa_data->adpcm, 0, sizeof(rdpsndDspAdpcm));
  //	set_params(alsa_data);
}


static void rdpsnd_ios_free(rdpsndDevicePlugin * devplugin) {
  log_verbose("rdpsnd_ios_free");
	rdpsnd_ios_close(devplugin);
	xfree(devplugin);
}


static void rdpsnd_ios_set_volume(rdpsndDevicePlugin * devplugin, uint32 value) {
  //    printf("rdpsnd_ios_set_volume: %8.8x\n", value);
}


static void rdpsnd_ios_play(rdpsndDevicePlugin * devplugin, uint8 * data, int size) {
  //    int rbytes_per_frame;
	//int sbytes_per_frame;
  rdpsndIosPlugin *soundInfo = (rdpsndIosPlugin *)devplugin;
	
	if(!data)
		return;
  /*
  sbytes_per_frame = soundInfo->source_channels * soundInfo->bytes_per_channel;
  //    rbytes_per_frame = soundInfo->actual_channels * soundInfo->bytes_per_channel;
  if ((size % sbytes_per_frame) != 0) { log_error("ERROR LEN MOD"); return; };
  if ((soundInfo->source_rate == soundInfo->actual_rate) && (soundInfo->source_channels == soundInfo->actual_channels)) {
    //        DDLogCVerbose(@"NO RESAMPLING NEEDED");
  } else {
    //        DDLogCWarn(@"RESAMPLING NEEDED");
  }*/
  
  //    OSStatus ret = AudioFileStreamParseBytes(soundInfo->stream, size, data, 0);
  //    printErrorMessage(ret);
  
  //    [(Sound *)sound playSoundWithData:data size:size];
	
	pthread_mutex_lock(&soundInfo->mutex);
	
	if (soundInfo->wformat == 0x11) {
		int decoded_size;
		//log_debug("rdpsnd_sles_play : decode ImaAdpcm");
		uint8* decoded_data = dsp_decode_ima_adpcm(&soundInfo->adpcm, (uint8 *) data, size, soundInfo->numChannels, soundInfo->adpcmBlockAlign, &decoded_size);
		soundInfo->next_size = decoded_size;
		if (decoded_size > soundInfo->data_max) {
			log_debug("rdpsnd_ios_play : realloc %d => %d", soundInfo->data_max, decoded_size);
			soundInfo->data_max = decoded_size;
			soundInfo->data_next = realloc(soundInfo->data_next, decoded_size);
			soundInfo->data_play = realloc(soundInfo->data_play, decoded_size);
		}
		memcpy(soundInfo->data_next, decoded_data, decoded_size);
		free(decoded_data);
	} else {
	  if (size > soundInfo->data_max) {
			log_debug("rdpsnd_ios_play : realloc %d => %d", soundInfo->data_max, size);
			soundInfo->data_max = size;
			soundInfo->data_next = realloc(soundInfo->data_next, size);
			soundInfo->data_play = realloc(soundInfo->data_play, size);
		}
		memcpy(soundInfo->data_next, data, size);
		soundInfo->next_size = size;
	}
  
	//log_verbose("play queue %d", soundInfo->next_size);
	
	if (!soundInfo->queueStarted) {
		swap_streams_and_play(soundInfo, soundInfo->audioQueue, soundInfo->buffer);
    log_verbose("start queue");
    OSStatus err = AudioQueueStart(soundInfo->audioQueue, NULL);
    printErrorMessage(err);
    //memset(soundInfo->rdpBuffer, 0, sizeof(soundInfo->rdpBuffer));
    //soundInfo->readBuf = soundInfo->writeBuf = 0;
  }
  
  //soundInfo->writeBuf += size;
  //if (soundInfo->writeBuf >= BUFFER_SIZE) soundInfo->writeBuf = 0;
  //log_verbose("writeBuf: %d", soundInfo->writeBuf);
  
  //    [s performSelector:@selector(flush) onThread:[s thread] withObject:nil waitUntilDone:NO];
	pthread_mutex_unlock(&soundInfo->mutex);
}


int iosaudioDeviceServiceEntry(PFREERDP_RDPSND_DEVICE_ENTRY_POINTS pEntryPoints) {
	rdpsndIosPlugin* iosplugin;
	
	iosplugin = xnew(rdpsndIosPlugin);
	
	pthread_mutex_init(&iosplugin->mutex, NULL);
  
	iosplugin->devplugin.Open            = rdpsnd_ios_open;
	iosplugin->devplugin.FormatSupported = rdpsnd_ios_format_supported;
	iosplugin->devplugin.SetFormat       = rdpsnd_ios_set_format;
	iosplugin->devplugin.SetVolume       = rdpsnd_ios_set_volume;
	iosplugin->devplugin.Start           = rdpsnd_ios_start;
	iosplugin->devplugin.Play            = rdpsnd_ios_play;
	iosplugin->devplugin.Close           = rdpsnd_ios_close;
	iosplugin->devplugin.Free            = rdpsnd_ios_free;
  
	AudioSessionInitialize(NULL, NULL, NULL, NULL);
  UInt32 category = kAudioSessionCategory_MediaPlayback;
  AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
  AudioSessionSetActive(true);
	
	//register ios rdpsnd device
	pEntryPoints->pRegisterRdpsndDevice(pEntryPoints->rdpsnd, &iosplugin->devplugin);

	return 0;
}

