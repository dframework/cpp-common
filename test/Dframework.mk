ifeq ("", "")
LOCAL_PATH := $(call my-dir)
common_PATH := ${LOCAL_PATH}/..

include $(CLEAR_VARS)

LOCAL_REQUIRE_MODULES := \
    ${common_PATH}/lib \

LOCAL_INCLUDES := \
    ${common_PATH}/include

LOCAL_CFLAGS :=

ifeq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_STATIC_LIBRARIES +=        \
        libws2_32                    \
        libpthread
else
    LOCAL_LDFLAGS := -pthread -ldl
endif

LOCAL_STATIC_LIBRARIES := \
    libdframework-common

#LOCAL_SHARED_LIBRARIES := \
#    libdframework-common

########################################
LOCAL_SRC_FILES := test.cpp
LOCAL_MODULE := test
include $(BUILD_EXCUTABLE)

########################################
#LOCAL_SRC_FILES := http.cpp
#LOCAL_MODULE := http
#include $(BUILD_EXCUTABLE)


#ifneq (${DDK_ENV_TARGET_OS}, "windows")
########################################
#LOCAL_SRC_FILES := dfwfs-umount.cpp
#LOCAL_MODULE := dfwfs-umount
#include $(BUILD_EXCUTABLE)

########################################
#LOCAL_SRC_FILES := thread.cpp
#LOCAL_MODULE := thread
#include $(BUILD_EXCUTABLE)

########################################
#LOCAL_SRC_FILES := lock.cpp
#LOCAL_MODULE := lock
#include $(BUILD_EXCUTABLE)
#endif


endif
