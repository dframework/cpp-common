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
    LOCAL_CFLAGS += -D_LARGE_FILES=1 -D_FILE_OFFSET_BITS=64 -D__USE_FILE_OFFSET64=1

include $(BUILD_STATIC_LIBRARY)

