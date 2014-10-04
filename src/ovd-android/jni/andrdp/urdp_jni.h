/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
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

#ifndef __URDP_JNI_H
#define __URDP_JNI_H

#include "config.h"

#include <errno.h>
#include <locale.h>
#include <pthread.h>
#include <jni.h>

#include <android/bitmap.h>

#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/gdi/dc.h>
#include <freerdp/gdi/region.h>
#include <freerdp/rail/rail.h>
#include <freerdp/cache/cache.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/semaphore.h>
#include <freerdp/utils/event.h>
#include <freerdp/constants.h>

#include "log.h"
#include "urdp.h"
#include "urdp_event.h"

#define RDP_JNI_EXPORT(name) JNIEXPORT jint JNICALL Java_org_ulteo_ovd_Rdp_##name

typedef struct {
	urdp_context* context;
	JavaVM *jvm;
	jobject obj;
	pthread_attr_t attr;
	pthread_t thread;
	jobject backbuffer;
} jni_data_struct;

#endif /* __URDP_JNI_H */
