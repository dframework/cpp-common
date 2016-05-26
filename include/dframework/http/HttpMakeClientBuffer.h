#ifndef DFRAMEWORK_HTTP_HTTPMAKECLIENTBUFFER_H
#define DFRAMEWORK_HTTP_HTTPMAKECLIENTBUFFER_H


#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>


#ifdef __cplusplus
namespace dframework {

    class HttpQuery;
    class HttpHeader;

    class HttpMakeClientBuffer
    {
    public:
        sp<HttpQuery> m_pQ;
        sp<HttpRound> m_pR;

        String     m_sHost;
        String     m_sContents;
        String     m_sContentType;
        dfw_size_t m_uContentLength;
        int        m_iDepth;
        bool       m_bDepth;
        bool       m_bMultipart;
        String     m_sBoundary;
        bool       m_bExpect100;

    public:
        HttpMakeClientBuffer();
        virtual ~HttpMakeClientBuffer();

        sp<Retval> make(sp<HttpQuery>& q, sp<HttpRound>& round);
        sp<Retval> makeHost();
        sp<Retval> makeContents();
        sp<Retval> makeMultipartContents();

        sp<Retval> appendHeaders(sp<HttpHeader>& h);

    };



};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPMAKECLIENTBUFFER_H */

