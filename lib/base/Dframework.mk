LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=    \
    Object.cpp        \
    Retval.cpp        \
    Condition.cpp     \
    Thread.cpp        \
    ThreadManager.cpp \
    System.cpp

ifeq (${DDK_ENV_TARGET_OS}, "windows")
  LOCAL_SRC_FILES +=  \
    Sigaction.cpp
endif

LOCAL_INCLUDES :=     \
    ${include_PATH}

LOCAL_CFLAGS:= -std=c++0x

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS += -fPIC
endif

LOCAL_MODULE := libdframework-common-base

include $(BUILD_STATIC_LIBRARY)

