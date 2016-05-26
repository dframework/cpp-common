LOCAL_PATH := $(call my-dir)
pkg_PATH := ${LOCAL_PATH}/../../packages
src_PATH := ${LOCAL_PATH}/../../lib
openssl_inc_PATH := ${pkg_PATH}/openssl/android/include
openssl_lib_PATH := ${pkg_PATH}/openssl/android/obj/local/${TARGET_ARCH_ABI}

sqlite3_PATH := ${pkg_PATH}/sqlite3
sqlite3_src_FILES :=                   \
    ${sqlite3_PATH}/sqlite3.c

pcre2_PATH := ${pkg_PATH}/pcre2/src
pcre2_src_FILES :=                     \
    ${pcre2_PATH}/pcre2_auto_possess.c \
    ${pcre2_PATH}/pcre2_compile.c      \
    ${pcre2_PATH}/pcre2_config.c       \
    ${pcre2_PATH}/pcre2_context.c      \
    ${pcre2_PATH}/pcre2_dfa_match.c    \
    ${pcre2_PATH}/pcre2_error.c        \
    ${pcre2_PATH}/pcre2_jit_compile.c  \
    ${pcre2_PATH}/pcre2_maketables.c   \
    ${pcre2_PATH}/pcre2_match.c        \
    ${pcre2_PATH}/pcre2_match_data.c   \
    ${pcre2_PATH}/pcre2_newline.c      \
    ${pcre2_PATH}/pcre2_ord2utf.c      \
    ${pcre2_PATH}/pcre2_pattern_info.c \
    ${pcre2_PATH}/pcre2_serialize.c    \
    ${pcre2_PATH}/pcre2_string_utils.c \
    ${pcre2_PATH}/pcre2_study.c        \
    ${pcre2_PATH}/pcre2_substitute.c   \
    ${pcre2_PATH}/pcre2_substring.c    \
    ${pcre2_PATH}/pcre2_tables.c       \
    ${pcre2_PATH}/pcre2_ucd.c          \
    ${pcre2_PATH}/pcre2_valid_utf.c    \
    ${pcre2_PATH}/pcre2_xclass.c       \
    ${pcre2_PATH}/pcre2_chartables.c

base_PATH := ${src_PATH}/base
base_src_FILES :=                      \
    ${base_PATH}/Object.cpp            \
    ${base_PATH}/Retval.cpp            \
    ${base_PATH}/Condition.cpp         \
    ${base_PATH}/Thread.cpp            \
    ${base_PATH}/ThreadManager.cpp     \
    ${base_PATH}/System.cpp
    #${base_PATH}/Sigaction.cpp // for windows

filedb_PATH := ${src_PATH}/filedb
filedb_src_FILES :=                    \
    ${filedb_PATH}/Filedb.cpp          \
    ${filedb_PATH}/FiledbSortedArray.cpp

fscore_PATH := ${src_PATH}/fscore
fscore_src_FILES :=                    \
    ${fscore_PATH}/Mount.cpp           \
    ${fscore_PATH}/FuseMount.cpp       \
    ${fscore_PATH}/Fuse_d.cpp          \
    ${fscore_PATH}/FuseUtil.cpp        \
    ${fscore_PATH}/FuseBaseWorker.cpp  \
    ${fscore_PATH}/FuseWorker.cpp      \
    ${fscore_PATH}/FuseRequest.cpp     \
    ${fscore_PATH}/FsConfig.cpp        \
    ${fscore_PATH}/FsNode.cpp          \
    ${fscore_PATH}/FsContext.cpp       \
    ${fscore_PATH}/FuseOpenCtx.cpp     \
    ${fscore_PATH}/FuseDirCtx.cpp      \
    ${fscore_PATH}/FsSample.cpp        \
    ${fscore_PATH}/FsWorker.cpp        \
    ${fscore_PATH}/FsService.cpp

http_PATH := ${src_PATH}/http
http_src_FILES :=                     \
    ${http_PATH}/HttpContentType.cpp  \
    ${http_PATH}/HttpAuth.cpp         \
    ${http_PATH}/HttpBasic.cpp        \
    ${http_PATH}/HttpDigest.cpp       \
    ${http_PATH}/HttpReader.cpp       \
    ${http_PATH}/HttpFormData.cpp     \
    ${http_PATH}/HttpRound.cpp        \
    ${http_PATH}/HttpHeader.cpp       \
    ${http_PATH}/HttpUtils.cpp        \
    ${http_PATH}/HttpChunked.cpp      \
    ${http_PATH}/IHttpReader.cpp      \
    ${http_PATH}/HttpClient.cpp       \
    ${http_PATH}/HttpLocation.cpp     \
    ${http_PATH}/HttpConnection.cpp   \
    ${http_PATH}/HttpMakeClientBuffer.cpp \
    ${http_PATH}/HttpQuery.cpp        \
    ${http_PATH}/HttpGet.cpp          \
    ${http_PATH}/HttpPost.cpp         \
    ${http_PATH}/HttpPropfind.cpp

httpd_PATH := ${src_PATH}/httpd
httpd_src_FILES :=                       \
    ${httpd_PATH}/OriginFs.cpp           \
    ${httpd_PATH}/HttpResponse.cpp       \
    ${httpd_PATH}/HttpRequest.cpp        \
    ${httpd_PATH}/HttpdModules.cpp       \
    ${httpd_PATH}/HttpdHost.cpp          \
    ${httpd_PATH}/HttpdConfigure.cpp     \
    ${httpd_PATH}/HttpdUtil.cpp          \
    ${httpd_PATH}/HttpdClient.cpp        \
    ${httpd_PATH}/HttpdSender.cpp        \
    ${httpd_PATH}/HttpdSendStream.cpp    \
    ${httpd_PATH}/HttpdSendLocalFile.cpp \
    ${httpd_PATH}/HttpdWorker.cpp        \
    ${httpd_PATH}/HttpdAcceptor.cpp      \
    ${httpd_PATH}/HttpdThread.cpp        \
    ${httpd_PATH}/HttpdService.cpp

io_PATH := ${src_PATH}/io
io_src_FILES :=                  \
    ${io_PATH}/Stat.cpp          \
    ${io_PATH}/Dir.cpp           \
    ${io_PATH}/DirBox.cpp        \
    ${io_PATH}/File.cpp          \
    ${io_PATH}/RecentlyFiles.cpp \
    ${io_PATH}/UriFs.cpp         \
    ${io_PATH}/HttpFs.cpp        \
    ${io_PATH}/SSH2Fs.cpp        \
    ${io_PATH}/LocalFs.cpp

lang_PATH := ${src_PATH}/lang
lang_src_FILES :=                \
    ${lang_PATH}/String.cpp      \
    ${lang_PATH}/Integer.cpp     \
    ${lang_PATH}/Long.cpp

log_PATH := ${src_PATH}/log
log_src_FILES :=                 \
    ${log_PATH}/Logger.cpp

net_PATH := ${src_PATH}/net
net_src_FILES :=                 \
    ${net_PATH}/URI.cpp          \
    ${net_PATH}/Hostname.cpp     \
    ${net_PATH}/HostManager.cpp  \
    ${net_PATH}/Net.cpp          \
    ${net_PATH}/Poll.cpp         \
    ${net_PATH}/Socket.cpp       \
    ${net_PATH}/ServerAccept.cpp

sql_PATH := ${src_PATH}/sql
sql_src_FILES :=                    \
    ${sql_PATH}/Sqlite3.cpp

util_PATH := ${src_PATH}/util
util_src_FILES :=                   \
    ${util_PATH}/Time.cpp           \
    ${util_PATH}/Regexp.cpp         \
    ${util_PATH}/NamedValue.cpp     \
    ${util_PATH}/NamedObject.cpp    \
    ${util_PATH}/MicroTimeDepth.cpp \
    ${util_PATH}/Base64.cpp         \
    ${util_PATH}/MD5.cpp            \
    ${util_PATH}/Opt.cpp            \
    ${util_PATH}/StringArray.cpp    \

xml_PATH := ${src_PATH}/xml
xml_src_FILES :=                    \
    ${xml_PATH}/XmlParser.cpp

aroid_PATH := ${src_PATH}/android
aroid_src_FILES :=                  \
    ${aroid_PATH}/langinfo.c

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=               \
    ${base_src_FILES}           \
    ${filedb_src_FILES}         \
    ${fscore_src_FILES}         \
    ${http_src_FILES}           \
    ${httpd_src_FILES}          \
    ${io_src_FILES}             \
    ${lang_src_FILES}           \
    ${log_src_FILES}            \
    ${net_src_FILES}            \
    ${sql_src_FILES}            \
    ${util_src_FILES}           \
    ${xml_src_FILES}            \
    ${sqlite3_src_FILES}        \
    ${pcre2_src_FILES}          \
    ${aroid_src_FILES}

LOCAL_C_INCLUDES :=             \
    ${LOCAL_PATH}/../../include \
    ${pkg_PATH}                 \
    ${pkg_PATH}/pcre2/src


LOCAL_CFLAGS :=                 \
    -DPCRE2_CODE_UNIT_WIDTH=8   \
    -DHAVE_CONFIG_H=1

LOCAL_CPPFLAGS :=               \
    -std=gnu++11

#LOCAL_LDFLAGS :=                \
#    ${openssl_lib_PATH}

LOCAL_STATIC_LIBRARIES :=       \
    libcrypto-static            \
    libssl-static

LOCAL_MODULE :=libdframework-common

include $(BUILD_STATIC_LIBRARY)

