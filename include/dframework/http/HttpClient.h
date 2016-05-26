#ifndef DFRAMEWORK_HTTP_HTTPCLIENT_H
#define DFRAMEWORK_HTTP_HTTPCLIENT_H

#include <dframework/base/Retval.h>
#include <dframework/base/Object.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/lang/String.h>

#ifdef __cplusplus
namespace dframework {

    class HttpClient : public Object
    {
    public:
        static const dfw_uint_t MAX_BLOCK_SIZE = 10240;
        static const int        MAX_RETRY_REQUEST = 10; /* Location */
        static const char*      USER_AGENT;

    public:
        HttpClient();
        virtual ~HttpClient();
    };



};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPCLIENT_H */

