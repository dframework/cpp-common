#!/bin/sh

mkdir -p android/arm64/lib
mkdir -p android/arm64/include
cp openssl-android-build/libs/android/clang/arm64-v8a/*.a android/arm64/lib/
cp -R openssl-android-build/libs/openssl android/arm64/include/

