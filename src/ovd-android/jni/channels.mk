freerdp_c_include := $(TOP_PATH)/FreeRDP/include $(TOP_PATH)
freerdp_cflags := -DDISABLE_TLS -DL_ENDIAN -DEXT_PATH=\"$(LOCAL_PATH)/../../\"
hooks := hooks

LOCAL_PATH := $(TOP_PATH)/FreeRDP/channels/rdpdr
include $(CLEAR_VARS)
LOCAL_MODULE := rdpdr
LOCAL_CFLAGS := $(freerdp_cflags)
LOCAL_C_INCLUDES := $(freerdp_c_include)
not_included := 
LOCAL_SRC_FILES := $(filter-out $(not_included), $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.c)))
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(TOP_PATH)
local_path := $(TOP_PATH)/FreeRDP/channels/rdpdr/disk
include $(CLEAR_VARS)
LOCAL_MODULE := disk
LOCAL_CFLAGS := $(freerdp_cflags)
LOCAL_C_INCLUDES := $(freerdp_c_include) $(TOP_PATH)/FreeRDP/channels/rdpdr $(TOP_PATH)/FreeRDP/channels/rdpdr/disk
not_included := FreeRDP/channels/rdpdr/disk/disk_file.c
LOCAL_SRC_FILES := $(filter-out $(not_included), $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(local_path)/*.c)))
LOCAL_SRC_FILES += $(hooks)/disk_file.c
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(TOP_PATH)/FreeRDP/channels/cliprdr
include $(CLEAR_VARS)
LOCAL_MODULE := cliprdr
LOCAL_CFLAGS := $(freerdp_cflags)
LOCAL_C_INCLUDES := $(freerdp_c_include)
not_included := 
LOCAL_SRC_FILES := $(filter-out $(not_included), $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.c)))
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(TOP_PATH)/FreeRDP/channels/rdpsnd
include $(CLEAR_VARS)
LOCAL_MODULE := rdpsnd
LOCAL_CFLAGS := $(freerdp_cflags)
LOCAL_C_INCLUDES := $(freerdp_c_include)
not_included := 
LOCAL_SRC_FILES := $(filter-out $(not_included), $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.c)))
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(TOP_PATH)/FreeRDP/channels/ukbrdr
include $(CLEAR_VARS)
LOCAL_MODULE := ukbrdr
LOCAL_CFLAGS := $(freerdp_cflags)
LOCAL_C_INCLUDES := $(freerdp_c_include)
not_included := 
LOCAL_SRC_FILES := $(filter-out $(not_included), $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.c)))
LOCAL_SHARED_LIBRARIES := freerdp
include $(BUILD_SHARED_LIBRARY)

