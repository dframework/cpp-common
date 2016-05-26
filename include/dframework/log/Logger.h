#ifndef DFRAMEWORK_LOG_LOGGER_H
#define DFRAMEWORK_LOG_LOGGER_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>

#define DFWLOG_INFO    1 // 001
#define DFWLOG_LOG     2 // 010
#define DFWLOG_DEBUG   3 // 011
#define DFWLOG_WARNING 4 // 100
#define DFWLOG_ERROR   5 // 101
#define DFWLOG_FATAL   7 // 111

#define DFWLOG_I       DFWLOG_INFO
#define DFWLOG_L       DFWLOG_LOG
#define DFWLOG_D       DFWLOG_DEBUG
#define DFWLOG_W       DFWLOG_WARNING
#define DFWLOG_E       DFWLOG_ERROR
#define DFWLOG_F       DFWLOG_FATAL

// ------------------------------------------------------------------

#define DFWLOG_GET_LEVEL(x)  ( x & 7 )
#define DFWLOG_GET_TYPE(x)   ( ( x >> 3 ) & 7 )  // [xx 000 xxx]
#define DFWLOG_GET_ID(x)     ((uint64_t)(x>>6))  // [00 xxx xxx]

#define DFWLOG_TYPE(x)       ( x << 3 )          // 0 ~ 7
#define DFWLOG_ID(x)         ( (uint64_t)(x << 6) )
#define DFWLOG_USERID(x)     ( (uint64_t)((x+1024) << 6) )
#define DFWLOG_GET_USERID(x) ( x+1024 )

#define DFWLOG_SYSLOG       DFWLOG_TYPE(1)
#define DFWLOG_ALL          DFWLOG_TYPE(2)
#define DFWLOG_USER         DFWLOG_TYPE(3)

#define DFWLOG_S            DFWLOG_SYSLOG
#define DFWLOG_A            DFWLOG_ALL
#define DFWLOG_U            DFWLOG_USER

// ------------------------------------------------------------------

#define DFWLOG_COMMON_ID    1
#define DFWLOG_FSCORE_ID    2
#define DFWLOG_SERVER_ID    3
#define DFWLOG_HTTPD_ID     4

// ------------------------------------------------------------------

#define DFWLOG_RET(x)        (x.has()?x.get():NULL)
#define DFWLOG_DFT()         __FILE__,__LINE__,__func__
#define DFWLOG_DEC(t)        DFWLOG_DFT(),(t),NULL, NULL
#define DFWLOG_DEC_C(t,c)    DFWLOG_DFT(),(t),(c) , NULL
#define DFWLOG_DEC_CR(t,c,r) DFWLOG_DFT(),(t),(c) , DFWLOG_RET(r)

// ------------------------------------------------------------------

#define DFWLOG(type,fmt,args...) \
    Logger::log(DFWLOG_DEC(type), fmt,##args)

#define DFWLOG_C(type,ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(type,ctx), fmt,##args)

#define DFWLOG_R(type,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(type,NULL,retval), fmt,##args)

#define DFWLOG_CR(type,ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(type,ctx ,retval), fmt,##args)

// ------------------------------------------------------------------

#define DFW_INFO(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_I), fmt,##args)

#define DFW_LOG(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_L), fmt,##args)

#define DFW_DEBUG(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_D), fmt,##args)

#define DFW_WARNING(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_W), fmt,##args)

#define DFW_ERROR(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_E), fmt,##args)

#define DFW_FATAL(fmt,args...) \
    Logger::log(DFWLOG_DEC(DFWLOG_F), fmt,##args)

// ------------------------------------------------------------------

#define DFW_INFO_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_I,ctx), fmt,##args)

#define DFW_LOG_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_L,ctx), fmt,##args)

#define DFW_DEBUG_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_D,ctx), fmt,##args)

#define DFW_WARNING_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_W,ctx), fmt,##args)

#define DFW_ERROR_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_E,ctx), fmt,##args)

#define DFW_FATAL_C(ctx,fmt,args...) \
    Logger::log(DFWLOG_DEC_C(DFWLOG_F,ctx), fmt,##args)

// ------------------------------------------------------------------

#define DFW_INFO_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_I,NULL,retval), fmt,##args)

#define DFW_LOG_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_L,NULL,retval), fmt,##args)

#define DFW_DEBUG_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_D,NULL,retval), fmt,##args)

#define DFW_WARNING_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_R,NULL,retval), fmt,##args)

#define DFW_ERROR_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_E,NULL,retval), fmt,##args)

#define DFW_FATAL_R(retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_F,NULL,retval), fmt,##args)

// ------------------------------------------------------------------

#define DFW_INFO_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_I,ctx,retval), fmt,##args)

#define DFW_LOG_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_L,ctx,retval), fmt,##args)

#define DFW_DEBUG_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_D,ctx,retval), fmt,##args)

#define DFW_WARNING_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_R,ctx,retval), fmt,##args)

#define DFW_ERROR_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_E,ctx,retval), fmt,##args)

#define DFW_FATAL_CR(ctx,retval,fmt,args...) \
    Logger::log(DFWLOG_DEC_CR(DFWLOG_F,ctx,retval), fmt,##args)

// ------------------------------------------------------------------

#ifdef __cplusplus
namespace dframework {

    class Logger : public Object
    {
    private:
        static Array<Logger> m_aLoggers;

    public:
        Logger();
        virtual ~Logger();

        static sp<Retval> addLogger(sp<Logger>& logger);

        static void syslog(int level, const char* fmt, ...);

        static sp<Retval> log(const char* file, unsigned line
                            , const char* func
                            , uint64_t type
                            , dfw_point_t ctx
                            , Retval* inret, const char* fmt, ...);

        inline virtual void log(const char* file, unsigned line
                            , const char* func
                            , unsigned level, unsigned type, uint64_t id
                            , dfw_point_t ctx
                            , Retval* inret, const char* buf){
            DFW_UNUSED(file);
            DFW_UNUSED(line);
            DFW_UNUSED(func);
            DFW_UNUSED(level);
            DFW_UNUSED(type);
            DFW_UNUSED(id);
            DFW_UNUSED(ctx);
            DFW_UNUSED(inret);
            DFW_UNUSED(buf);
        }

        inline virtual bool enableId(uint64_t id) {
            return ( 0==id ? true : false);
        }

    };

};
#endif

#endif /* DFRAMEWORK_LOG_LOGGER_H */

