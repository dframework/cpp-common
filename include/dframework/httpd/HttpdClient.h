#ifndef DFRAMEWORK_HTTPD_HTTPDCLIENT_H
#define DFRAMEWORK_HTTPD_HTTPDCLIENT_H

#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/net/URI.h>
#include <dframework/net/ServerAccept.h> // <ClientSocket.h>
#include <dframework/httpd/HttpRequest.h>
#include <dframework/httpd/HttpResponse.h>
#include <dframework/httpd/types.h>

namespace dframework {

    class HttpdWorker;
    class HttpdThread;
    class HttpdConfigure;

    class HttpdClient : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(HttpdClient);

    private:
        sp<ClientSocket> m_sock;
        sp<HttpRequest>  m_req;
        sp<HttpResponse> m_resp;
        sp<Object> m_configure;
        uint64_t   m_request_count;
        bool       m_bStop;

    public:
        HttpdClient();
        virtual ~HttpdClient();

        void ready();
        void stop();
        void close();

        void setSocket(sp<ClientSocket>& sock);
        inline void setConfigure(sp<Object>& configure){
            m_configure = configure;
        }
        inline sp<HttpRequest> getRequest(){ return m_req; }
        inline sp<HttpResponse> getResponse(){ return m_resp; }
        inline int getCStatus() { return m_req->m_cstatus; }

        sp<Retval> parseRequest();
        sp<Retval> readyRequest();

        bool isLocalFile();
        inline void setHost(sp<Object>& host){
            m_req->m_host = host;
        }
#if 0
        inline void setDocumentRoot(sp<URI> sDocumentRoot){
            m_req->m_sDocumentRoot = sDocumentRoot;
        }
#endif
        inline uint64_t getRequestCount() { 
            return m_request_count;
        }
        inline int getHandle(){
            return (m_sock.has() ? m_sock->getHandle() : -1);
        }

        String getFilename();

        sp<Retval> set301Response(dfw_httpstatus_t* out, String& sLocation);
        sp<Retval> set304Response(dfw_httpstatus_t* out);
        sp<Retval> set403Response(dfw_httpstatus_t* out);
        sp<Retval> set404Response(dfw_httpstatus_t* out);

        sp<Retval> se_checkLocalFile(dfw_httpstatus_t* out);

        sp<Retval> setResponse(dfw_httpstatus_t status
                             , bool keepAlive=false);
        sp<Retval> setResponse(dfw_httpstatus_t status
                             , size_t contentLength
                             , bool keepAlive=false);
        sp<Retval> setResponseLocation(dfw_httpstatus_t status
                             , size_t contentLength
                             , const char* location
                             , bool keepAlive=false);

    private:
        sp<Retval> sendResponseStatus();
    public:
        sp<Retval> sendResponse();
        sp<Retval> makeStream();
        sp<Retval> sendStream(int* iscomplete);
    private:
        sp<Retval> sendLocalFile_ready();
    public:
        sp<Retval> sendLocalFile();

        inline bool isKeepAlive(){
            return (m_resp.has() ? m_resp->m_bKeepAlive : false);
        }

        inline int getServerPort(){ return m_sock->getServerPort(); }

        inline const char* getIp(){ return m_sock->getIp(); }

        //-----------------------------------------------------------
        // HttpRequest
        inline sp<Object> getHost() {
            return (m_req.has() ? m_req->m_host : NULL);
        }
#if 0
        inline sp<URI> getDocumentRoot() {
            return (m_req.has() ? m_req->m_sDocumentRoot : NULL);
        }
#endif
        inline String getLocalFileNm() {
            return (m_req.has() ? m_req->m_sLocalFileNm : NULL);
        }
        inline String getRequestLine() {
            return (m_req.has() ? m_req->m_sRequestLine : NULL);
        }
        inline String getMethod() {
            return (m_req.has() ? m_req->m_sMethod : NULL);
        }
        inline String getFile() {
            return (m_req.has() ? m_req->m_sFile : NULL);
        }
        inline String getFileExt() {
            return (m_req.has() ? m_req->m_sFileExt : NULL);
        }
        inline String getQuery() {
            return (m_req.has() ? m_req->m_sQuery : NULL);
        }
        inline String getProtocol() {
            return (m_req.has() ? m_req->m_sProtocol : NULL);
        }
        inline String getVersion() {
            return (m_req.has() ? m_req->m_sVersion : NULL);
        }
        inline String getHostname() {
            return (m_req.has() ? m_req->m_sHost : NULL);
        }
        inline String getUserAgent() {
            return (m_req.has() ? m_req->m_sUserAgent : NULL);
        }
        inline String getConnection() {
            return (m_req.has() ? m_req->m_sConnection : NULL);
        }
        inline String getAcceptLanguage() {
            return (m_req.has() ? m_req->m_sAcceptLanguage : NULL);
        }
        inline String getAcceptEncoding() {
            return (m_req.has() ? m_req->m_sAcceptEncoding : NULL);
        }
        inline String getIfModifiedSince() {
            return (m_req.has() ? m_req->m_sIfModifiedSince : NULL);
        }
        inline bool isRequestKeepAlive() {
            return (m_req.has() ? m_req->m_bKeepAlive : false);
        }

        inline int getStatus() {
            return (m_resp.has() ? m_resp->getStatus() : 0); 
        }
        inline uint64_t getSendedSize() {
            return (m_resp.has() ? m_resp->getSendedSize() : 0); 
        }

        friend class HttpdWorker;
        friend class HttpdThread;
    };

};

#endif /* DFRAMEWORK_HTTPD_HTTPDCLIENT_H */

