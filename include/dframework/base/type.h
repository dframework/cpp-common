#ifndef DFRAMEWORK_BASE_TYPE_H
#define DFRAMEWORK_BASE_TYPE_H
#define DFRAMEWORK_BASE_TYPE_VERSION 1


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define DFRAMEWORK_LIB_MAJOR 0
#define DFRAMEWORK_LIB_MINOR 1
#define DFRAMEWORK_LIB_PATCH 4


#if defined(_WIN32) || defined(_WIN64)
#   define DFW_OS_WINDOWS 1
#   define DFW_DLL_EXPORT __declspec(dllexport)
#   define DFW_DLL_IMPORT __declspec(dllimport)
#else
#   define DFW_OS_LINUX   1
#   define DFW_DLL_EXPORT
#   define DFW_DLL_IMPORT
#endif

#if !defined(NULL)
#   define     NULL    ((void*)0)
#endif

#define DFW_UNUSED(x) (void)(x)

#ifdef DFW_ABORT
#   undef DFW_ABORT
#endif

#define DFW_ABORT(x) {  \
    if(x) {             \
        printf("ABORT:: %s:%d\r\n", __FILE__,__LINE__); \
        ::abort();      \
    }                   \
}

#define DFW_FREE(x) {   \
    if(x) {             \
        ::free(x);      \
        x = NULL;       \
    }                   \
}

#define DFW_DELETE(x) { \
    if(x) {             \
        delete x;       \
        x = NULL;       \
    }                   \
}

#define DFW_FILE_INIT(x) (x = -1)
#define DFW_FILE_ENABLE(x) (!(x == -1))

#define DFW_FILE_CLOSE(x) { \
    if(x!=-1) {             \
        ::close(x);         \
        x = -1;             \
    }                       \
}

#define DFW_SOCK_INIT(x) (x = -1)
#define DFW_SOCK_ENABLE(x) (!(x == -1))

#ifdef _WIN32
# define DFW_SOCK_CLOSE(x) { \
    if(x!=-1) {              \
        ::closesocket(x);    \
        x = -1;              \
    }                        \
}
#else
# define DFW_SOCK_CLOSE(x) { \
    if(x!=-1) {              \
        ::close(x);          \
        x = -1;              \
    }                        \
}
#endif

#define DFW_STATIC
#define DFW_PRIVATE
#define DFW_STATIC_PRIVATE

#define DFW_IN
#define DFW_OUT


typedef unsigned char      dfw_byte_t;
typedef unsigned short     dfw_word_t;
typedef unsigned int       dfw_uint_t;
typedef unsigned long      dfw_ulong_t;
typedef unsigned long      dfw_size_t;
typedef void*              dfw_point_t;
typedef unsigned long      dfw_tid_t;
typedef unsigned long long dfw_uint64_t;
typedef dfw_uint64_t       dfw_time_t;

enum dfw_compare_t {
    DFW_COMPARE_LT = -1,
    DFW_COMPARE_EQ = 0,
    DFW_COMPARE_GT = 1,
};


#define DFW_BYTE4TOINT( str ) \
     ( ((dfw_byte_t)str[0]<<24) | ((dfw_byte_t)str[1]<<16) | ((dfw_byte_t)str[2]<<8) | ((dfw_byte_t)str[3]) )
#define DFW_BYTE2TOINT( str ) ( ((dfw_byte_t)str[0]<<8) | ((dfw_byte_t(str[1]) )

#define DFW_INTTOBYTE4( dest, i ) { \
    dest[3] = (i&0xff);             \
    dest[2] = ((i>>8)&0xff);        \
    dest[1] = ((i>>16)&0xff);       \
    dest[0] = ((i>>24)&0xff);       \
    }
#define DFW_INTTOBYTE2( dest, i ) { \
    dest[1] = (i&0xff);             \
    dest[0] = ((i>>8)&0xff);        \
    }


#endif /* DFRAMEWORK_BASE_TYPE_H */

