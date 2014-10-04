TOP_PATH := $(call my-dir)

include $(TOP_PATH)/jpeg.mk
include $(TOP_PATH)/crypto.mk
include $(TOP_PATH)/ssl.mk
include $(TOP_PATH)/freerdp.mk
include $(TOP_PATH)/channels.mk
include $(TOP_PATH)/andrdp/Android.mk
