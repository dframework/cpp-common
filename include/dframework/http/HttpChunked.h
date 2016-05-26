#ifndef DFRAMEWORK_HTTP_HTTPCHUNKED_H
#define DFRAMEWORK_HTTP_HTTPCHUNKED_H


#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/http/IHttpReader.h>


#ifdef __cplusplus
namespace dframework {

    class HttpChunked : public IHttpReader
    {
    public:
        String      m_sChunkedBuffer;

        HttpChunked();
        virtual ~HttpChunked();


        virtual sp<Retval> readBlocks();
        virtual sp<Retval> read(unsigned* out, char* b, unsigned size);

    private:
        sp<Retval> procChunkedBlocks(
                const char *buffer, size_t size, bool isended);
        sp<Retval> parseChunkedBlocks(
                size_t *o_size, char *buf, size_t t_size);

        sp<Retval> readSub(unsigned* out, char* b, unsigned size);
        sp<Retval> procChunkedRead(unsigned* out, char* b, unsigned size
                                  , const char *buffer, size_t isize);
        sp<Retval> parseChunkedRead(unsigned* out, char* b, unsigned size
                                  , size_t *o_isize, const char *ibuf, size_t isize);
    };


};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPCHUNKED_H */

