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

#include <freerdp/kbd/kbd.h>

#include "urdp_jni.h"
#include "urdp_debug.h"

static jni_data_struct jni_data;

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	setlocale(LC_ALL, "");
	#ifdef DEBUG_LOG
	urdp_setup_stdio();
	#endif
	return JNI_VERSION_1_6;
}

JNIEnv *getJNIEnv() {
	JNIEnv *env;
	(*jni_data.jvm)->GetEnv(jni_data.jvm, (void **) &env, JNI_VERSION_1_6);
	return env;
}

/*
 * Make a copy of the java jstring object and returns a null terminated utf-8 encoded c char array.
 * You are responsible do unalloc it when unnecessary.
 * see: http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6681965
 */
char* dupjstring(JNIEnv *env, jstring string) {
	const char *array = (*env)->GetStringUTFChars(env, string, NULL);
	jsize length = (*env)->GetStringUTFLength(env, string);
	char* cstring = malloc(length + 1);
	memcpy(cstring, array, length);
	cstring[length] = 0;
	(*env)->ReleaseStringUTFChars(env, string, array);
	return cstring;
}

jmethodID get_jni_method(JNIEnv *env, char* name, char* signature) {
	//find java method of Rdp object
	jclass cls = (*env)->GetObjectClass(env, jni_data.obj);
	if (cls == NULL)
		return NULL;

	jmethodID method = (*env)->GetMethodID(env, cls, name, signature);
	(*env)->DeleteLocalRef(env, cls);
	return method;
}

RDP_JNI_EXPORT(clickDown)(JNIEnv *env, jobject this, jint x, jint y, jint button) {
	log_debug("Panel : click down : x = %d - y = %d - btn = %d", x, y, button);
	return urdp_click_down(jni_data.context, x, y, button);
}

RDP_JNI_EXPORT(clickMove)(JNIEnv *env, jobject this, jint x, jint y) {
	log_debug("Panel : click move : x = %d - y = %d", x, y);
	return urdp_click_move(jni_data.context, x, y);
}

RDP_JNI_EXPORT(clickUp)(JNIEnv *env, jobject this, jint x, jint y, jint button) {
	log_debug("Panel : click up : x = %d - y = %d - btn = %d", x, y, button);
	return urdp_click_up(jni_data.context, x, y, button);
}

RDP_JNI_EXPORT(sendUnicode)(JNIEnv *env, jobject this, jint unicode) {
	log_debug("sendUnicode: 0x%4.4X", unicode);
	return urdp_send_unicode(jni_data.context, unicode);
}

RDP_JNI_EXPORT(sendVkcode)(JNIEnv *env, jobject this, jint up, jint vkeyCode) {
	uint8 scancode;
	boolean extended;
	uint16 flags;

	scancode = freerdp_kbd_get_scancode_by_virtualkey(vkeyCode, &extended);

	if (up != 0)
		flags = KBD_FLAGS_RELEASE;
	else
		flags = KBD_FLAGS_DOWN;

	if (extended)
		flags |= KBD_FLAGS_EXTENDED;

	log_debug("sendVkcode up=%d vkey=0x%X ext=%d key=0x%X", up, vkeyCode, extended, scancode);
	return urdp_send_scancode(jni_data.context, flags, scancode);
}

RDP_JNI_EXPORT(sendScancode)(JNIEnv *env, jobject this, jint up, jint extended, jint keyCode) {
	int flags;

	log_debug("sendScancode up=%d ext=%d key=0x%X", up, extended, keyCode);

	if (up != 0)
		flags = KBD_FLAGS_RELEASE;
	else
		flags = KBD_FLAGS_DOWN;

	if (extended != 0)
		flags |= KBD_FLAGS_EXTENDED;

	return urdp_send_scancode(jni_data.context, flags, keyCode);
}

RDP_JNI_EXPORT(sendImePreeditString)(JNIEnv *env, jobject this, jstring jdata) {
	const char *cdata = (*env)->GetStringChars(env, jdata, NULL);
	const long cdata_size = (*env)->GetStringLength(env, jdata);
	long i;
	log_debug("sendImePreeditString: %ld : ", cdata_size);
	for(i=0 ; i<cdata_size ; ++i) {
		log_debug("0x%02X ", cdata[i]);
	}

	urdp_ukbrdr_send_ime_preedit_string(jni_data.context, cdata, cdata_size*2);
	return 0;
}

RDP_JNI_EXPORT(sendImePreeditStringStop)(JNIEnv *env, jobject this) {
	urdp_ukbrdr_send_ime_preedit_string_stop(jni_data.context);
	return 0;
}

RDP_JNI_EXPORT(sendClipboard)(JNIEnv *env, jobject this, jstring jdata) {
	const char *cdata = (*env)->GetStringUTFChars(env, jdata, NULL);
	log_debug("sendClipboard: %s", cdata);
	urdp_cliprdr_send_unicode(jni_data.context, cdata);
	(*env)->ReleaseStringUTFChars(env, jdata, cdata);
	return 0;
}

void* thread_func(urdp_context* context) {
	JNIEnv *env = getJNIEnv();

	//attach this thread to jvm
	if ((*jni_data.jvm)->AttachCurrentThread(jni_data.jvm, &env, NULL) < 0) {
		log_error("run_rdp : error attach current thread");
		return NULL;
	}

	AND_EXIT_CODE why = urdp_connect(context);

	jmethodID method_onRdpClose = get_jni_method(env, "onRdpClose", "(I)V");
	(*env)->CallVoidMethod(env, jni_data.obj, method_onRdpClose, why);

	(*env)->DeleteGlobalRef(env, jni_data.obj);
	(*env)->DeleteGlobalRef(env, jni_data.backbuffer);

	//detach this thread from jvm
	if ((*jni_data.jvm)->DetachCurrentThread(jni_data.jvm) < 0) {
		log_error("run_rdp : error detach current thread");
		return NULL;
	}

	pthread_exit(NULL);

	log_info("run_rdp : end");

	return NULL;
}

AND_EXIT_CODE jni_begin_paint(urdp_context* context, void** data) {
	JNIEnv *env = getJNIEnv();
	int ret;

	if ((ret = AndroidBitmap_lockPixels(env, jni_data.backbuffer, data) < 0)) {
		log_error("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return ERROR_ALLOC;
	}

	return SUCCESS;
}

AND_EXIT_CODE jni_end_paint(urdp_context* context, int x, int y, int w, int h) {
	JNIEnv *env = getJNIEnv();

	AndroidBitmap_unlockPixels(env, jni_data.backbuffer);
	jmethodID method_draw = get_jni_method(env, "draw", "(IIII)V");
	(*env)->CallVoidMethod(env, jni_data.obj, method_draw, x, y, w, h);
	return SUCCESS;
}

void* jni_desktop_resize(urdp_context* context, int width, int height, int bpp) {
	log_debug("desktop_resize w=%d h=%d bpp=%d", width, height, bpp);

	JNIEnv *env = getJNIEnv();
	jmethodID method_allocBuffer = get_jni_method(env, "allocBuffer", "(III)Landroid/graphics/Bitmap;");
	jobject backbuffer = (*env)->CallObjectMethod(env, jni_data.obj, method_allocBuffer, width, height, bpp);

	//store reference to the backbuffer
	if (jni_data.backbuffer != NULL) {
		(*env)->DeleteGlobalRef(env, jni_data.backbuffer);
	}

	jni_data.backbuffer = (*env)->NewGlobalRef(env, backbuffer);

	return (void*)1;
}

urdp_rdpdr_disk* jni_get_disks(urdp_context* context) {
	JNIEnv *env = getJNIEnv();
	jmethodID method_getDisks = get_jni_method(env, "getDisks", "()[Ljava/lang/String;");
	urdp_rdpdr_disk* disks;
	jobjectArray jdisks = (*env)->CallObjectMethod(env, jni_data.obj, method_getDisks);
	int stringCount = (*env)->GetArrayLength(env, jdisks);
	int i, nb;
	jstring string;

	disks = malloc(sizeof(urdp_rdpdr_disk) * (stringCount+1));
	nb = 0;
	for (i=0; i<stringCount; i+=2) {
		string = (jstring) (*env)->GetObjectArrayElement(env, jdisks, i);
		disks[nb].name = dupjstring(env, string);

		string = (jstring) (*env)->GetObjectArrayElement(env, jdisks, i+1);
		disks[nb].path = dupjstring(env, string);
		nb++;
	}
	disks[nb].name = NULL;
	disks[nb].path = NULL;
	return disks;
}

char* jni_get_client_name(urdp_context* context) {
	JNIEnv *env = getJNIEnv();
	jmethodID method_getClientName = get_jni_method(env, "getClientName", "()Ljava/lang/String;");
	jstring client_name = (*env)->CallObjectMethod(env, jni_data.obj, method_getClientName);
	return dupjstring(env, client_name);
}

AND_EXIT_CODE jni_pre_connect(urdp_context* context) {
	// load rdpsnd plugin
	RDP_PLUGIN_DATA *rdpsnd_data = NULL;

	rdpsnd_data = (RDP_PLUGIN_DATA *) xmalloc(2 * sizeof(RDP_PLUGIN_DATA));
	rdpsnd_data[0].size = sizeof(RDP_PLUGIN_DATA);
	rdpsnd_data[0].data[0] = "sles";
	rdpsnd_data[0].data[1] = NULL;
	rdpsnd_data[0].data[2] = NULL;
	rdpsnd_data[0].data[3] = NULL;
	rdpsnd_data[1].size = 0;

	int ret = freerdp_channels_load_plugin(context->context.channels, context->context.instance->settings, "rdpsnd", rdpsnd_data);
	if (ret != 0) {
		log_error("failed loading rdpsnd plugin");
	} else {
		context->context.instance->settings->audio_playback = true;
	}

	return SUCCESS;
}

AND_EXIT_CODE jni_post_connect(urdp_context* context) {
	return SUCCESS;
}


void jni_process_clipboard(urdp_context* context, uint32 format, const uint8* data, uint32 size) {
	JNIEnv *env = getJNIEnv();
	jmethodID method_update_clipboard = get_jni_method(env, "onUpdateClipboard", "(Ljava/lang/String;)V");
	jstring jdata = (*env)->NewStringUTF(env, (char*)data);

	switch (format) {
	case CB_FORMAT_UNICODETEXT:
	case CB_FORMAT_TEXT:
		(*env)->CallObjectMethod(env, jni_data.obj, method_update_clipboard, jdata);
		break;
	}
}

void jni_process_printjob(urdp_context* context, const char* filename) {
	JNIEnv *env = getJNIEnv();
	jmethodID method_printjob = get_jni_method(env, "onPrintJob", "(Ljava/lang/String;)V");
	jstring jfilename = (*env)->NewStringUTF(env, filename);

	(*env)->CallObjectMethod(env, jni_data.obj, method_printjob, jfilename);
}

AND_EXIT_CODE jni_context_free(urdp_context* context) {
	return SUCCESS;
}

AND_EXIT_CODE jni_context_new(urdp_context* context) {
	jni_data.backbuffer = NULL;
	context->urdp_begin_draw = (p_urdp_begin_draw)jni_begin_paint;
	context->urdp_draw = jni_end_paint;
	context->urdp_desktop_resize = jni_desktop_resize;
	context->urdp_get_client_name = jni_get_client_name;
	context->urdp_get_disks = jni_get_disks;
	context->urdp_pre_connect = jni_pre_connect;
	context->urdp_post_connect = jni_post_connect;
	context->urdp_process_clipboard = jni_process_clipboard;
	context->urdp_process_printjob = jni_process_printjob;
	context->urdp_context_free = jni_context_free;
	return SUCCESS;
}

RDP_JNI_EXPORT(stop)(JNIEnv *env, jobject this) {
	jni_data.context->rdp_run = false;
	pthread_join(jni_data.thread, NULL);
	pthread_attr_destroy(&jni_data.attr);
	log_info("stop : rdp terminated");
	return 0;
}

RDP_JNI_EXPORT(rdp)(JNIEnv *env, jobject this, jint width, jint height, jstring username, jstring password, jstring hostname, jint port, jboolean use_gateway, jstring token,
		jstring rdpshell, jboolean bulk_compress, jint connection_type) {
	rdpSettings* settings;

	(*env)->GetJavaVM(env, &jni_data.jvm);
	jni_data.obj = (*env)->NewGlobalRef(env, this);

	jni_data.context = urdp_init(sizeof(urdp_context), jni_context_new);
	settings = jni_data.context->context.instance->settings;

	settings->width = width;
	settings->height = height;
	settings->color_depth = 16;
	settings->username = dupjstring(env, username);
	settings->password = dupjstring(env, password);
	settings->hostname = dupjstring(env, hostname);
	settings->shell = dupjstring(env, rdpshell);
	if (port > 0)
		settings->port = port;
	if (use_gateway) {
		settings->gateway = true;
		char* ctoken = dupjstring(env, token);
		strncpy(settings->gateway_token, ctoken, sizeof(settings->gateway_token));
		log_debug("Using gateway %s", settings->gateway_token);
		free(ctoken);
	}
	settings->compression = false; //bulk_compress;
	if (settings->compression)
		log_debug("Bulk compression enabled");

	settings->tls_security = false;
	settings->nla_security = false;
	settings->rdp_security = true;
	settings->jpeg = true;
	settings->ns_codec = true;
	settings->connection_type = connection_type;
	settings->disable_wallpaper = true;
	settings->disable_full_window_drag = true;
	settings->disable_menu_animations = true;
	settings->disable_theming = true;
	settings->smooth_fonts = false;
	settings->desktop_composition = false;
	settings->performance_flags = PERF_FLAG_NONE;

	if (settings->smooth_fonts)
		settings->performance_flags |= PERF_ENABLE_FONT_SMOOTHING;

	if (settings->desktop_composition)
		settings->performance_flags |= PERF_ENABLE_DESKTOP_COMPOSITION;

	if (settings->disable_wallpaper)
		settings->performance_flags |= PERF_DISABLE_WALLPAPER;

	if (settings->disable_full_window_drag)
		settings->performance_flags |= PERF_DISABLE_FULLWINDOWDRAG;

	if (settings->disable_menu_animations)
		settings->performance_flags |= PERF_DISABLE_MENUANIMATIONS;

	if (settings->disable_theming)
		settings->performance_flags |= PERF_DISABLE_THEMING;

	if (false) { // RemoteFx
		settings->connection_type = CONNECTION_TYPE_LAN;
		settings->rfx_codec = true;
		settings->fastpath_output = true;
		settings->color_depth = 32;
		settings->frame_acknowledge = false;
		settings->performance_flags = PERF_FLAG_NONE;
		settings->large_pointer = true;
	}

	log_debug("Start RDP : %dx%d %s %s %s:%d", settings->width, settings->height, settings->username, settings->password, settings->hostname, settings->port);

	pthread_attr_init(&jni_data.attr);
	pthread_attr_setdetachstate(&jni_data.attr, PTHREAD_CREATE_JOINABLE);
	if (pthread_create(&jni_data.thread, 0, (void*(*)(void*)) thread_func, jni_data.context) != 0) {
		log_error("main : error pthread_create");
	}

	log_info("main : end rdp init");
	return 0;
}
