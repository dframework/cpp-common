LOCAL_PATH := $(call my-dir)

ifeq (${DDK_ENV_TARGET_OS}, "windows")
  openssl_PATH := ${LOCAL_PATH}/../packages/openssl/${DDK_ENV_TARGET_OS}/${DDK_ENV_TARGET_CPU}
elifeq (${DDK_ENV_TARGET_OS}, "ios")
  openssl_PATH := ${LOCAL_PATH}/../packages/openssl/${DDK_ENV_TARGET_OS}/ios${DDK_APPLE_SDK_VERSION}-${DDK_ENV_TARGET_CPU}
else
  openssl_PATH := ${LOCAL_PATH}/../packages/openssl/${DDK_ENV_TARGET_OS}/${DDK_ENV_TARGET_CPU}
endif

include $(CLEAR_VARS)

ifeq (${DDK_ENV_TARGET_OS}, "windows")
    LOCAL_SRC_FILES := ../lib/android/langinfo.c
endif
ifeq (${DDK_ENV_TARGET_OS}, "android")
    LOCAL_SRC_FILES := ../lib/android/langinfo.c
endif

LOCAL_INCLUDES :=     \
    ${LOCAL_PATH}/../include

LOCAL_STATIC_LIBRARIES :=        \
    libpcre2                     \
    libdframework-common-sqlite3 \
    libdframework-common-lang    \
    libdframework-common-base    \
    libdframework-common-xml     \
    libdframework-common-util    \
    libdframework-common-io      \
    libdframework-common-net     \
    libdframework-common-filedb  \
    libdframework-common-http    \
    libdframework-common-httpd   \
    libdframework-common-log     \
    libdframework-common-sql     \
    libdframework-common-ssh2    \
    libcrypto                    \
    libssl                       \
    libssh2

ifeq (${DDK_ENV_TARGET_OS}, "windows")
  LOCAL_STATIC_LIBRARIES +=      \
    libdframework-common-dl      \
    libpthread                   \
    libws2_32                    \
    libgdi32                     \
    libpsapi

else
  LOCAL_STATIC_LIBRARIES +=      \
    libdframework-common-monitor \

  LOCAL_LDFLAGS := -lz
endif

@cp ${openssl_PATH}/lib/libcrypto.a ${DDK_ENV_TARGET_BUILD}/
@if [ $? -ne 0 ]; then
@    echo "ERROR: cp openssl-path/lib/libcrypto.a /build-dir/"
@fi
@cp ${openssl_PATH}/lib/libssl.a ${DDK_ENV_TARGET_BUILD}/
@if [ $? -ne 0 ]; then
@    echo "ERROR: cp openssl-path/lib/libssl.a /build-dir/"
@fi
##############################################################
# This is old
#ifeq (${DDK_ENV_TARGET_OS}, "ios")
#    cp ${libssh2_PATH}/lib/libssh2.a ${DDK_ENV_TARGET_BUILD}/
#endif
##############################################################

LOCAL_VERSION := 0.3.28
LOCAL_MODULE := libdframework-common

include $(BUILD_STATIC_LIBRARY)

ifeq (${DDK_ENV_TARGET_OS}, "linux")
  include $(BUILD_SHARED_LIBRARY)
elifeq (${DDK_ENV_TARGET_OS}, "darwin")
  include $(BUILD_SHARED_LIBRARY)
endif

pkg_path:=/usr/local

package:

ifeq (${DDK_ENV_TARGET_OS}, "windows")
  dest_NM := ${LOCAL_MODULE}
  dest_FNM := ${dest_NM}-${LOCAL_VERSION}.zip
  org_PATH := ${DDK_ENV_TARGET_PKG}/${dest_NM}
  dest_PATH := ${org_PATH}/${dest_NM}
  @if test -d ${org_PATH} ; then
  @  rm -rf ${org_PATH}
  @fi

  @mkdir -p ${dest_PATH}
  @cp ${DDK_ENV_TARGET_BUILD}/libdframework-common.a ${dest_PATH}/libdframework-common.lib
  @cp -R ../include ${dest_PATH}/
  @d_pwd=`pwd`
  @cd ${org_PATH}
  @zip -r libdframework-common-${LOCAL_VERSION}.zip ./libdframework-common/* 
  @cd ${d_pwd}
  echo ""
  echo "  Complete packaging for windows (${org_PATH})"
  echo ""
elifeq (${DDK_ENV_TARGET_OS}, "ios")
   @echo "Not supported ios"
   @exit 1
elifeq (${DDK_ENV_TARGET_OS}, "android")
   @echo "Not supported android"
   @exit 1
else
   $(package-start sis,lib,${LOCAL_MODULE},${LOCAL_VERSION})

   ifeq (${DDK_ENV_TARGET_OS}, "darwin")
       $(package ${DDK_ENV_TARGET_PATH}/build/${LOCAL_MODULE}.${LOCAL_VERSION}.${DDC_SHARED_LIB_EXT} ${pkg_path}/lib/)
   else
       $(package ${DDK_ENV_TARGET_PATH}/build/${LOCAL_MODULE}.${DDC_SHARED_LIB_EXT}.${LOCAL_VERSION} ${pkg_path}/lib/)
   endif

   $(package ${DDK_ENV_TARGET_PATH}/build/${LOCAL_MODULE}.a ${pkg_path}/lib/)
   $(package ${LOCAL_PATH}/../include ${pkg_path}/)
   $(package-end)
endif


package-install:

ifeq (${DDK_ENV_TARGET_OS}, "windows")
   @echo "Not supported windows"
eleq (${DDK_ENV_TARGET_OS}, "ios")
   @echo "Not supported ios"
eleq (${DDK_ENV_TARGET_OS}, "android")
   @echo "Not supported android"
else
   $(package-install sis,lib,${LOCAL_MODULE},${LOCAL_VERSION})
endif


package-all:

ifeq (${DDK_ENV_TARGET_OS}, "windows")
   @echo "Not supported windows"
eleq (${DDK_ENV_TARGET_OS}, "ios")
   @echo "Not supported ios"
eleq (${DDK_ENV_TARGET_OS}, "android")
   @echo "Not supported android"
else
    $(target package)
    $(target package-install)
endif


dist:

ifeq (${DDK_ENV_TARGET_OS}, "windows")
   dist_pass=`sudo cat /root/sis-pass`
   dist_host=`sudo cat /root/sis-dist`
   org_PATH := ${DDK_ENV_TARGET_PKG}/${LOCAL_MODULE}
   org_NM := ${org_PATH}/libdframework-common-${LOCAL_VERSION}.zip
   sshpass -p${dist_pass} scp ${org_NM} ${dist_host}:/data/www-real/zonedrm/www/static/download/
   if [ $? -ne 0 ]; then
       echo "    - cp ${org_NM} ... FAIL"
   else
       echo "    - cp ${org_NM} ... OK"
   fi
eleq (${DDK_ENV_TARGET_OS}, "ios")
   @echo "Not supported ios"
eleq (${DDK_ENV_TARGET_OS}, "android")
   @echo "Not supported android"
else
   dist_pass=`sudo cat /root/sis-pass`
   dist_host=`sudo cat /root/sis-dist`
   #dist_nm=$(call_package_get_pkgname "${LOCAL_MODULE}" "${LOCAL_VERSION}" "lib")
   dist_nm=$(call_package_get_pkgname ${LOCAL_MODULE} ${LOCAL_VERSION} lib)
   dist_prefix="${DDK_ENV_TARGET_PKG}/${LOCAL_MODULE}/sis/${dist_nm}"
   sshpass -p${dist_pass} scp ${dist_prefix}.sh ${dist_host}:/data/www-real/zonedrm/www/static/download/
   if [ $? -ne 0 ]; then
       echo "    - cp ${dist_nm}.sh ... FAIL"
   else
       echo "    - cp ${dist_nm}.sh ... OK"
   fi

endif

