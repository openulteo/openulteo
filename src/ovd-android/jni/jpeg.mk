LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libjpeg

LOCAL_C_INCLUDES := $(LOCAL_PATH)/jpeg/include
LOCAL_SRC_FILES := $(patsubst jni/%, %, $(wildcard $(LOCAL_PATH)/jpeg/*.c))

LOCAL_CFLAGS += -DAVOID_TABLES 
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

include $(BUILD_STATIC_LIBRARY)
