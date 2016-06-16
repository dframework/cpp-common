include $(CLEAR_VARS)

LOCAL_PATH := $(call my-dir)

LOCAL_SRC_FILES :=  crypt.c channel.c comp.c hostkey.c kex.c mac.c misc.c packet.c publickey.c scp.c session.c sftp.c userauth.c transport.c version.c knownhost.c agent.c openssl.c pem.c keepalive.c global.c

LOCAL_INCLUDES := ${LOCAL_PATH}/include 
ifeq (${DDK_ENV_TARGET_OS}, "ios")
    LOCAL_INCLUDES += ${LOCAL_PATH}/../../openssl/${DDK_ENV_TARGET_OS}/${DDK_ENV_TARGET_OS}${DDK_APPLE_SDK_VERSION}-${DDK_ENV_TARGET_CPU}/include
else
    LOCAL_INCLUDES += ${LOCAL_PATH}/../../openssl/${DDK_ENV_TARGET_OS}/${DDK_ENV_TARGET_CPU}/include
endif

LOCAL_CFLAGS := -fPIC -DHAVE_CONFIG_H=1

LOCAL_MODULE := libssh2

include $(BUILD_STATIC_LIBRARY)
