LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := andrdp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../FreeRDP/include \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../FreeRDP/libfreerdp-gdi/ \
	$(TOP_PATH)/jpeg/include

LOCAL_CFLAGS := -Wall

LOCAL_SRC_FILES +=  urdp.c \
					urdp_cliprdr.c \
					urdp_event.c \
					urdp_pointer.c \
					urdp_rdpdr.c \
					urdp_jni.c \
					urdp_debug.c \
					urdp_ukbrdr.c \
					urdp_jpeg.c

LOCAL_SHARED_LIBRARIES += freerdp jpeg

LOCAL_LDLIBS := -llog \
				-ljnigraphics

include $(BUILD_SHARED_LIBRARY)


# android sound support

include $(CLEAR_VARS)
LOCAL_MODULE := rdpsnd_sles
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../FreeRDP/include $(LOCAL_PATH)/../FreeRDP/channels/rdpsnd $(LOCAL_PATH)/../
LOCAL_CFLAGS := -Wall -DPLUGIN_PATH=\"/data/data/org.ulteo.ovd/lib\"
LOCAL_SRC_FILES := urdp_audio.c
LOCAL_LDLIBS := -llog -lOpenSLES
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

# android print support

include $(CLEAR_VARS)
LOCAL_MODULE := rdpdr_pdf
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../FreeRDP/include $(LOCAL_PATH)/../FreeRDP/channels/rdpdr $(LOCAL_PATH)/../
LOCAL_CFLAGS := -Wall -DPLUGIN_PATH=\"/data/data/org.ulteo.ovd/lib\"
LOCAL_SRC_FILES := urdp_pdf.c
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

# android ukbrdr support
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := ukbrdr
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/../FreeRDP/include $(LOCAL_PATH)/../FreeRDP/channels/ukbrdr $(LOCAL_PATH)/../
#LOCAL_CFLAGS := -Wall -DPLUGIN_PATH=\"/data/data/org.ulteo.ovd/lib\"
#LOCAL_SRC_FILES := urdp_ukbrdr.c
#LOCAL_LDLIBS := -llog
#LOCAL_SHARED_LIBRARIES := freerdp
#include $(BUILD_SHARED_LIBRARY)
