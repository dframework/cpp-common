#ifndef DFRAMEWORK_HTTP_HTTPUTILS_H
#define DFRAMEWORK_HTTP_HTTPUTILS_H


#ifdef __cplusplus
namespace dframework {

    class HttpUtils {
    public:
        HttpUtils();
        virtual ~HttpUtils();

        static const char *contentType(const char *fm);
    };


};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPUTILS_H */

