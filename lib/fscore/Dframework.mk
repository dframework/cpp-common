LOCAL_PATH := $(call my-dir)
include_PATH := ${LOCAL_PATH}/../../include

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=     \
    Mount.cpp          \
    FuseMount.cpp      \
    Fuse_d.cpp         \
    FuseUtil.cpp       \
    FuseBaseWorker.cpp \
    FuseWorker.cpp     \
    FuseRequest.cpp    \
    FsConfig.cpp       \
    FsNode.cpp         \
    FsContext.cpp      \
    FuseOpenCtx.cpp    \
    FuseDirCtx.cpp     \
    FsSample.cpp       \
    FsWorker.cpp       \
    FsService.cpp      \

LOCAL_INCLUDES := \
    ${include_PATH}

LOCAL_MODULE := libdframework-common-fscore

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

include $(BUILD_STATIC_LIBRARY)

