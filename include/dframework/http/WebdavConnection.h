#ifndef DFRAMEWORK_HTTP_WEBDAV_CONNECTION_H
#define DFRAMEWORK_HTTP_WEBDAV_CONNECTION_H

#include <dframework/base/Retval.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpDav.h>


#ifdef __cplusplus
namespace dframework {

    class WebdavConnection : 
            public HttpConnection,
            public HttpDav,
            public OnHttpPropfindListener
    {
    public:
        WebdavConnection();
        virtual ~WebdavConnection();

        sp<Retval> propfind(const char *uri);

        virtual sp<Retval> onRequest();
        virtual sp<Retval> onResponse(const char *buffer, dfw_size_t size);
        virtual sp<Retval> onPropfind(HttpDav *dav, HttpPropfind *data);

    };

};
#endif


#endif /* DFRAMEWORK_HTTP_WEBDAV_CONNECTION_H */

