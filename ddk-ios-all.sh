#!/bin/sh

if [ "$DFRAMEWORK_DDK_HOME" = "" ]; then
    echo "Not found DFRAMEWORK_DDK_HOME environment."
    exit 1
fi

ddk_shell="$DFRAMEWORK_DDK_HOME/build/make.sh"
if test ! -f $ddk_shell ; then
    echo "Not found ddk-build excutable shell script : $ddk_shell"
    exit 1
fi

which=`which ddk-build`
if [ "$which" = "" ]; then
    alias ddk-build='$DFRAMEWORK_DDK_HOME/build/make.sh'
fi


HOST_OS=`uname -s`
if [ "$HOST_OS" != "Darwin" ]; then
    echo "Build for iOS on Darwin. Your OS is $HOST_OS."
    echo "You can build on Darwin (Mac Os)."
    exit 1
fi


ddk_build_target()
{
    ddk-build --add-target=ios-${1}
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

D_PWD=`pwd`

echo ""
echo " *** iOS build ***"
echo ""
echo " 1. Openssl"
echo " 2. libssh2"
echo " 3. cpp-common of dframework"
echo ""

############################################
# for openssl
echo ""
echo " *** Openssl for iOS"
echo ""
cd packages/openssl/ios
./build-openssl.sh
if [ $? -ne 0 ]; then
    echo "Not compile openssl for ios."
    exit 1
fi
cd $D_PWD

############################################
# for libssh2
echo ""
echo " *** libssh2 for iOS"
echo ""
cd packages/libssh2
./build-libssh2-ios.sh
if [ $? -ne 0 ]; then
    echo "Not compile libssh2 for ios."
    exit 1
fi
cd $D_PWD

############################################
# for dframework
echo ""
echo " *** dframework cpp-common for iOS"
echo ""
ddk_build_target "i386"
ddk_build_target "x86_64"
ddk_build_target "armv7"
ddk_build_target "armv7s"
ddk_build_target "arm64"


echo ""
echo "Complete IOS All ... OK"
echo ""

