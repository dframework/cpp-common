LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    HttpContentType.cpp \
    HttpAuth.cpp \
    HttpBasic.cpp \
    HttpDigest.cpp \
    HttpReader.cpp \
    HttpFormData.cpp \
    HttpRound.cpp \
    HttpHeader.cpp \
    HttpUtils.cpp \
    HttpChunked.cpp \
    IHttpReader.cpp \
    HttpClient.cpp \
    HttpLocation.cpp \
    HttpConnection.cpp \
    HttpMakeClientBuffer.cpp \
    HttpQuery.cpp \
    HttpGet.cpp \
    HttpPost.cpp \
    HttpPropfind.cpp \

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_MODULE := libdframework-common-http

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

