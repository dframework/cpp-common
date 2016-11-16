#ifndef DFRAMEWORK_HTTPD_HTTPDUTIL_H
#define DFRAMEWORK_HTTPD_HTTPDUTIL_H

#include <dframework/base/Retval.h>
#include <dframework/util/Time.h>
#include <dframework/http/HttpHeader.h>
#include <dframework/httpd/HttpRequest.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdUtil : public Object
    {
    public:
        static int toLower(int c);
        static sp<Retval> urldecode(String& out, const char *url);

        static bool isDateMask(const char *data, const char *mask);
        static sp<Retval> expget(time_t *t, Time *xt);
        static sp<Retval> parseToTime(time_t* out, const char* date);

        static sp<Retval> checkRangeBytes(
                              HttpRequest* request
                            , String& sRange, String& sAcceptRange
                            , String& sStart, String& sEnd, int* minus);
                            //, char** pStart, char** pEnd, int* minus);

    public:
        HttpdUtil();
        virtual ~HttpdUtil();

    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDUTIL_H */

