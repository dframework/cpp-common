#ifndef DFRAMEWORK_HTTP_HTTPCONTENTTYPE_H
#define DFRAMEWORK_HTTP_HTTPCONTENTTYPE_H

#include <dframework/base/Retval.h>

#ifdef __cplusplus
namespace dframework {

    class HttpContentType : public Object
    {
    public:
        static const char* getContentType(const char* ext);

    private:
        static const char* getContentType_1(const char* ext);
        static const char* getContentType_2(const char* ext);
        static const char* getContentType_3(const char* ext);
        static const char* getContentType_4(const char* ext);

    };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPCONTENTTYPE_H */

