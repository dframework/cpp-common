LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    String.cpp     \
    Integer.cpp    \
    Long.cpp       \
    Float.cpp

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_MODULE := libdframework-common-lang

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

