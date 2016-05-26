#ifndef DFRAMEWORK_HTTP_HTTPPOST_H
#define DFRAMEWORK_HTTP_HTTPPOST_H

#include <dframework/http/HttpClient.h>

#ifdef __cplusplus
namespace dframework {

    class HttpPost : public HttpQuery 
    {
    public:
        HttpPost();
        virtual ~HttpPost();

        virtual sp<Retval> query(const URI& uri, const char* method=NULL);
        virtual sp<Retval> query(const char* uri, const char* method=NULL);

    };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPPOST_H */

