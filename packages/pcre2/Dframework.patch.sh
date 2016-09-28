#!/bin/sh

F_CONFIGURE(){

    tar xvfz pcre2-10.20.tar.gz
    if [ $? -ne 0 ]; then
        return 1
    fi

    nowdir=`pwd`
    cd pcre2-10.20
    if [ $? -ne 0 ]; then
        echo "  @ ERROR: cd pcre2-10.20"
        echo "       at ${nowdir}"
        return 1
    fi

    nowdir=`pwd`
    ./configure --prefix=$nowdir/build
    if [ $? -ne 0 ]; then
        echo "  @ ERROR: ./configure"
        echo "       at ${nowdir}"
        return 1
    fi

    rm -rf "src/pcre2_chartables.c"
    cp "src/pcre2_chartables.c.dist" "src/pcre2_chartables.c"
    if [ $? -ne 0 ]; then
        echo "  @ ERROR: cp \"src/pcre2_chartables.c.dist\" \"src/pcre2_chartables.c\""
        echo "       at ${nowdir}"
        return 1
    fi

    return 0
}

F_COPYIT(){
    SRC="pcre2_auto_possess.c
         pcre2_compile.c 
         pcre2_config.c  
         pcre2_context.c  
         pcre2_dfa_match.c  
         pcre2_error.c    
         pcre2_jit_compile.c 
         pcre2_maketables.c
         pcre2_match.c 
         pcre2_match_data.c  
         pcre2_newline.c    
         pcre2_ord2utf.c   
         pcre2_pattern_info.c  
         pcre2_serialize.c  
         pcre2_string_utils.c  
         pcre2_study.c
         pcre2_substitute.c
         pcre2_substring.c
         pcre2_tables.c
         pcre2_ucd.c
         pcre2_valid_utf.c
         pcre2_xclass.c
         pcre2_chartables.c"

    SRC_S=""
    SRC_A=$(echo $SRC | tr " " "\n")
    for x in $SRC_A
    do
        src_x="src/${x}"
        cp "${src_x}" "../src/${x}"
        ret=$?
        if [ $ret -ne 0 ]; then
            echo "  @ ERROR: cp \"${src_x}\" \"../src/${x}\""
            echo "       at ${nowdir}"
            return $ret
        fi
        if [ "$SRC_S" = "" ]; then
            SRC_S="${x}"
        else
            SRC_S="${SRC_S} ${x}"
        fi
    done

    HEADER="config.h pcre2.h 
            pcre2_internal.h  
            pcre2_intmodedep.h
            pcre2_ucp.h
            pcre2posix.h
            pcre2_jit_match.c
            pcre2_jit_misc.c"
    HEADER_A=$(echo $HEADER | tr " " "\n")
    for x in $HEADER_A
    do
        header_x="src/${x}"
        cp "${header_x}" "../src/${x}"
        ret=$?
        if [ $ret -ne 0 ]; then
            echo "  @ ERROR: cp \"${header_x}\" \"../src/${x}\""
            echo "       at ${nowdir}"
            return $ret
        fi
    done

    return 0
}


F_MAKE_MK(){
    MK_S="include \$(CLEAR_VARS)"
    MK_S="${MK_S}\n\nLOCAL_SRC_FILES := ${SRC_S}"
    MK_S="${MK_S}\n\nLOCAL_CFLAGS := -fPIC -DPCRE2_CODE_UNIT_WIDTH=8 -DHAVE_CONFIG_H=1"
    MK_S="${MK_S}\n\nLOCAL_MODULE := libpcre2"
    MK_S="${MK_S}\n\ninclude \$(BUILD_STATIC_LIBRARY)"

    nowdir=`pwd`
    `echo "${MK_S}" > src/Dframework.mk`
    ret=$?
    if [ $ret -ne 0 ]; then
        echo "  @ ERROR: echo 'contents' > src/Dframework.mk"
        echo "      at ${nowdir}"
        return $ret
    fi

    return 0
}

#dftables.c  
#pcre2demo.c
#pcre2_printint.c  
#pcre2grep.c
#pcre2posix.c
#pcre2_jit_match.c 
#pcre2test.c
#pcre2_jit_misc.c    
#pcre2_jit_test.c   

F_MAIN(){
    current=`pwd`

    F_CONFIGURE
    ret=$?
    if [ $ret -ne 0 ]; then
        return $ret
    fi

    F_COPYIT
    ret=$?
    if [ $ret -ne 0 ]; then
        return $ret
    fi

    cd "$current"
    ret=$?
    if [ $ret -ne 0 ]; then
        echo "  @ ERROR: cd \"$current\""
        echo "       at ${nowdir}"
        return $ret
    fi

    F_MAKE_MK
    ret=$?
    if [ $ret -ne 0 ]; then
        return $ret
    fi

#    rm -rf pcre2-10.20

    return 0
}

F_MAIN
ret=$?
if [ $ret -ne 0 ]; then
    echo "  ... FAILED\n"
    exit $ret
else
    echo "  ... OK\n"
fi

