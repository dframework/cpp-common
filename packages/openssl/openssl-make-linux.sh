#!/bin/sh

D_PWD=`pwd`
D_WORKSPACE="workspace"
OPENSSL_NM="openssl-1.0.1r"
OPENSSL_FN="${OPENSSL_NM}.tar.gz"
OPENSSL_BD="${D_PWD}/${D_WORKSPACE}/build"

openssl_exit(){
    if [ $1 -ne 0 ]; then
        echo $2
        exit $1
    fi
}

#####################################################

if test ! -d $D_WORKSPACE ; then
    mkdir -p $D_WORKSPACE
    openssl_exit $? "mkdir -p ${D_WORKSPACE} at ${D_PWD}"
fi

cd $D_WORKSPACE
openssl_exit $? "cd ${D_WORKSPACE} at ${D_PWD}"
echo "Enter ${D_WORKSPACE} ..."

#####################################################
if test ! -f $OPENSSL_FN ; then
    wget https://www.openssl.org/source/$OPENSSL_FN
    openssl_exit $? "wget https://www.openssl.org/source/${OPENSSL_FN} at ${D_WORKSPACE}/"
    echo "wget https://www.openssl.org/source/$OPENSSL_FN"
fi

#####################################################
if test ! -d $OPENSSL_NM ; then
    echo "tar xvfz $OPENSSL_FN"
    tmp=`tar xvfz $OPENSSL_FN`
    openssl_exit $? "tar xvfz $OPENSSL_FN at ${D_WORKSPACE}/\n${tmp}"
fi

#####################################################
cd $OPENSSL_NM
openssl_exit $? "cd $OPENSSL_NM at ${D_WORKSPACE}/"
echo "Enter ${D_WORKSPACE}/${OPENSSL_NM} ..."

./config --prefix=$OPENSSL_BD shared
openssl_exit $? "./config --prefix=${OPENSSL_BD} at ${D_WORKSPACE}/${OPENSSL_NM}\n${tmp}"

make
openssl_exit $? "make at ${D_WORKSPACE}/${OPENSSL_NM}"

make install
openssl_exit $? "make install at ${D_WORKSPACE}/${OPENSSL_NM}"

cd $D_PWD
mkdir -p linux/x86_64
cp -R ${D_PWD}/${D_WORKSPACE}/build/* linux/x86_64/

echo ""
echo "Success openssl for LINUX."
echo ""

