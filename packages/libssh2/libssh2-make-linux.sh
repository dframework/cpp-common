#!/bin/sh

D_PWD=`pwd`
D_WORKSPACE="workspace"
D_DEST_NM="libssh2-1.6.0"
D_DEST_FN="${D_DEST_NM}.tar.gz"
D_DEST_BD="${D_PWD}/${D_WORKSPACE}/build"
  
D_SRCPATH=""
D_SRCS=""
D_MAKELOG="_make.log"

D_OSNAME=$1

if [ "$D_OSNAME" = "" ]; then
  tmp_unamea=`uname -a`
  tmp_unameb=`expr "${tmp_unamea}" : '\(^[A-Za-z0-9]\{1,\}\)[[:blank:]]'`
  D_OSNAME=`echo $tmp_unameb | tr "[A-Z]" "[a-z]"`
fi


if [ "$D_MINGW" = "" ]; then
    case $D_OSNAME in
    linux)
      D_LIBSSL_PREFIX="--with-libssl-prefix=${D_PWD}/../openssl/${D_OSNAME}/x86_64"
      D_DEST_SRC="src-linux"
    ;;
    darwin)
      D_LIBSSL_PREFIX="--with-libssl-prefix=${D_PWD}/../openssl/${D_OSNAME}/x86_64"
      D_DEST_SRC="src-darwin"
    ;;
    android)
      D_LIBSSL_PREFIX="--with-libssl-prefix=${D_PWD}/../openssl/${D_OSNAME}/x86_64"
      D_DEST_SRC="src-$D_OSNAME"
    ;;
  # ios)
  #   D_LIBSSL_PREFIX="--with-libssl-prefix=${D_PWD}/../openssl/${D_OSNAME}"
  #   D_DEST_SRC="src-$D_OSNAME"
  # ;;
  *)
      echo "Not found target name"
      exit 1
  ;;
esac
else
  D_LIBSSL_PREFIX="--with-libssl-prefix=${D_PWD}/../openssl/windows/${CPU_TYPE}"
  D_DEST_SRC="src-mingw"
fi

dest_exit(){
    if [ $1 -ne 0 ]; then
        echo $2
        exit $1
    fi
}

spinner(){
  local stoped=$1
  local delay=0.75
  local spinstr='|/-\'
  local val=""
  while [ "$stoped" ]; do
    if test -f $stoped ; then
      val=`cat $stoped`
      #rm -rf $stoped
      stoped=""
      continue
    fi

    local temp=${spinstr#?}
    printf "[%c] " "$spinstr"
    local spinstr=$temp${spinstr%"$temp"}
    sleep $delay
    printf "\b\b\b\b"
  done

  if [ "$val" = "0" ]; then
    printf " [ FAILED ]    "
    echo ""
    exit 1
  else
    printf " [ OK ]   "
  fi
    printf "\b\b\b\b"
    echo ""
}

downloading(){
  local stoped="_download.stop"
  if test -f $stoped ; then
      rm -rf $stoped
  fi
  if [ "$D_OSNAME" = "darwin" ]; then
      echo -n " * curl https://www.libssh2.org/download/$D_DEST_FN -o $D_DEST_FN ... "
      ((curl https://www.libssh2.org/download/$D_DEST_FN -o $D_DEST_FN > /dev/null 2>&1 \
          && echo "1" > $stoped) || echo "0" > $stoped) & spinner $stoped
  else
      echo -n " * wget http://www.libssh2.org/download/$D_DEST_FN ... "
      ((wget http://www.libssh2.org/download/$D_DEST_FN > /dev/null 2>&1 \
          && echo "1" > $stoped) || echo "0" > $stoped) & spinner $stoped
  fi
}

configure(){
  local stoped="_configure.stop"
  if test -f $stoped ; then
      return 0
      #rm -rf $stoped
  fi
  if [ "$D_OSNAME" = "darwin" ]; then
      echo -n " * Configure ... "
      echo "./configure $D_MINGW --prefix=$D_DEST_BD --enable-shared=yes ${D_LIBSSL_PREFIX}"
      ((./configure $D_MINGW --prefix=$D_DEST_BD --enable-shared=yes ${D_LIBSSL_PREFIX}   > "_configure.log" 2>&1 \
          && echo "1" > $stoped) || echo "0" > $stoped) & spinner $stoped
  else
      #echo "./configure --prefix=$D_DEST_BD --enable-shared=yes ${D_LIBSSL_PREFIX}"
      echo "./configure $D_MINGW --prefix=$D_DEST_BD --enable-shared=yes ${D_LIBSSL_PREFIX}"
      echo -n " * Configure ... "
      ((./configure $D_MINGW --prefix=$D_DEST_BD --enable-shared=yes ${D_LIBSSL_PREFIX} > "_configure.log" 2>&1 \
          && echo "1" > $stoped) || echo "0" > $stoped) & spinner $stoped
  fi
}

making(){
  local stoped="_make.stop"
  if test -f $stoped ; then
      return 0
      #rm -rf $stoped
  fi
  echo -n " * Make ... "
  ((make > $D_MAKELOG 2>&1 \
    && echo "1" > $stoped) || echo "0" > $stoped) & spinner $stoped
}


collect_source(){
  for x in $D_SRC
  do
    if [ "$x" = "$1" ]; then
        return
    fi
  done
  D_SRC="${D_SRC} ${1}"
}

copy_srcs(){
  local path=

  for x in $D_SRC
  do
    path="${D_SRCPATH}/${x}"
    if test ! -f $path ; then
        dest_exit 1 "ERROR: Not find source : ${path}"
    fi
    cp $path $D_PWD/$D_DEST_SRC/
    dest_exit $? "ERROR: cp $path $D_PWD/$D_DEST_SRC/"
    cp $D_SRCPATH/*.h $D_PWD/$D_DEST_SRC/
    dest_exit $? "ERROR: cp $D_SRCPATH/*.h $D_PWD/$D_DEST_SRC/"
    cp $D_SRCPATH/../include/*.h $D_PWD/$D_DEST_SRC/include/
    dest_exit $? "ERROR: cp $D_SRCPATH/../include/*.h $D_PWD/$D_DEST_SRC/include/"
  done
}

make_mk(){
  echo " * Make Dframework.mk"
  local MK_S=""
  MK_S="include \$(CLEAR_VARS)"
  MK_S="${MK_S}\n\nLOCAL_PATH := \$(call my-dir)"
  MK_S="${MK_S}\n\nLOCAL_SRC_FILES := ${D_SRC}"

  MK_S="${MK_S}\n\nLOCAL_INCLUDES := \${LOCAL_PATH}/include \${LOCAL_PATH}/../../openssl/\${DDK_ENV_TARGET_OS}/\${DDK_ENV_TARGET_CPU}/include"

if [ "$D_MINGW" = "" ]; then
  MK_S="${MK_S}\n\nLOCAL_CFLAGS := -fPIC -DHAVE_CONFIG_H=1"
else
  MK_S="${MK_S}\n\nLOCAL_CFLAGS := -DHAVE_CONFIG_H=1"
fi
  MK_S="${MK_S}\n\nLOCAL_MODULE := libssh2"
  MK_S="${MK_S}\n\ninclude \$(BUILD_STATIC_LIBRARY)"

  `echo "${MK_S}" > ${D_PWD}/$D_DEST_SRC/Dframework.mk`
  dest_exit $? "ERROR: Not make mk file. at ${D_PWD}/$D_DEST_SRC/Dframework.mk"
}

searching_linux(){
  echo " * Searching source files ... "
  local cmt=""
  local method=1
  while read line
  do
    case $method in
    1)
      cmt=`expr "$line" : "^[[:print:]]\{1,\}Entering directory '\([[:print:]]\{1,\}\/src\)[']\{1\}\$"`
      if [ "$cmt" != "" ]; then
        method=2
        D_SRCPATH="$cmt"
      fi
    ;;
    2)
      cmt=`expr "$line" : "^[[:print:]]\{1,\}\(Leaving directory '[[:print:]]\{1,\}\/src[']\{1\}\$\)"`
      if [ "$cmt" != "" ]; then
        method=3
        continue
      fi
      cmt=`expr "$line" : "^[[:print:]]\{1,\}gcc[[:blank:]]\{1\}[[:print:]]\{1,\} -c \([[:print:]]\{1,\}\.c\)[[:blank:]]\{1\}[[:print:]]\{1,\}"`
      if [ "$cmt" != "" ]; then
        collect_source "${cmt}"
        continue
      fi
    ;;
    3)
      return 0
    ;;
    esac
  done < "${D_MAKELOG}"
}

searching_darwin(){
  echo " * Searching source files ... "
  local cmt=""
  local method=1

  D_SRCPATH="${D_PWD}/${D_WORKSPACE}/${D_DEST_NM}/src"

  while read line
  do
    case $method in
    1)
      cmt=`expr "$line" : "^[[:print:]]\{1,\}\(--mode=link\)[[:print:]]\{1,\}\$"`
      if [ "$cmt" != "" ]; then
        return 0
      fi
      cmt=`expr "$line" : "^[[:print:]]\{1,\}gcc[[:blank:]]\{1\}[[:print:]]\{1,\} -c \([[:print:]]\{1,\}\.c\)[[:blank:]]\{1\}[[:print:]]\{1,\}"`
      if [ "$cmt" != "" ]; then
        collect_source "${cmt}"
        continue
      fi
    ;;
    esac
  done < "${D_MAKELOG}"
  return 1
}

#####################################################

if [ "$D_MINGW" = "" ]; then
    echo "$D_DEST_NM on $D_OSNAME ..."
else
    echo "$D_DEST_NM on $D_OSNAME for windows(mingw)..."
fi

if test ! -d $D_DEST_SRC ; then
    mkdir -p $D_DEST_SRC 
    dest_exit $? "ERROR: mkdir -p $D_DEST_SRC at ${D_PWD}"
    mkdir -p $D_DEST_SRC/include
    dest_exit $? "ERROR: mkdir -p $D_DEST_SRC/include at ${D_PWD}"
fi

if test ! -d $D_WORKSPACE ; then
    mkdir -p $D_WORKSPACE
    dest_exit $? "ERROR: mkdir -p ${D_WORKSPACE} at ${D_PWD}"
fi

cd $D_WORKSPACE
dest_exit $? "ERROR: cd ${D_WORKSPACE} at ${D_PWD}"
echo "Entering ${D_WORKSPACE} ..."

#####################################################
if test ! -f $D_DEST_FN ; then
    downloading
fi

#####################################################
if test -d $D_DEST_NM ; then
    rm -rf $D_DEST_NM
    dest_exit $? "ERROR: rm -rf $D_DEST_NM"
fi

if test ! -d $D_DEST_NM ; then
    tmp=`tar xvfz $D_DEST_FN 2>&1`
    dest_exit $? "ERROR: tar xvfz $D_DEST_FN at ${D_WORKSPACE}/\n${tmp}"
fi

#####################################################
cd $D_DEST_NM
dest_exit $? "ERROR: cd $D_DEST_NM at ${D_WORKSPACE}/"
echo "Entering ${D_WORKSPACE}/${D_DEST_NM} ..."

configure

making

if [ "$D_OSNAME" = "darwin" ]; then
    searching_darwin
else
    searching_linux
fi

copy_srcs

make_mk

echo ""
echo "Success libssh2 to Dframework.mk."
echo ""

exit 0

make install
dest_exit $? "ERROR: make install at ${D_WORKSPACE}/${D_DEST_NM}"

echo ""
echo "Success libssh2 for LINUX."
echo ""

