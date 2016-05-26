LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include
packages_PATH := ${LOCAL_PATH}/../../packages

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    Sqlite3.cpp    \

LOCAL_INCLUDES :=   \
    ${include_PATH} \
    ${packages_PATH}

LOCAL_MODULE := libdframework-common-sql

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

