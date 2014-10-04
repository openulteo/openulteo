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

#ifndef __URDP_LOG_H
#define __URDP_LOG_H

#include "urdp_config.h"

#if DEBUG_ON == true
 #define DEBUG_LOG 1
#endif

#ifdef __ANDROID__

#include <android/log.h>


#define LOG_TAG "urdp"

#define log_error(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define log_info(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define log_warning(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#ifdef DEBUG_LOG
#define log_debug(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define log_verbose(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define log_debug(...) ;
#define log_verbose(...) ;
#endif

#else // __ANDROID__

#define log_error(...) printf("Error: "__VA_ARGS__);printf("\n")
#define log_info(...) printf("Info: "__VA_ARGS__);printf("\n")
#define log_warning(...) printf("Warning: "__VA_ARGS__);printf("\n")
#ifdef DEBUG_LOG
#define log_debug(...) printf("Debug: "__VA_ARGS__);printf("\n")
#define log_verbose(...) printf("Verbose: "__VA_ARGS__);printf("\n")
#else
#define log_debug(...) ;
#define log_verbose(...) ;
#endif

#endif // __ANDROID__


#endif // __URDP_LOG_H
