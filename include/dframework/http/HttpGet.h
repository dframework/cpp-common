#ifndef DFRAMEWORK_HTTP_HTTPGET_H
#define DFRAMEWORK_HTTP_HTTPGET_H

#include <dframework/http/HttpClient.h>

#ifdef __cplusplus
namespace dframework {

    class HttpGet : public HttpQuery 
    {
    public:
        HttpGet();
        virtual ~HttpGet();

        virtual sp<Retval> query(const URI& uri, const char* method=NULL);
        virtual sp<Retval> query(const char* uri, const char* method=NULL);
    };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPGET_H */

