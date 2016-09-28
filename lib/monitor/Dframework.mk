LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=   \
    Monitor.cpp      \
    MonBase.cpp      \
    MonCpustat.cpp   \
    MonMemory.cpp    \
    MonLoadavg.cpp   \
    MonDiskstats.cpp \
    MonPacket.cpp    \
    MonSocket.cpp

LOCAL_CFLAGS := -fPIC

LOCAL_INCLUDES :=                \
    ${LOCAL_PATH}/../../include  \

LOCAL_MODULE := libdframework-common-monitor

ifneq (${DDK_ENV_TARGET_OS}, "windows")
include $(BUILD_STATIC_LIBRARY)
endif

