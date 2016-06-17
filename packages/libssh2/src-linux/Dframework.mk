include $(CLEAR_VARS)

LOCAL_PATH := $(call my-dir)

LOCAL_SRC_FILES :=  channel.c comp.c crypt.c hostkey.c kex.c mac.c misc.c packet.c publickey.c scp.c session.c sftp.c userauth.c transport.c version.c knownhost.c agent.c openssl.c pem.c keepalive.c global.c

ifeq (${DDK_ENV_TARGET_OS}, "darwin")

LOCAL_INCLUDES := ${LOCAL_PATH}/include ${LOCAL_PATH}/../../openssl/darwin/x86_64/include

elifeq (${DDK_ENV_TARGET_OS}, "linux")

LOCAL_INCLUDES := ${LOCAL_PATH}/include ${LOCAL_PATH}/../../openssl/linux/x86_64/include

endif

LOCAL_CFLAGS := -fPIC -DHAVE_CONFIG_H=1

LOCAL_MODULE := libssh2

include $(BUILD_STATIC_LIBRARY)
