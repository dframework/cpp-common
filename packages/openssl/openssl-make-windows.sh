#!/bin/sh

D_PWD=`pwd`
D_WORKSPACE="workspace"
OPENSSL_NM="openssl-1.0.1r"
OPENSSL_FN="${OPENSSL_NM}.tar.gz"
OPENSSL_BD="${D_PWD}/${D_WORKSPACE}/build"

openssl_exit(){
    if [ $1 -ne 0 ]; then
        echo "error: $2"
        exit $1
    fi
}

backup_config="Configure.backup"
#MINGW="x86_64-w64-mingw32"
MINGW="i686-w64-mingw32"

#PWD=`pwd`
#WORKSPACE="${PWD}/openssl-build"

if test ! -d $D_WORKSPACE ; then
    mkdir -p $D_WORKSPACE
    openssl_exit $? "mkdir -p ${D_WORKSPACE} at ${D_PWD}"
fi

cd $D_WORKSPACE
openssl_exit $? "cd ${D_WORKSPACE} at ${D_PWD}"
echo "Enter ${D_WORKSPACE} ..."

if test ! -f $OPENSSL_FN ; then
    cp ../$OPENSSL_FN ./
    tar xvfz $OPENSSL_FN
fi

cd $OPENSSL_NM
openssl_exit $? "cd $OPENSSL_NM"

cp Configure ${backup_config}
openssl_exit $? "cp Configure ${backup_config}"

sed -i 's/:.dll.a/ -Wl,--export-all -shared:.dll.a/g' Configure
openssl_exit $? "sed -i 's/:.dll.a/ -Wl,--export-all -shared:.dll.a/g' Configure"

sed -i 's,.*target already defined.*,$target=$_;,g' Configure
openssl_exit $? "sed -i 's,.*target already defined.*,$target=$_;,g' Configure"

#
case ${MINGW} in
(*i?86*) 
    TARGET=mingw
    export CFLAGS="${CFLAGS} -m32"
;;
(*x86_64*)
     TARGET=mingw64
;;
(*) false;;
esac

#
./Configure ${TARGET} shared --cross-compile-prefix=${MINGW}- --prefix=${OPENSSL_BD}/usr
openssl_exit $? "./Configure ${TARGET} shared --cross-compile-prefix=${MINGW}- --prefix=${OPENSSL_BD}/usr"

#
make
openssl_exit $? "make"

#
make install
openssl_exit $? "make install"

#
cp *.dll ${OPENSSL_BD}/usr/lib/
openssl_exit $? "cp *.dll ${OPENSSL_BD}/usr/lib/"

#
cd ${OPENSSL_BD}/usr/include
openssl_exit $? "cd ${OPENSSL_BD}/usr/include"

# Fix compile bug in OpenSSL
sed -i '/#define HEADER_X509V3_H/a \\n#ifdef X509_NAME\n#undef X509_NAME\n#endif' openssl/x509v3.h
openssl_exit $? "sed -i '/#define HEADER_X509V3_H/a \\n#ifdef X509_NAME\n#undef X509_NAME\n#endif' openssl/x509v3.h"

echo "\n ... OK"

