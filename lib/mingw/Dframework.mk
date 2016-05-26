LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

ifeq (${DDK_ENV_TARGET_OS}, "windows")

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= dlfcn.c

LOCAL_INCLUDES :=   \
    ${include_PATH}

LOCAL_MODULE :=libdframework-common-dl

include $(BUILD_STATIC_LIBRARY)

endif
