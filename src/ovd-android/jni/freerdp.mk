hooks := hooks

include $(CLEAR_VARS)
LOCAL_MODULE := freerdp
LOCAL_PATH := $(TOP_PATH)
LOCAL_C_INCLUDES := $(TOP_PATH)/FreeRDP/include $(TOP_PATH)/openssl/include $(TOP_PATH)
LOCAL_CFLAGS := -DDISABLE_TLS -DL_ENDIAN -DEXT_PATH=\"$(LOCAL_PATH)/../../\" -DPLUGIN_PATH=\"/data/data/org.ulteo.ovd/lib\" -DHOME_PATH=\"/data/data/org.ulteo.ovd\"
LOCAL_SHARED_LIBRARIES := ssl crypto
LOCAL_LDLIBS := -lz -llog

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-utils
not_included := $(addprefix $(local_path)/, args.c passphrase.c file.c)
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))
LOCAL_SRC_FILES += $(hooks)/file.c

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-gdi
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-cache
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-codec
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))
ifneq ($(TARGET_ARCH_ABI),armeabi-v7a)
not_included += $(addprefix $(local_path)/, rfx_neon.c)
endif
ifneq ($(TARGET_ARCH_ABI),x86)
not_included += $(addprefix $(local_path)/, nsc_sse2.c rfx_sse2.c)
endif

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-crypto
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-sspi
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c) $(wildcard $(local_path)/Kerberos/*.c)  $(wildcard $(local_path)/Negociate/*.c)  $(wildcard $(local_path)/NTLM/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-channels
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-kbd
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

local_path := $(TOP_PATH)/FreeRDP/libfreerdp-core
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c))

LOCAL_SRC_FILES := $(filter-out $(patsubst $(LOCAL_PATH)/%, %, $(not_included)), $(LOCAL_SRC_FILES))

include $(BUILD_SHARED_LIBRARY)
