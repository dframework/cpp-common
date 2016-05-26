include $(CLEAR_VARS)

LOCAL_SRC_FILES := sqlite3.c

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC
endif

ifeq (${DDK_ENV_TARGET_OS}, "ios")
    LOCAL_CFLAGS += -Wunused-parameter
endif

LOCAL_MODULE := libdframework-common-sqlite3

include $(BUILD_STATIC_LIBRARY)

