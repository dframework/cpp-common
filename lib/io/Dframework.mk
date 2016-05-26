LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=     \
    Stat.cpp          \
    Dir.cpp           \
    DirBox.cpp        \
    File.cpp          \
    RecentlyFiles.cpp \
    UriFs.cpp         \
    HttpFs.cpp        \
    SSH2Fs.cpp        \
    LocalFs.cpp

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_MODULE :=libdframework-common-io

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

