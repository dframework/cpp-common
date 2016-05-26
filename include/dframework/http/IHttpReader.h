#ifndef DFRAMEWORK_HTTP_IHTTPREADER_H
#define DFRAMEWORK_HTTP_IHTTPREADER_H


#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/net/Socket.h>

#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpQuery;
    class HttpRound;

    class IHttpReader
    {
    protected:
        virtual sp<Retval> readLastBlocks();
        virtual sp<Retval> procBlocks(
                    const char* buffer, size_t size, bool isended);


    public:
        friend class HttpConnection;

        sp<HttpConnection> m_pConnection;
        sp<Socket>         m_pSocket;
        sp<HttpQuery>      m_pQuery;
        sp<HttpRound>      m_pRound;

//        String             m_sRecvBuffer;
//        size_t             m_uBlockSize;

        IHttpReader();
        virtual ~IHttpReader();

        virtual void reset(sp<HttpConnection>& conn, sp<HttpRound>& round);
        virtual sp<Retval> readBlocks();
        virtual sp<Retval> read(unsigned* out, char* b, unsigned s);

    };


};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPREADER_H */


