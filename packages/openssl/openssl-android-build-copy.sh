#!/bin/sh

mkdir -p android/arm64/lib
mkdir -p android/arm64/include
cp openssl-android-build/libs/android/clang/arm64-v8a/*.a android/arm64/lib/
cp -R openssl-android-build/libs/openssl android/arm64/include/

mkdir -p android/armv7/lib
mkdir -p android/armv7/include
cp openssl-android-build/libs/android/clang/armeabi-v7a/*.a android/armv7/lib/
cp -R openssl-android-build/libs/openssl android/armv7/include/

mkdir -p android/x86/lib
mkdir -p android/x86/include
cp openssl-android-build/libs/android/clang/x86/*.a android/x86/lib/
cp -R openssl-android-build/libs/openssl android/x86/include/

mkdir -p android/x86_64/lib
mkdir -p android/x86_64/include
cp openssl-android-build/libs/android/clang/x86/*.a android/x86_64/lib/
cp -R openssl-android-build/libs/openssl android/x86_64/include/

