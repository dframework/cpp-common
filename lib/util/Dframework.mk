LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include
packages_PATH := ${LOCAL_PATH}/../../packages

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=     \
    Util.cpp           \
    Time.cpp           \
    Regexp.cpp         \
    NamedValue.cpp     \
    NamedObject.cpp    \
    MicroTimeDepth.cpp \
    Base64.cpp         \
    MD5.cpp            \
    Opt.cpp            \
    StringArray.cpp    \

LOCAL_INCLUDES :=      \
    ${include_PATH}    \
    ${packages_PATH}/pcre2/src

ifeq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -DPCRE2_STATIC
else
    LOCAL_CFLAGS := -fPIC
endif

LOCAL_MODULE := libdframework-common-util

include $(BUILD_STATIC_LIBRARY)

