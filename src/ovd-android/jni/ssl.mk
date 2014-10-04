LOCAL_PATH:= $(call my-dir)
OPENSSL_PATH := openssl
include $(CLEAR_VARS)

# Load packaging rules from openssl-android project
include $(LOCAL_PATH)/$(OPENSSL_PATH)/Ssl-config-target.mk
include $(LOCAL_PATH)/$(OPENSSL_PATH)/android-config.mk

LOCAL_MODULE := libssl

# Rewrite includes path
LOCAL_C_INCLUDES := $(addprefix $(LOCAL_PATH)/$(OPENSSL_PATH)/, $(patsubst external/openssl/%,%, $(LOCAL_C_INCLUDES)))

# Select current arch and rewrite src path
ifeq ($(TARGET_ARCH),arm)
	LOCAL_SRC_FILES := $(addprefix $(OPENSSL_PATH)/, $(LOCAL_SRC_FILES_arm))
	LOCAL_CFLAGS += $(LOCAL_CFLAGS_arm)
endif

ifeq ($(TARGET_ARCH),arm64)
	LOCAL_SRC_FILES := $(addprefix $(OPENSSL_PATH)/, $(LOCAL_SRC_FILES_arm64))
	LOCAL_CFLAGS += $(LOCAL_CFLAGS_arm64)
endif

ifeq ($(TARGET_ARCH),x86)
	LOCAL_SRC_FILES := $(addprefix $(OPENSSL_PATH)/, $(LOCAL_SRC_FILES_x86))
	LOCAL_CFLAGS += $(LOCAL_CFLAGS_x86)
endif

ifeq ($(TARGET_ARCH),x86_64)
	LOCAL_SRC_FILES := $(addprefix $(OPENSSL_PATH)/, $(LOCAL_SRC_FILES_x86_64))
	LOCAL_CFLAGS += $(LOCAL_CFLAGS_x86_64)
endif

include $(BUILD_STATIC_LIBRARY)
