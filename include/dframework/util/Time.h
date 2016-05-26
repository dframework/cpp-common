#ifndef DFRAMEWORK_UTIL_TIME_H
#define DFRAMEWORK_UTIL_TIME_H
#define DFRAMEWORK_UTIL_TIME_VERSION 1

#include <dframework/base/Retval.h>

enum dfw_timetype_t {
    DFW_TIMETYPE_LOCAL = 1,
    DFW_TIMETYPE_GM = 2,
};

struct dfw_timeval{
    dfw_uint64_t sec;
    dfw_uint64_t usec;
};
typedef struct dfw_timeval dfw_timeval;

#ifdef __cplusplus
namespace dframework {

    class Time : public Object
    {
    public:
        static const char WEEK_DAYS[7][4];
        static const char SHORT_MONTHS[12][4];

    private:
        dfw_time_t     m_iTimestamp;
        dfw_timetype_t m_iTimeType;

        int m_sec;
        int m_min;
        int m_hour;
        int m_mday;
        int m_mon;
        int m_year;
        int m_wday;
        int m_yday;
        int m_isdst;
        long m_gmtoff;
        String m_zone;

    public:
        Time(const Time& time);
        Time(dfw_timetype_t type=DFW_TIMETYPE_LOCAL);
        Time(dfw_time_t t, dfw_timetype_t type=DFW_TIMETYPE_LOCAL);
        virtual ~Time();

        sp<Retval> get(dfw_time_t t, dfw_timetype_t type=DFW_TIMETYPE_LOCAL);

        inline void setSecond(int sec) { m_sec = sec; }
        inline void setMinute(int min) { m_min = min; }
        inline void setHour(int hour) { m_hour = hour; }
        inline void setDay(int day) { m_mday = day; }
        inline void setMonth(int mon) { m_mon = mon; }
        inline void setYear(int year) { m_year = year; }
        inline void setWeek(int wday) { m_wday = wday; }
        inline void setYearDay(int yday) { m_yday = yday; }
        inline void setGmtoff(int gmtoff) { m_gmtoff = gmtoff; }

        inline int getSecond() const { return m_sec; }
        inline int getMinute() const { return m_min; }
        inline int getHour() const { return m_hour; }
        inline int getDay() const { return m_mday; }
        inline int getMonth() const { return m_mon; }
        inline int getYear() const { return m_year; }
        inline int getWeek() const { return m_wday; }
        inline int getYearDay() const { return m_yday; }
        inline int isDaylight() const { return m_isdst; }
        inline long getGmtOffset() const { return m_gmtoff; }
        inline const String& getTimezone() const { return m_zone; }
        inline int getGmtOffsetHour() const {
            return (int)((m_gmtoff/60)/60); 
        }
        inline int getGmtOffsetMin() const {
            return (int)((m_gmtoff/60)%60); 
        }

        String format(const char *fmt);

        static dfw_time_t currentTimeMillis();
        static sp<Retval> currentTimeMillis(dfw_timeval *out_tv);

        static sp<Retval> iso8601(dfw_time_t *out, const char* date);
        static sp<Retval> rfc1123(dfw_time_t *out, const char* date);
        static sp<Retval> rfc1036(dfw_time_t *out, const char* date);
        static sp<Retval> asctime(dfw_time_t *out, const char* date);
        static sp<Retval> parse(  dfw_time_t *out, const char* date);

        inline static sp<Retval> iso8601(dfw_time_t *o, const String& d){
            return iso8601(o, d.toChars());
        }
        inline static sp<Retval> rfc1123(dfw_time_t *o, const String& d){
            return rfc1123(o, d.toChars());
        }
        inline static sp<Retval> rfc1036(dfw_time_t *o, const String& d){
            return rfc1036(o, d.toChars());
        }
        inline static sp<Retval> asctime(dfw_time_t *o, const String& d){
            return asctime(o, d.toChars());
        }
        inline static sp<Retval> parse(  dfw_time_t *o, const String& d){
            return parse(o, d.toChars());
        }

        static void getDateStringRfc822(String& out, dfw_time_t time);

    };

};
#endif

#endif /* DFRAMEWORK_UTIL_TIME_H */

