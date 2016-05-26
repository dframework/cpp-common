#ifndef DFRAMEWORK_BASE_RETVAL_H
#define DFRAMEWORK_BASE_RETVAL_H
#define DFRAMEWORK_BASE_RETVAL_VERSION 1

#ifdef __APPLE__
# include "dframework/lang/String.h"
#else
# include <dframework/lang/String.h>
#endif

#define DFW_RETVAL_NEW(rno, eno) \
    Retval::get((rno), (eno), __FILE__,__LINE__,__func__)

#define DFW_RETVAL_NEW_MSG(rno, eno, fmt, args...)  \
    Retval::get((rno), (eno), __FILE__,__LINE__,__func__,fmt,##args)

#define DFW_RETVAL_NEWX(x) \
    Retval::get(x.m_retno, x.m_errno, __FILE__,__LINE__,__func__)

#define DFW_RETVAL_NEWX_MSG(x, fmt, args...) \
    Retval::get(x.m_retno, x.m_errno, __FILE__,__LINE__,__func__,fmt,##args)

#define DFW_RET_ADD(r)   (r->addStack(__FILE__,__LINE__,__func__))
#define DFW_RETVAL_H(r)  (r.has())
#define DFW_RETVAL_V(r)  (r.has() ? r->value():0)
#define DFW_RETVAL_C(r)  (r.has() ? DFW_RET_ADD(r):NULL)
#define DFW_RETVAL_D(r)  DFW_RET_ADD(r)

#define DFW_RETVAL_D_SET_VAL(r,v) \
    (r->setValue(v)->addStack(__FILE__,__LINE__,__func__))
#define DFW_RETVAL_D_SET_ERR(r,e) \
    (r->setError(e)->addStack(__FILE__,__LINE__,__func__))
#define DFW_RETVAL_D_SET(r,v,e) \
    (r->set(v,e)->addStack(__FILE__,__LINE__,__func__))

#define DFW_RET(r,e)     (!!(r = e))
#define DFW_RET_C(r,e)   (DFW_RET(r,e) ? DFW_RET_ADD(r):NULL)

#define DFW_RETVAL_HASX(x) (x.m_retno)

#define DFW_OK_LOCAL  {DFW_OK, 0}

// ------------------------------------------------------------------

#if defined(__MINGW32__) || defined(__MSYS__) || defined(_WIN32)
#   define DFW_ERRNO_EINPROGRESS     WSAENOTCONN
#   define DFW_ERRNO_EAGAIN          WSAEWOULDBLOCK
#else
#   define DFW_ERRNO_EINPROGRESS     EINPROGRESS
#   define DFW_ERRNO_EAGAIN          EAGAIN
#endif

// ------------------------------------------------------------------

typedef int   dfw_errno_t;

// ------------------------------------------------------------------

enum dfw_retno_t {
    DFW_OK               =    0,
    DFW_T_COMPLETE       =    1,
    DFW_T_CHUNKED_END    =    2,
    DFW_T_RECONNECT      =    3,
    DFW_T_READY          =    4,
    DFW_T_RUN            =    5,
    DFW_T_STOP           =    6,
    DFW_T_CANCLE         =    7,
    DFW_T_ETC            =    8,

    DFW_ERROR            = 2001,
    DFW_E_INTR           = 2002,
    DFW_E_AGAIN          = 2003,
    DFW_E_TIMEOUT        = 2004,
    DFW_E_INVAL          = 2005,
    DFW_E_NOMEM          = 2006,
    DFW_E_DEADLK         = 2007,
    DFW_E_BUSY           = 2008,
    DFW_E_PERM           = 2009,
    DFW_E_ACCES          = 2010,
    DFW_E_BADF           = 2011,
    DFW_E_FAULT          = 2012,
    DFW_E_LOOP           = 2013,
    DFW_E_NAMETOOLONG    = 2014,
    DFW_E_NOENT          = 2015,
    DFW_E_NOTDIR         = 2016,
    DFW_E_OVERFLOW       = 2017,
    DFW_E_MFILE          = 2018,
    DFW_E_NFILE          = 2019,
    DFW_E_NOSPC          = 2020,
    DFW_E_AFNOSUPPORT    = 2021,
    DFW_E_PROTONOSUPPORT = 2022,
    DFW_E_NOPROTOOPT     = 2023,
    DFW_E_NOTSOCK        = 2024,
    DFW_E_NOLCK          = 2025,
    DFW_E_ADDRINUSE      = 2026,
    DFW_E_ALREADY        = 2027,
    DFW_E_CONNREFUSED    = 2028,
    DFW_E_ISCONN         = 2029,
    DFW_E_NETUNREACH     = 2030,
    DFW_E_DESTADDRREQ    = 2031,
    DFW_E_FBIG           = 2032,
    DFW_E_PIPE           = 2033,
    DFW_E_IO             = 2034,
    DFW_E_ISDIR          = 2035,
    DFW_E_ILSEQ          = 2036,
    DFW_E_RANGE          = 2037,
    DFW_E_EXIST          = 2038,
    DFW_E_NODEV          = 2039,
    DFW_E_NXIO           = 2040,
    DFW_E_ROFS           = 2041,
    DFW_E_TXTBSY         = 2042,
    //DFW_E_WOULDBLOCK     = 2043,
    DFW_E_NOTBLK         = 2044,
    DFW_E_NOTCONN        = 2046,
    DFW_E_PROTO          = 2047,
    DFW_E_NOSYS          = 2048,
    DFW_E_SRCH           = 2049,
    DFW_E_NOTTY          = 2050,
 
    DFW_E_NOSUPPORT      = 2201,
    DFW_E_ADDRFAMILY     = 2202,
    DFW_E_BADFLAGS       = 2203,
    DFW_E_FAIL           = 2204,
    DFW_E_FAMILY         = 2205,
    DFW_E_NODATA         = 2206,
    DFW_E_NONAME         = 2207,
    DFW_E_SERVICE        = 2208,
    DFW_E_SOCKTYPE       = 2209,
    DFW_E_SYSTEM         = 2210,
    DFW_E_ADDRTYPE       = 2211,
    DFW_E_OPNOTSUPP      = 2212,
    DFW_E_HANDLE         = 2213,
    DFW_E_INPROGRESS     = 2214,
    DFW_E_EALREADY       = 2215,
    DFW_E_MSGSIZE        = 2216,

    DFW_E_THREAD_STATUS       = 3101,
    DFW_E_STAT                = 3110,
    DFW_E_OPEN                = 3120,
    DFW_E_OPENDIR             = 3121,
    DFW_E_NOT_CACHE_HOST      = 3130,
    DFW_E_GETADDRINFO         = 3131,
    DFW_E_HOST_NOT_FOUND      = 3132,
    DFW_E_NO_RECOVERY         = 3133,
    DFW_E_GETHOSTBYNAME       = 3134,
    DFW_E_REGEXP_NOT_PATTERN  = 3140,
    DFW_E_REGEXP_NOT_MATCH    = 3141,
    DFW_E_NO_HTTP_AUTH_TYPE   = 3150,
    DFW_E_NO_WWW_AUTHENTICATE = 3151,
    DFW_E_HTTP_CHUNKED        = 3152,
    DFW_E_CONNECT             = 3153,
    DFW_E_DISCONNECT          = 3154,
    DFW_E_MAX_REQUESTCOUNT    = 3155,
    DFW_E_HTTPAUTH            = 3156,
    DFW_E_HTTP_REQUEST        = 3157,
    DFW_E_NOTFINDLOCATION     = 3158,
    DFW_E_HOSTNAME            = 3159,
    DFW_E_SOCKET              = 3160,
    DFW_E_SETSOCKOPT          = 3161,
    DFW_E_GETSOCKOPT          = 3162,
    DFW_E_FCNTL               = 3163,
    DFW_E_POLL                = 3164,
    DFW_E_POLLERR             = 3165,
    DFW_E_POLLHUP             = 3166,
    DFW_E_POLLNVAL            = 3167,
    DFW_E_SELECT              = 3168,
    DFW_E_WRITE               = 3169,
    DFW_E_RECV                = 3170,
    DFW_E_SSCANF              = 3171,
    DFW_E_MOUNT               = 3172,
    DFW_E_UMOUNT              = 3173,
    DFW_E_UMOUNT2             = 3174,
    DFW_E_WRITEV              = 3175,
    DFW_E_READ                = 3176,
    DFW_E_LISTEN              = 3177,
    DFW_E_BIND                = 3178,
    DFW_E_ACCEPT              = 3179,
    DFW_E_CONNRESET           = 3180,

    DFW_E_NOLOGGER            = 3200,

};

typedef struct dfw_retval_t dfw_retval_t;

struct dfw_retval_t {
    dfw_retno_t m_retno;
    dfw_errno_t m_errno;
};

#ifdef __cplusplus
namespace dframework {

    class RetContext : public Object
    {
    public:
        inline RetContext() {}
        inline virtual ~RetContext() {}
    };

    class Retval : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(Retval);

        class Stack {
        public:
            char  *m_pFile;
            char  *m_pFunc;
            int    m_iLine;
            Stack *m_pNext;

            Stack(const char* file, int line, const char* func);
            virtual ~Stack();
        };

    private:
        dfw_retno_t     m_iValue;
        dfw_errno_t     m_iErrno;
        String          m_sMessage;
        Stack          *m_pStack;
        Stack          *m_pLastStack;
        sp<RetContext>  m_pContext;

    private:
        Retval(dfw_retno_t val, dfw_errno_t errnum
             , const char* file, int line, const char* func);
        inline void setMessage(const String& msg) { m_sMessage = msg; }
    public:
        virtual ~Retval();

        sp<Retval> addStack(const char* file, int line, const char* func);
        sp<Retval> setValue(dfw_retno_t v);
        sp<Retval> setError(dfw_errno_t v);
        sp<Retval> set(dfw_retno_t r, dfw_errno_t e);
        String dump();

        static dfw_retno_t retno(dfw_errno_t errnum);

        inline static sp<Retval> get(dfw_retno_t retno
                            , dfw_errno_t errnum
                            , const char* file
                            , int line
                            , const char* func){
            return new Retval(retno, errnum, file, line, func);
        }
        static sp<Retval> get(dfw_retno_t retno
                            , dfw_errno_t errnum
                            , const char *file
                            , int line
                            , const char *func
                            , const char *msgfmt, ...);
        inline dfw_retno_t value() const { return m_iValue; }
        inline dfw_errno_t error() const { return m_iErrno; }
        inline const String& message()   const { return m_sMessage; }
        inline const Stack* stack() const { return m_pStack; }

        inline bool iserror() const {
            return ((0<=m_iValue) && (m_iValue<1000));
        }

        inline sp<RetContext> getContext() const { return m_pContext; }
        inline void setContext(sp<RetContext> p) { m_pContext = p; }

        static const char* errno_short(
                  dfw_retno_t *rno, int eno, const char* default_msg);
        static const char* wsa2value(dfw_retno_t *rno, int wsa
                           , const char* default_msg);
    };

}; // end namespace dframework
#endif // end if cplusplus

#endif /* DFRAMEWORK_BASE_RETVAL_H */

