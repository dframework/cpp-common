#ifndef DFRAMEWORK_HTTP_HTTPAUTH_H
#define DFRAMEWORK_HTTP_HTTPAUTH_H


#include <dframework/base/Retval.h>


enum dfw_httpauthtype_t {
    DFW_HTTPAUTHTYPE_UNKNOWN = 0,
    DFW_HTTPAUTHTYPE_BASIC   = 1,
    DFW_HTTPAUTHTYPE_DIGEST  = 2,
};


#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpQuery;

    class HttpAuth
    {
    private:
        dfw_httpauthtype_t getType(const char *p);

    public:
        HttpAuth();
        virtual ~HttpAuth();

        sp<Retval> auth(sp<HttpConnection>& conn, sp<HttpQuery>& pQ);
    };

};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPAUTH_H */

