include $(CLEAR_VARS)

LOCAL_SRC_FILES := pcre2_auto_possess.c pcre2_compile.c pcre2_config.c pcre2_context.c pcre2_dfa_match.c pcre2_error.c pcre2_jit_compile.c pcre2_maketables.c pcre2_match.c pcre2_match_data.c pcre2_newline.c pcre2_ord2utf.c pcre2_pattern_info.c pcre2_serialize.c pcre2_string_utils.c pcre2_study.c pcre2_substitute.c pcre2_substring.c pcre2_tables.c pcre2_ucd.c pcre2_valid_utf.c pcre2_xclass.c pcre2_chartables.c

ifneq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_CFLAGS := -fPIC -DPCRE2_CODE_UNIT_WIDTH=8 -DHAVE_CONFIG_H=1
else
    LOCAL_CFLAGS := -DPCRE2_CODE_UNIT_WIDTH=8 -DHAVE_CONFIG_H=1
endif

LOCAL_MODULE := libpcre2

include $(BUILD_STATIC_LIBRARY)
