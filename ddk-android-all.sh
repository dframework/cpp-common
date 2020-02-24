#!/bin/sh

D_PWD=`pwd`

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

sample()
{
  echo ""
  echo " *** Android build ***"
  echo ""
  echo " 1. Openssl"
  echo " 2. libssh2"
  echo " 3. cpp-common of dframework"
  echo ""

  ############################################
  # for openssl
  echo ""
  echo " *** Openssl for Android"
  echo ""
  cd packages/openssl/android
  ./build-openssl.sh
  if [ $? -ne 0 ]; then
    echo "Not compile openssl for android."
    exit 1
  fi
  cd $D_PWD

  ############################################
  # for libssh2
  echo ""
  echo " *** libssh2 for Android"
  echo ""
  cd packages/libssh2
  ./build-libssh2-android.sh
  if [ $? -ne 0 ]; then
    echo "Not compile libssh2 for android."
    exit 1
  fi
  cd $D_PWD
}

ddk_build_libssh2()
{
    local libssh2path="packages/libssh2/src-linux"

    cd $libssh2path
    if [ $? -ne 0 ]; then
        echo "Error:  cd packages/libssh2/src-linux"
        exit 1
    fi
    echo "Enter {$libssh2path}"

    ddk_build_target "x86"
    ddk_build_target "x86_64"
#    ddk_build_target "armv7"
#    ddk_build_target "arm64"

    cd $D_PWD
    if [ $? -ne 0 ]; then
        echo "Error:  cd {$D_PWD}"
        exit 1
    fi
    echo "Leave {$libssh2path}"
}

ddk_build_target()
{
    ddk-build --add-target=android-${1}
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

############################################
# for dframework
echo ""
echo " *** dframework cpp-common for android"
echo ""

ddk_build_libssh2

ddk_build_target "x86"
ddk_build_target "x86_64"
#ddk_build_target "armv7"
#ddk_build_target "arm64"

echo ""
echo "Complete Android All ... OK"
echo ""

