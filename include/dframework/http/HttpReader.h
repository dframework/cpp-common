#ifndef DFRAMEWORK_HTTP_HTTPREADER_H
#define DFRAMEWORK_HTTP_HTTPREADER_H


#include <dframework/lang/String.h>
#include <dframework/http/IHttpReader.h>


#ifdef __cplusplus
namespace dframework {

    class HttpReader : public IHttpReader
    {
    public:
        HttpReader();
        virtual ~HttpReader();
    };


};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPREADER_H */


