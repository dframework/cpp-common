#!/bin/sh

D_PWD=`pwd`
D_WORKSPACE="workspace"
D_TARGETS=""

tmp_unamea=`uname -a`
tmp_unameb=`expr "${tmp_unamea}" : '\(^[A-Za-z0-9]\{1,\}\)[[:blank:]]'`
D_ENV_OSNAME=`echo $tmp_unameb | tr "[A-Z]" "[a-z]"`


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


get_targets()
{
    local iserror=0
    local list=
    local val=
    local target=
    local testtargets=`ddk-build --print-targets`
    for target in $testtargets
    do
       val=`expr "${target}" : '^android-\([[:print:]]\{1,\}\)'`
       if [ "$val" != "" ]; then
           case "${val}" in
           armv7) D_TARGETS="${D_TARGETS} armeabi-v7a" ;;
           arm64) D_TARGETS="${D_TARGETS} arm64-v8a" ;;
           x86) D_TARGETS="${D_TARGETS} x86" ;;
           x86_64) D_TARGETS="${D_TARGETS} x86_64" ;;
           *)
               iserror=1
               echo "Uknown target ${val} in get_targets() at build-openssl.sh"
           ;;
           esac
       fi
    done

    if [ $iserror -eq 1 ]; then
        exit 1
    fi
}

get_targets
D_APPLICATION_TEXT="APP_ABI := ${D_TARGETS}"

mkdir -p $D_WORKSPACE
cd $D_WORKSPACE

if test ! -d jni ; then
    if test ! -d android-external-openssl ; then
        git clone https://github.com/fries/android-external-openssl.git
        if [ $? -ne 0 ]; then
            echo "ERROR: git clone https://github.com/fries/android-external-openssl.git"
            exit 1
        fi
    fi
    mv android-external-openssl jni
    if [ $? -ne 0 ]; then
        echo "ERROR: mv android-external-openssl jni"
        exit 1
    fi
fi

cp $D_PWD/AndroidManifest.xml ./
if [ $? -ne 0 ]; then
    echo "ERROR: cp $D_PWD/AndroidMainfest.xml ./"
    exit 1
fi

cd jni
if [ $? -ne 0 ]; then
    echo "ERROR: cd jni"
    exit 1
fi

if test -f "Application.mk" ; then
    date=`date +%Y%m%d%H%I%S`
    mv Application.mk Application.mk-${date}
fi

D_APPLICATION_TEXT="APP_ABI := ${D_TARGETS}"
if [ "$D_ENV_OSNAME" = "centos" ]; then
`echo -e ${D_APPLICATION_TEXT} > Application.mk`
else
`echo ${D_APPLICATION_TEXT} > Application.mk`
fi
if [ $? -ne 0 ]; then
    echo "ERROR: echo D_APPLICATION_TEXT > Application.mk"
    exit 1
fi

install()
{
  local val=
  local target=
  for target in ${D_TARGETS}
  do
    val=$target
    case $val in
    armeabi-v7a) val="armv7" ;;
    arm64-v8a) val="arm64" ;;
    esac

    local path="${D_PWD}/${D_WORKSPACE}/obj/local/${target}"
    local crypto="${path}/libcrypto-static.a"
    local ssl="${path}/libssl-static.a"
    local include="${D_PWD}/${D_WORKSPACE}/jni/include"
    if test ! -f ${crypto} ; then
        echo "Not found ${crypto}"
        exit 1
    fi
    if test ! -f ${ssl} ; then
        echo "Not found ${ssl}"
        exit 1
    fi

    local dest="${val}"
    mkdir -p $dest/lib
    cp $crypto $dest/lib/libcrypto.a
    cp $ssl $dest/lib/libssl.a
    cp -R $include $dest/
  done
}

ndk_build(){
  ndk-build
  if [ $? -ne 0 ]; then
      echo "ERROR: ndk-build"
      exit 1
  fi
}

###############################
ndk_build

cd $D_PWD

install

#rm -rf $D_WORKSPACE

