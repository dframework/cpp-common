LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=         \
    OriginFs.cpp           \
    HttpResponse.cpp       \
    HttpRequest.cpp        \
    HttpdModules.cpp       \
    HttpdHost.cpp          \
    HttpdConfigure.cpp     \
    HttpdUtil.cpp          \
    HttpdClient.cpp        \
    HttpdSender.cpp        \
    HttpdSendStream.cpp    \
    HttpdSendLocalFile.cpp \
    HttpdWorker.cpp        \
    HttpdAcceptor.cpp      \
    HttpdThread.cpp        \
    HttpdService.cpp

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_MODULE := libdframework-common-httpd

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

