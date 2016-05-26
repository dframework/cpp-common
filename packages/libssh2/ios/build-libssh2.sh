#!/bin/bash
#  Automatic build script for libssh2 
#  for iPhoneOS and iPhoneSimulator
#
#  Created by Felix Schulze on 02.02.11.
#  Copyright 2010-2015 Felix Schulze. All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################
#
# https://github.com/x2on/OpenSSL-for-iPhone.git
###########################################################################

sh_exit(){
    if [ $1 -ne 0 ]; then
        echo $2
        exit $1
    fi
}

D_HOME=`pwd`
D_WORKSPACE="workspace"
mkdir -p $D_WORKSPACE
cd $D_WORKSPACE


#  Change values here
#
VERSION="1.4.3"
#
###########################################################################
#
# Don't change anything here
SDKVERSION=`xcrun -sdk iphoneos --show-sdk-version`                                                          
CURRENTPATH=`pwd`
ARCHS="i386 x86_64 armv7 armv7s arm64"
DEVELOPER=`xcode-select -print-path`
##########
set -e
if [ ! -e libssh2-${VERSION}.tar.gz ]; then
    echo "Downloading libssh2-${VERSION}.tar.gz"
    curl -O https://www.libssh2.org/download/libssh2-${VERSION}.tar.gz
    sh_exit $? "ERROR:: curl -O https://www.libssh2.org/download/libssh2-${VERSION}.tar.gz"
else
    echo "Using libssh2-${VERSION}.tar.gz"
fi

echo "Checking file: libssh2-${VERSION}.tar.gz"
md5=`md5 -q libssh2-${VERSION}.tar.gz`
if [ $md5 != "071004c60c5d6f90354ad1b701013a0b" ]
then
    echo "File corrupt, please download again."
    exit 1
else
    echo "Checksum verified."
fi

mkdir -p bin
mkdir -p lib
mkdir -p src

build()
{
for ARCH in ${ARCHS}
do
	if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]];
	then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi

        echo ""
        echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
	echo "Building libssh2 for ${PLATFORM} ${SDKVERSION} ${ARCH}"
	echo "Please stand by..."
	tar zxf libssh2-${VERSION}.tar.gz -C src
        sh_exit $? "ERROR:: tar zxf libssh2-${VERSION}.tar.gz -C src"
	cd src/libssh2-${VERSION}

	export DEVROOT="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
	export SDKROOT="${DEVROOT}/SDKs/${PLATFORM}${SDKVERSION}.sdk"
	export LD=${DEVROOT}/usr/bin/ld
	export CC=${DEVELOPER}/usr/bin/gcc
	export CXX=${DEVELOPER}/usr/bin/g++
	export AR=${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar
	export AS=${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/as
	export NM=${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/nm
	export RANLIB=${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib
	export LDFLAGS="-arch ${ARCH} -pipe -no-cpp-precomp -isysroot ${SDKROOT} -L${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/lib -miphoneos-version-min=7.0"
	export CFLAGS="-arch ${ARCH} -pipe -no-cpp-precomp -isysroot ${SDKROOT} -I${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/include -miphoneos-version-min=7.0"
	export CPPFLAGS="-arch ${ARCH} -pipe -no-cpp-precomp -isysroot ${SDKROOT} -I${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/include -miphoneos-version-min=7.0"

	mkdir -p "${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"
        sh_exit $? "ERROR:: mkdir -p ${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"

	LOG="${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/build-libssh2-${VERSION}.log"
	echo ${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk
	
		
	HOST="${ARCH}"
	if [ "${ARCH}" == "arm64" ];
	then
		HOST="aarch64"
	fi

        D_OPTIONS="--with-openssl --with-libssl-prefix=${CURRENTPATH}/../../../openssl/ios/ios${SDKVERSION}-${ARCH}"
	
        echo "./configure --host=${HOST}-apple-darwin --prefix=${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk ${D_OPTIONS} --disable-shared --enable-static ..."

        ./configure --host=${HOST}-apple-darwin --prefix=${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk ${D_OPTIONS} --disable-shared --enable-static >> "${LOG}" 2>&1
        sh_exit $? "ERROR:: ./configure --host=${HOST}-apple-darwin --prefix=${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk ${D_OPTIONS} --disable-shared --enable-static >> ${LOG} 2>&1"

        echo "make ..."
	make >> "${LOG}" 2>&1
        sh_exit $? "ERROR:: make"

        echo "make install ..."
	make install >> "${LOG}" 2>&1
        sh_exit $? "ERROR:: make install"

	cd ${CURRENTPATH}
	rm -rf src/libssh2-${VERSION}
	
done
}

build_lipo()
{
  echo ""
  echo "Build library..."
  lipo -create ${CURRENTPATH}/bin/iPhoneSimulator${SDKVERSION}-i386.sdk/lib/libssh2.a ${CURRENTPATH}/bin/iPhoneSimulator${SDKVERSION}-x86_64.sdk/lib/libssh2.a ${CURRENTPATH}/bin/iPhoneOS${SDKVERSION}-armv7.sdk/lib/libssh2.a ${CURRENTPATH}/bin/iPhoneOS${SDKVERSION}-armv7s.sdk/lib/libssh2.a ${CURRENTPATH}/bin/iPhoneOS${SDKVERSION}-arm64.sdk/lib/libssh2.a -output ${CURRENTPATH}/lib/libssh2.a
  mkdir -p ${CURRENTPATH}/include/libssh2
  cp -R ${CURRENTPATH}/bin/iPhoneSimulator${SDKVERSION}-i386.sdk/include/libssh2* ${CURRENTPATH}/include/libssh2/
}

build_cp_lib()
{
  local arch="${1}"
  local orgarch="${1}"
  local lplat=""
  if [[ "${arch}" == "i386" || "${arch}" == "x86_64" ]]; then
    lplat="iPhoneSimulator"
  elif [ "${arch}" == "tv_x86_64" ]; then
    orgarch="x86_64"
    lplat="AppleTVSimulator"
  elif [ "${arch}" == "tv_arm64" ]; then
    orgarch="arm64"
    lplat="AppleTVOS"
  else
    lplat="iPhoneOS"
  fi

    local org_dir="${CURRENTPATH}/bin/${lplat}${SDKVERSION}-${orgarch}.sdk"
    local dst_dir="${D_HOME}/ios${SDKVERSION}-${arch}"

    mkdir -p $dst_dir/lib
    mkdir -p $dst_dir/include/libssh2

    cp    $org_dir/lib/${2} $dst_dir/lib/
    cp -R $org_dir/include/libssh2* $dst_dir/include/libssh2/
}

build_last()
{
  cd $D_HOME

  build_cp_lib "armv7" "libssh2.a"
  build_cp_lib "armv7s" "libssh2.a"
  build_cp_lib "arm64" "libssh2.a"
  build_cp_lib "i386" "libssh2.a"
  build_cp_lib "x86_64" "libssh2.a"
}


build

#build lipo

build_last
  
cd $D_HOME

rm -rf $D_WORKSPACE

echo ""
echo "Building done."
