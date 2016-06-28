LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=   \
    Monitor.cpp      \
    MonBase.cpp      \
    MonCpustat.cpp   \
    MonMemory.cpp    \
    MonLoadavg.cpp   \
    MonDiskstats.cpp \
    MonDiskspace.cpp \
    MonPacket.cpp    \
    MonSocket.cpp

LOCAL_CFLAGS := -fPIC

LOCAL_INCLUDES :=                \
    ${LOCAL_PATH}/../../include  \

LOCAL_MODULE := libdframework-common-monitor

include $(BUILD_STATIC_LIBRARY)

