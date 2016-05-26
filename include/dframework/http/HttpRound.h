#ifndef DFRAMEWORK_HTTP_HTTPROUND_H
#define DFRAMEWORK_HTTP_HTTPROUND_H


#include <dframework/base/Retval.h>
#include <dframework/base/Object.h>
#include <dframework/lang/String.h>
#include <dframework/net/Hostname.h>
#include <dframework/net/URI.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/http/HttpHeader.h>


#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpAuth;

    class HttpRound : public Object
    {
    public:
        friend class HttpConnection;
        friend class HttpAuth;

        int            m_iRound;
        sp<HttpHeader> m_responseHeader;

        String         m_sUri;
        URI            m_oUri;
        Hostname       m_oHostname;

        /* request data. */
        String         m_sStatusLine;
        int            m_iMajorVersion;
        int            m_iMinorVersion;
        int            m_iStatus;
        String         m_sStatusMsg;

        /* response data. */
        dfw_size_t     m_uRecvContentLength;
        String         m_sContentTypeLine;
        String         m_sContentType;
        String         m_sContentSubType;
        String         m_sCharset;

        String         m_sReaderBuffer;  // HttpReader and HttpChunker
        String         m_sChunkedBuffer; // parsed contents
        bool           m_bChunked;

        dfw_time_t     m_tLastModified;
        dfw_time_t     m_tDate;
        dfw_time_t     m_tExpires;
        bool           m_bKeepAlive;
        dfw_size_t     m_uContentLength;
        String         m_sLocation;
        String         m_sServer;
        int            m_iTimeout;
        int            m_iTimeoutMax;
        bool           m_bUseSpecialStatusCode;

#if 0
        /* times */
        struct timeval m_tConnectTime;
        struct timeval m_tRequestTime;
        struct timeval m_tResponseHeaderTime;
        struct timeval m_tResponseContentTime;
#endif

        /* request temped data */
        sp<Retval>     m_iLastDStatus;
        String         m_sHeadBuffer;
        String         m_sContentBuffer;

        /* response temped data */
        String         m_sRecvBuffer;
        String         m_sLineBuffer;
        bool           m_bOnRecvContents;

        HttpRound(int round);
        virtual ~HttpRound();

        sp<Retval> ready(const char *uri);
        sp<Retval> ready(const URI& uri);
        sp<Retval> parseUri();

        sp<Retval> readLine();
        sp<Retval> parseStatusLine();
        sp<Retval> parseHeaderLine();
        sp<Retval> patchHeader(String& name, String& value);
        sp<Retval> parseContentType(const char *str);

    };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPROUND_H */

