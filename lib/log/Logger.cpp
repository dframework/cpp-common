#include <dframework/log/Logger.h>
#include <stdarg.h>
#include <ctype.h>
#ifndef _WIN32
#include <syslog.h>
#endif

namespace dframework {

    Array<Logger> Logger::m_aLoggers;

    Logger::Logger(){
    }

    Logger::~Logger(){
    }

    DFW_STATIC
    sp<Retval> Logger::addLogger(sp<Logger>& logger){
        AutoLock _l(&m_aLoggers);
        sp<Retval> retval;
        return DFW_RET_C(retval, m_aLoggers.insert(logger));
    }

    DFW_STATIC
    void Logger::syslog(int level, const char* fmt, ...){
#ifndef _WIN32
        char* buf = NULL;
        size_t bufsize;
        va_list ap, ap2;

        DFW_VALIST_SIZE(bufsize, temp, fmt, ap);
        DFW_VALIST_STRDUP(buf, bufsize, fmt, ap2);

        if( buf ){
            switch(level){
            case DFWLOG_W :
                ::syslog(LOG_WARNING, "%s", buf); 
                break;

            case DFWLOG_E :
            case DFWLOG_F :
                ::syslog(LOG_ERR, "%s", buf); 
                break;

            case DFWLOG_I :
            case DFWLOG_L :
            case DFWLOG_D :
            default : 
                ::syslog(LOG_INFO, "%s", buf);
                break;
            }
            DFW_FREE(buf);
        }
#endif
    }

    DFW_STATIC
    sp<Retval> Logger::log(const char* file, unsigned line
                         , const char* func
                         , uint64_t types
                         , dfw_point_t ctx
                         , Retval* inret, const char* fmt, ...)
    {
        sp<Retval> retval;
        int size;
        char* buf = NULL;
        unsigned level = DFWLOG_GET_LEVEL(types);
        unsigned type = DFWLOG_GET_TYPE(types);
        uint64_t id = DFWLOG_GET_ID(types);

        if(type==0) type = DFWLOG_USER;

        {
            AutoLock _l(&m_aLoggers);
            if( 0==(size=m_aLoggers.size()) )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR,0
                           ,"Not has Loggers.");
        }

        {
            size_t bufsize;
            va_list ap, ap2;
            DFW_VALIST_SIZE(bufsize, temp, fmt, ap);
            DFW_VALIST_STRDUP(buf, bufsize, fmt, ap2);
            if( !buf )
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
        }

        if( (type==DFWLOG_S) || (type==DFWLOG_A) ){
            String sysmsg = String::format("%s -- %s(id=%ld) at %s:%u"
                                         , buf, func, id, file, line);
            Logger::syslog(level, sysmsg.toChars());
        }

        bool isLogging = false;
        if( type!=DFWLOG_S ){
            for(int k=0; k<size; k++){
                sp<Logger> logger = m_aLoggers.get(k);
                if( !logger.has() ) continue;
                if( type == DFWLOG_A ){
                    isLogging = true;
                    logger->log(file, line, func, level, type, id
                          , ctx, inret, buf);
                }else if( (logger->enableId(id)) ){
                    isLogging = true;
                    logger->log(file, line, func, level, type, id
                          , ctx, inret, buf);
                    break;
                }
            }
        }

        DFW_FREE(buf);

        if( !isLogging ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not find logger. id=%u", id);
        }
        return NULL;
    }

};

