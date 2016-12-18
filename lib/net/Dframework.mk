LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=   \
    URI.cpp          \
    Hostname.cpp     \
    HostManager.cpp  \
    Net.cpp          \
    Poll.cpp         \
    Socket.cpp       \
    ServerAccept.cpp \

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_CFLAGS := -DDFW_USE_POLL -DDDK_ENV_TARGET_CPU=${DDK_ENV_TARGET_CPU}

ifeq (${DDK_ENV_TARGET_CPU}, "x86")
    LOCAL_CFLAGS += -DDDK_ENV_TARGET_CPU_X86
endif

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS += -fPIC
endif


LOCAL_MODULE := libdframework-common-net

include $(BUILD_STATIC_LIBRARY)

