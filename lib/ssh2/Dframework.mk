include $(CLEAR_VARS)

LOCAL_PATH := $(call my-dir)

pkg_PATH := ${LOCAL_PATH}/../../packages
include_PATH := ${LOCAL_PATH}/../../include

ifeq (${DDK_ENV_TARGET_OS}, "windows")
  libssh2_PATH := ${pkg_PATH}/libssh2/src-mingw
elifeq (${DDK_ENV_TARGET_OS}, "android")
  libssh2_PATH := ${pkg_PATH}/libssh2/src-${DDK_ENV_OSNAME}
elifeq (${DDK_ENV_TARGET_OS}, "ios")
  libssh2_PATH := ${pkg_PATH}/libssh2/src-${DDK_ENV_OSNAME}
else
  libssh2_PATH := ${pkg_PATH}/libssh2/src-${DDK_ENV_TARGET_OS}
endif

LOCAL_SRC_FILES :=          \
    SSH2.cpp                \
    SSH2Global.cpp          \
    SSH2Session.cpp

LOCAL_INCLUDES :=           \
    ${include_PATH}         \
    ${libssh2_PATH}         \
    ${libssh2_PATH}/include

LOCAL_SHARED_LIBRARIES :=   \
    libdframework-common

#LOCAL_LDFLAGS :=            \
    -Wl,--no-as-needed      \
    -pthread -ldl -lz

LOCAL_CFLAGS:= -std=c++0x

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS += -fPIC
endif

LOCAL_MODULE := libdframework-common-ssh2

include $(BUILD_STATIC_LIBRARY)
