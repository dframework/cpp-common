#ifndef DFRAMEWORK_HTTPD_HTTPREQUEST_H
#define DFRAMEWORK_HTTPD_HTTPREQUEST_H

#include <dframework/http/HttpHeader.h>
#include <dframework/httpd/types.h>
#include <dframework/net/URI.h>
#include <dframework/util/NamedObject.h>
#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdClient;
    class HttpdWorker;
    class HttpdThread;
    class HttpdSendLocalFile;
    class HttpdSendStream;

    class HttpRequest : public HttpHeader
    {
    private:
        ArraySorted<NamedObject> m_aContexts;

        sp<Object> m_host;
        //sp<URI> m_sDocumentRoot;
        String     m_sRequestLine;
        String     m_sLocalFileNm;
        int        m_cstatus;
        String     m_sBuffer;
        String     m_sRequestHeaders;

        String     m_sMethod;
        String     m_sFile;
        String     m_sFileExt;
        String     m_sQuery;
        String     m_sProtocol;
        String     m_sVersion;
        String     m_sRequest;

        String     m_sHost;
        String     m_sUserAgent;
        String     m_sConnection;
        String     m_sAcceptLanguage;
        String     m_sAcceptEncoding;
        String     m_sIfModifiedSince;
        String     m_sIfNoneMatch;
        String     m_sIfRange;
        bool       m_bIfRange;
        int        m_iUpgradeInsecureRequests;

        bool       m_bKeepAlive;
        bool       m_bCacheControl;
        uint64_t   m_iCacheControlMaxAge;

    public:
        HttpRequest();
        virtual ~HttpRequest();

        inline sp<Object> getHostObject() { return m_host; }
        inline String getRequestLine() { return m_sRequestLine; }
        inline String getLocalFileName() { return m_sLocalFileNm; }
        inline int cstatus() { return m_cstatus; }
        inline String getBuffer() { return m_sBuffer; }

        inline String getMethod() { return m_sMethod; }
        inline String getFile() { return m_sFile; }
        inline String getFileExt() { return m_sFileExt; }
        inline String getQuery() { return m_sQuery; }
        inline String getProtocol() { return m_sProtocol; }
        inline String getVersion() { return m_sVersion; }

        inline String getUserAgent() { return m_sUserAgent; }
        inline bool isKeepAlive() { return m_bKeepAlive; }

        inline const char* getRequestString(){ return m_sRequest.toChars(); }
        inline void setRequestString(const char* s){ m_sRequest = s; }
        inline const char* getRequestHeaderString() { return m_sRequestHeaders.toChars(); }

        inline bool IsIfRange() { return m_bIfRange; }

        sp<NamedObject> removeContext(const char* name);

        sp<NamedObject> getContext(unsigned position);
        sp<NamedObject> getContext(const char* name);
        sp<Object> getContextObject(const char* name);
        sp<String> getContextString(const char* name);
        int         getContextInt(const char* name, int defaultVal);

        sp<Retval> addContext(const char* name, const char* value);
        sp<Retval> addContext(const char* name, sp<Object> value);
        inline sp<Retval> addContext(String& sName, const char* value){
            return addContext(sName.toChars(), value);
        }
        inline sp<Retval> addContext(const char* name, String& sValue){
            return addContext(name, sValue.toChars());
        }
        inline sp<Retval> addContext(String& sName, String& sValue){
            return addContext(sName.toChars(), sValue.toChars());
        }
        inline sp<Retval> addContext(String& sName, sp<Object>& value){
            return addContext(sName.toChars(), value);
        }

        inline String getHost() { return m_sHost; }

    private:
        sp<Retval> parseRequestHeaders();
        sp<Retval> parseRequestReady();
    public:
        sp<Retval> parseRequest(const char* buf, size_t rsize);


        friend class HttpdClient;
        friend class HttpdWorker;
        friend class HttpdThread;
        friend class HttpdSendLocalFile;
        friend class HttpdSendStream;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPREQUEST_H */

