
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

include $(LOCAL_PATH)/../../../ThirdParty/PSCommon/BuildSystem/CommonAndroid.mk

# set path to source
MY_PREFIX := $(LOCAL_PATH)

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)/*.cpp \
	$(MY_PREFIX)/uvc/*.c \
	$(MY_PREFIX)/uvc/*.cpp

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../Include \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/uvc	\
	$(LOCAL_PATH)/../PS1080/Core \
	$(LOCAL_PATH)/../PS1080/Include \
	$(LOCAL_PATH)/../PS1080/DriverImpl \
	$(LOCAL_PATH)/../PS1080/	\
	$(LOCAL_PATH)/../PS1080/Sensor

LOCAL_STATIC_LIBRARIES := XnLib
LOCAL_SHARED_LIBRARIES := libusb 
LOCAL_LDLIBS += -llog

LOCAL_MODULE := Depth3D

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)

#include XnLib
include $(LOCAL_PATH)/../../../ThirdParty/PSCommon/XnLib/Source/Android.mk
