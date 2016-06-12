#include <dframework/util/Time.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#ifdef WIN32
#include <windows.h> /* for TIME_ZONE_INFORMATION */
#endif

/* ISO8601: 2001-01-01T12:30:00Z */
#define ISO8601_FORMAT_Z "%04d-%02d-%02dT%02d:%02d:%lfZ"
#define ISO8601_FORMAT_M "%04d-%02d-%02dT%02d:%02d:%lf-%02d:%02d"
#define ISO8601_FORMAT_P "%04d-%02d-%02dT%02d:%02d:%lf+%02d:%02d"

/* RFC1123: Sun, 06 Nov 1994 08:49:37 GMT */
#define RFC1123_FORMAT "%3s, %02d %3s %4d %02d:%02d:%02d GMT"
/* RFC850:  Sunday, 06-Nov-94 08:49:37 GMT */
#define RFC1036_FORMAT "%10s %2d-%3s-%2d %2d:%2d:%2d GMT"
/* asctime: Wed Jun 30 21:49:08 1993 */
#define ASCTIME_FORMAT "%3s %3s %2d %2d:%2d:%2d %4d"

#if defined(HAVE_STRUCT_TM_TM_GMTOFF)
#   define GMTOFF(t) ((t).tm_gmtoff)
#elif defined(HAVE_STRUCT_TM___TM_GMTOFF)
#   define GMTOFF(t) ((t).__tm_gmtoff)
#elif defined(WIN32)
#   define GMTOFF(t) (gmt_to_local_win32())
#elif defined(HAVE_TIMEZONE)
#   define GMTOFF(t) (-timezone + ((t).tm_isdst > 0 ? 3600 : 0))
#else
#   define GMTOFF(t) (0)
#endif


namespace dframework {

#ifdef WIN32

#ifdef __cplusplus
extern "C" {
#endif

time_t gmt_to_local_win32(void);

#ifdef __cplusplus
};
#endif

time_t gmt_to_local_win32(void)
{
    TIME_ZONE_INFORMATION tzinfo;
    DWORD dwStandardDaylight;
    long bias;

    dwStandardDaylight = GetTimeZoneInformation(&tzinfo);
    bias = tzinfo.Bias;

    if (dwStandardDaylight == TIME_ZONE_ID_STANDARD)
        bias += tzinfo.StandardBias;
    
    if (dwStandardDaylight == TIME_ZONE_ID_DAYLIGHT)
        bias += tzinfo.DaylightBias;
    
    return (- bias * 60);
}
#endif

    const char Time::WEEK_DAYS[7][4] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    const char Time::SHORT_MONTHS[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    Time::Time(dfw_timetype_t type)
            : Object()
    {
        get(0, type);
    }

    Time::Time(dfw_time_t t, dfw_timetype_t type)
            : Object()
    {
        get(t, type);
    }

    Time::Time(const Time& time)
            : Object()
    {
        m_iTimestamp = time.m_iTimestamp;
        m_iTimeType  = time.m_iTimeType;
        m_sec   = time.m_sec;
        m_min   = time.m_min;
        m_hour  = time.m_hour;
        m_mday  = time.m_mday;
        m_mon   = time.m_mon;
        m_year  = time.m_year;
        m_wday  = time.m_wday;
        m_yday  = time.m_yday;
        m_isdst = time.m_isdst;
#ifndef WIN32
        m_gmtoff= time.m_gmtoff;
        m_zone  = time.m_zone;
#endif
    }

    Time::~Time(){}

    sp<Retval> Time::get(dfw_time_t t, dfw_timetype_t type){
        time_t rt;
        if(t==0)
            ::time(&rt);
        else
            rt = (time_t)t;

        struct tm test_tm;
        ::memset(&test_tm, 0, sizeof(struct tm));

        m_iTimestamp = t;
        m_iTimeType = type;

        switch(type){
        case DFW_TIMETYPE_GM :
            ::gmtime_r(&rt, &test_tm);
            break;
        case DFW_TIMETYPE_LOCAL :
            ::localtime_r(&rt, &test_tm);
            break;
        default:
            return DFW_RETVAL_NEW(DFW_ERROR, 0);
        }

        m_sec   = test_tm.tm_sec;
        m_min   = test_tm.tm_min;
        m_hour  = test_tm.tm_hour;
        m_mday  = test_tm.tm_mday;
        m_mon   = test_tm.tm_mon;
        m_year  = test_tm.tm_year;
        m_wday  = test_tm.tm_wday;
        m_yday  = test_tm.tm_yday;
        m_isdst = test_tm.tm_isdst;
#ifndef WIN32
        m_gmtoff= test_tm.tm_gmtoff;
        m_zone  = test_tm.tm_zone;
#endif

        return NULL;
    }

    String Time::format(const char *fmt){
        struct tm test_tm;
        ::memset(&test_tm, 0, sizeof(struct tm));

        test_tm.tm_sec = m_sec;
        test_tm.tm_min = m_min;
        test_tm.tm_hour = m_hour;
        test_tm.tm_mday = m_mday;
        test_tm.tm_mon = m_mon;
        test_tm.tm_year = m_year;
        test_tm.tm_wday = m_wday;
        test_tm.tm_yday = m_yday;
        test_tm.tm_isdst = m_isdst;
#ifndef WIN32
        test_tm.tm_gmtoff = m_gmtoff;
        test_tm.tm_zone = (char*)m_zone.toChars();
#endif

        char buf[1024];
        ::strftime(buf, sizeof(buf), fmt, &test_tm);
        String r = buf;
#ifndef WIN32
        test_tm.tm_zone = NULL;
#endif
        return r;
    }

    /* static */
    String Time::format(const char* fmt, dfw_time_t in){
        Time t(in);
        return t.format(fmt);
    }

    /* static */
    sp<Retval> Time::currentTimeMillis(dfw_timeval *out_tv){
        if(!out_tv) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        struct timeval tv;
        if( -1 != ::gettimeofday(&tv, NULL)){
            out_tv->sec = tv.tv_sec;
            out_tv->usec = tv.tv_usec;
            return NULL;
        }

        int eno = errno;
        switch(eno){
        case EFAULT:
            return DFW_RETVAL_NEW(DFW_E_FAULT, eno);
        case EINVAL:
            return DFW_RETVAL_NEW(DFW_E_INVAL, eno);
        case EPERM:
            return DFW_RETVAL_NEW(DFW_E_PERM, eno);
        }
        return DFW_RETVAL_NEW(DFW_E_PERM, eno);
    }

    /* static */ 
    dfw_time_t Time::currentTimeMillis(){
        dfw_timeval val;
        sp<Retval> retval = currentTimeMillis(&val);
        if(DFW_RETVAL_H(retval)) return -1;
        return (val.sec*1000) + (val.usec/1000);
    }

    /* static */
    sp<Retval> Time::iso8601(dfw_time_t *out, const char* date)
    {
        struct tm gmt;
        ::memset(&gmt, 0, sizeof(struct tm));
        int off_hour, off_min;
        double sec;
        off_t fix;
        int n;

        if( !out ) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        if ((n = sscanf(date, ISO8601_FORMAT_P,
                    &gmt.tm_year, &gmt.tm_mon, &gmt.tm_mday,
                    &gmt.tm_hour, &gmt.tm_min, &sec,
                    &off_hour, &off_min)) == 8) {
            gmt.tm_sec = (int)sec;
            fix = - off_hour * 3600 - off_min * 60;
        }else if ((n = sscanf(date, ISO8601_FORMAT_M,
                    &gmt.tm_year, &gmt.tm_mon, &gmt.tm_mday,
                    &gmt.tm_hour, &gmt.tm_min, &sec,
                    &off_hour, &off_min)) == 8) {
            gmt.tm_sec = (int)sec;
            fix = off_hour * 3600 + off_min * 60;
        }else if ((n = sscanf(date, ISO8601_FORMAT_Z,
                    &gmt.tm_year, &gmt.tm_mon, &gmt.tm_mday,
                    &gmt.tm_hour, &gmt.tm_min, &sec)) == 6) {
            gmt.tm_sec = (int)sec;
            fix = 0;
        }else{
            int eno = errno;  
            switch(eno){
            /*case EAGAIN:
                return DFW_RETVAL_NEW(DFW_E_AGAIN, eno);
            case EBADF:
                return DFW_RETVAL_NEW(DFW_E_BADF, eno);
            case EILSEQ:
                return DFW_RETVAL_NEW(DFW_E_ILSEQ, eno);
            case EINTR:
                return DFW_RETVAL_NEW(DFW_E_INTR, eno);*/
            case EINVAL:
                return DFW_RETVAL_NEW(DFW_E_INVAL, eno);
            case ENOMEM:
                return DFW_RETVAL_NEW(DFW_E_NOMEM, eno);
            case ERANGE:
                return DFW_RETVAL_NEW(DFW_E_RANGE, eno);
            }
            return DFW_RETVAL_NEW(DFW_E_SSCANF, eno);
        }

        gmt.tm_year -= 1900;
        gmt.tm_isdst = -1;
        gmt.tm_mon--;

        *out = ((mktime(&gmt) + fix) + GMTOFF(gmt));

        return NULL;
    }

    /* static */
    sp<Retval> Time::rfc1123(dfw_time_t *out, const char *date){
        struct tm gmt;
        ::memset(&gmt, 0, sizeof(struct tm));
        char wkday[4], mon[4];
        int n;
        time_t result;

        if(!out) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        /*  it goes: Sun, 06 Nov 1994 08:49:37 GMT */
        n = ::sscanf(date, RFC1123_FORMAT,
            wkday, &gmt.tm_mday, mon, &gmt.tm_year, &gmt.tm_hour,
            &gmt.tm_min, &gmt.tm_sec);
        if( n!=7 ) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        gmt.tm_year -= 1900;
        for (n=0; n<12; n++){
            if (::strcmp(mon, Time::SHORT_MONTHS[n]) == 0)
                break;
        }
        gmt.tm_mon = n;
        gmt.tm_isdst = -1;
        result = ::mktime(&gmt);
        result += GMTOFF(gmt);

        *out = result;
        if(-1==result) return DFW_RETVAL_NEW(DFW_ERROR, 0);
        return NULL;
    }

    /* static */
    sp<Retval> Time::rfc1036(dfw_time_t *out, const char *date){
        struct tm gmt;
        ::memset(&gmt, 0, sizeof(struct tm));
        int n;
        char wkday[11], mon[4];
        time_t result;

        if(!out) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        /* RFC850/1036 style dates: Sunday, 06-Nov-94 08:49:37 GMT */
        n = ::sscanf(date, RFC1036_FORMAT,
                wkday, &gmt.tm_mday, mon, &gmt.tm_year,
                &gmt.tm_hour, &gmt.tm_min, &gmt.tm_sec);
        if( n!=7 ) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        for (n=0; n<12; n++){
            if (::strcmp(mon, Time::SHORT_MONTHS[n]) == 0)
                break;
        }

        /* Defeat Y2K bug. */
        if (gmt.tm_year < 50)
            gmt.tm_year += 100;

        gmt.tm_mon = n;
        gmt.tm_isdst = -1;
        result = ::mktime(&gmt);
        result += GMTOFF(gmt);

        *out = result;
        if(-1==result) return DFW_RETVAL_NEW(DFW_ERROR, 0);
        return NULL;
    }

    sp<Retval> Time::asctime(dfw_time_t *out, const char *date){
        struct tm gmt;
        ::memset(&gmt, 0, sizeof(struct tm));
        int n;
        char wkday[4], mon[4];
        time_t result;

        if(!out) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        n = ::sscanf(date, ASCTIME_FORMAT,
                wkday, mon, &gmt.tm_mday,
                &gmt.tm_hour, &gmt.tm_min, &gmt.tm_sec,
                &gmt.tm_year);
        if( n!=7 ) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        for (n=0; n<12; n++){
            if (::strcmp(mon, Time::SHORT_MONTHS[n]) == 0)
                break;
        }

        gmt.tm_mon = n;
        gmt.tm_isdst = -1;
        result = ::mktime(&gmt);
        result += GMTOFF(gmt);

        *out = result;
        if(-1==result) return DFW_RETVAL_NEW(DFW_ERROR, 0);
        return NULL;
    }

    /* static */
    sp<Retval> Time::parse(dfw_time_t *out, const char *date){
        sp<Retval> retval;

        if(!out) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        if(!DFW_RET(retval, rfc1123(out, date)))
            return NULL;
        if(!DFW_RET(retval, rfc1036(out, date)))
            return NULL;
        if(!DFW_RET(retval, asctime(out, date)))
            return NULL;

        return DFW_RETVAL_D(retval);
    }

    void Time::getDateStringRfc822(String& out, dfw_time_t time){
#define DFW_RFC822_DATE_LEN (30)
        char local_str[DFW_RFC822_DATE_LEN+1];
        char* date_str = local_str;
        ::memset(local_str, 0, DFW_RFC822_DATE_LEN+1);

        Time tm;
        tm.get(time);
        const char* s;

        s = &Time::WEEK_DAYS[tm.m_wday][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ',';
        *date_str++ = ' ';
        *date_str++ = tm.m_mday / 10 + '0';
        *date_str++ = tm.m_mday % 10 + '0';
        *date_str++ = ' ';
        s = &Time::SHORT_MONTHS[tm.m_mon][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ' ';
        int real_year = 1900 + tm.m_year;
        /* This routine isn't y10k ready. */
        *date_str++ = real_year / 1000 + '0';
        *date_str++ = real_year % 1000 / 100 + '0';
        *date_str++ = real_year % 100 / 10 + '0';
        *date_str++ = real_year % 10 + '0';
        *date_str++ = ' ';
        *date_str++ = tm.m_hour / 10 + '0';
        *date_str++ = tm.m_hour % 10 + '0';
        *date_str++ = ':';
        *date_str++ = tm.m_min / 10 + '0';
        *date_str++ = tm.m_min % 10 + '0';
        *date_str++ = ':';
        *date_str++ = tm.m_sec / 10 + '0';
        *date_str++ = tm.m_sec % 10 + '0';
        *date_str++ = ' ';
        *date_str++ = 'G';
        *date_str++ = 'M';
        *date_str++ = 'T';
        *date_str++ = 0;
        out = local_str;
    }

};

