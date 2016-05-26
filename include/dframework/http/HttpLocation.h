#ifndef DFRAMEWORK_HTTP_HTTPLOCATION_H
#define DFRAMEWORK_HTTP_HTTPLOCATION_H


#include <dframework/base/Retval.h>
#include <dframework/net/Socket.h>


#ifdef __cplusplus
namespace dframework {

    class HttpLocation {
    public:
        HttpLocation();
        virtual ~HttpLocation();

        sp<Retval> parse(
                URI& outUri, URI& orgUri, String& location, const char *orgip);
    };

};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPLOCATION_H */

