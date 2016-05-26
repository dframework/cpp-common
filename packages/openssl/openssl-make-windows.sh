#!/bin/sh

openssl_exit(){
    if [ $1 -ne 0 ]; then
        echo "error: $2"
        exit $1
    fi
}

PWD=`pwd`
backup_config="Configure.backup"
MINGW="x86_64-w64-mingw32"
WORKSPACE="${PWD}/openssl-build"

cd openssl-1.*
openssl_exit $? "cd openssl-1.*"

cp Configure ${backup_config}
openssl_exit $? "cp Configure ${backup_config}"

sed -i 's/:.dll.a/ -Wl,--export-all -shared:.dll.a/g' Configure
openssl_exit $? "sed -i 's/:.dll.a/ -Wl,--export-all -shared:.dll.a/g' Configure"

sed -i 's,.*target already defined.*,$target=$_;,g' Configure
openssl_exit $? "sed -i 's,.*target already defined.*,$target=$_;,g' Configure"

#
case ${MINGW} in
(*i?86*) TARGET=mingw;;
(*x86_64*) TARGET=mingw64;;
(*) false;;
esac

#
./Configure ${TARGET} shared --cross-compile-prefix=${MINGW}- --prefix=${WORKSPACE}/usr
openssl_exit $? "./Configure ${TARGET} shared --cross-compile-prefix=${MINGW}- --prefix=${WORKSPACE}/usr"

#
make
openssl_exit $? "make"

#
make install
openssl_exit $? "make install"

#
cp *.dll ${WORKSPACE}/usr/lib/
openssl_exit $? "cp *.dll ${WORKSPACE}/usr/lib/"

#
cd ${WORKSPACE}/usr/include
openssl_exit $? "cd ${WORKSPACE}/usr/include"

# Fix compile bug in OpenSSL
sed -i '/#define HEADER_X509V3_H/a \\n#ifdef X509_NAME\n#undef X509_NAME\n#endif' openssl/x509v3.h
openssl_exit $? "sed -i '/#define HEADER_X509V3_H/a \\n#ifdef X509_NAME\n#undef X509_NAME\n#endif' openssl/x509v3.h"

echo "\n ... OK"

