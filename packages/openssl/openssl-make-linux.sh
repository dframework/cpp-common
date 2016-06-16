#!/bin/sh

D_PWD=`pwd`
D_WORKSPACE="workspace"
OPENSSL_NM="openssl-1.0.1r"
OPENSSL_FN="${OPENSSL_NM}.tar.gz"
OPENSSL_BD="${D_PWD}/${D_WORKSPACE}/build"

tmp_unamea=`uname -a`
tmp_unameb=`expr "${tmp_unamea}" : '\(^[A-Za-z0-9]\{1,\}\)[[:blank:]]'`
D_OSNAME=`echo $tmp_unameb | tr "[A-Z]" "[a-z]"`


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
    if [ "$D_OSNAME" = "darwin" ]; then
        curl https://www.openssl.org/source/$OPENSSL_FN -o $OPENSSL_FN
        openssl_exit $? "curl https://www.openssl.org/source/$OPENSSL_FN -o $OPENSSL_FN at ${D_WORKSPACE}/"
        echo "curl https://www.openssl.org/source/$OPENSSL_FN -o $OPENSSL_FN"
    else
        wget https://www.openssl.org/source/$OPENSSL_FN
        openssl_exit $? "wget https://www.openssl.org/source/${OPENSSL_FN} at ${D_WORKSPACE}/"
        echo "wget https://www.openssl.org/source/$OPENSSL_FN"
    fi
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

    
if [ "$D_OSNAME" = "darwin" ]; then
    ./Configure darwin64-x86_64-cc --prefix=$OPENSSL_BD shared
    openssl_exit $? "./Configure darwin64-x86_64-cc --prefix=$OPENSSL_BD shared at ${D_WORKSPACE}/${OPENSSL_NM}\n${tmp}"
else
    ./config --prefix=$OPENSSL_BD shared
    openssl_exit $? "./config --prefix=${OPENSSL_BD} at ${D_WORKSPACE}/${OPENSSL_NM}\n${tmp}"
fi

make
openssl_exit $? "make at ${D_WORKSPACE}/${OPENSSL_NM}"

make install
openssl_exit $? "make install at ${D_WORKSPACE}/${OPENSSL_NM}"

cd $D_PWD
mkdir -p ${D_OSNAME}/x86_64
cp -R ${D_PWD}/${D_WORKSPACE}/build/* ${D_OSNAME}/x86_64/

echo ""
echo "Success openssl for LINUX."
echo ""

