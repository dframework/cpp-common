#!/bin/sh

D_PWD=`pwd`
D_ARMV7="armv7"
D_ARM64="arm64"
D_X86="x86"
D_X86_64="x86_64"
D_ORIGIN="origin-x86"

MKDIR(){
    local path=
    path="$1"
    if test ! -d $path/lib ; then
        mkdir -p $path/lib
        if [ $? -ne 0 ]; then
            echo "mkdir $path"
            exit 1
        fi
    fi
}

COPY_OBJ(){
    local SRC="$1"
    local DST="$2"

    cp obj/local/$SRC/libcrypto-static.a ../$DST/lib/libcrypto.a
    if [ $? -ne 0 ]; then
        echo "cp obj/local/$SRC/libcrypto-static.a ../$DST/libcrypto.a"
        exit 1
    fi

    cp obj/local/$SRC/libssl-static.a ../$DST/lib/libssl.a
    if [ $? -ne 0 ]; then
        echo "cp obj/local/$SRC/libssl-static.a ../$DST/libssl.a"
        exit 1
    fi

    cp -R jni/include ../$DST/
}

#MKDIR "$D_ARMV7"
#MKDIR "$D_ARM64"
MKDIR "$D_X86"
MKDIR "$D_X86_64"

cd source
if [ $? -ne 0 ]; then
    echo "cd origin-x86"
    exit 1
fi

ndk-build
if [ $? -ne 0 ]; then
    echo "ndk-build"
    exit 1
fi

#COPY_OBJ "armeabi-v7a" "${D_ARMV7}"
#COPY_OBJ "arm64-v8a" "${D_ARM64}"
COPY_OBJ "x86" "${D_X86}"
COPY_OBJ "x86_64" "${D_X86_64}"

echo " OK "
