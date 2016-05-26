#!/bin/sh


D_PWD=`pwd`
D_DEST_NM="libssh2-1.6.0"


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


ddk_build_all()
{
    ddk-build --add-target=ios-i386
    if [ $? -ne 0 ]; then
        exit 1
    fi
    ddk-build --add-target=ios-x86_64
    if [ $? -ne 0 ]; then
        exit 1
    fi
    ddk-build --add-target=ios-armv7
    if [ $? -ne 0 ]; then
        exit 1
    fi
    ddk-build --add-target=ios-armv7s
    if [ $? -ne 0 ]; then
        exit 1
    fi
    ddk-build --add-target=ios-arm64
    if [ $? -ne 0 ]; then
        exit 1
    fi
}


if test -d src-linux ; then
    echo ""
    echo "You has src-linux folder ..."
    echo ""
    ddk_build_all
    exit 0
fi

if test -d workspace/$D_DEST_NM ; then
    rm -rf workspace/$D_DEST_NM
fi

#############################################

./libssh2-make-linux.sh
if [ $? -ne 0 ]; then
    exit 1
fi

if test ! -d $D_PWD/src-linux ; then
    echo "Not found $D_PWD/src-linux"
    exit 1
fi

cd $D_PWD/src-linux
if [ $? -ne 0 ]; then
    exit 1
fi


ddk_build_all

echo ""
echo "Complete IOS All ... OK"
echo ""

