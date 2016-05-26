#ifndef DFRAMEWORK_HTTPD_HTTPRESPONSE_H
#define DFRAMEWORK_HTTPD_HTTPRESPONSE_H

#include <dframework/http/HttpHeader.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/httpd/OriginFs.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdThread;
    class HttpdClient;
    class HttpdSender;

    class HttpResponse : public HttpHeader
    {
    private:
        dfw_httpstatus_t m_status;
        uint64_t         m_iSendedSize;
        String           m_sBody;
        bool             m_bKeepAlive;

        String  m_sDate;
        String  m_sServer;
        String  m_sLastModified;
        String  m_sETag;
        String  m_sKeepAlive;
        String  m_sConnection;
        String  m_sContentType;

        // for HttpdSendLocalFile;
        sp<OriginFs>     m_originFs;
        int              m_iFileStatus;
        size_t           m_iFileSize;
        time_t           m_iFileMTime;
        String           m_sFileETag;
        String           m_sFileContentType;
#define DFW_HTTPD_BUFFER 102400
        char             m_buffer[DFW_HTTPD_BUFFER];
        unsigned         m_iBufferLen;
        uint64_t         m_iFileSendLength;
        uint64_t         m_iFileOffset;
        uint64_t         m_iFileSended;

        // for HttpdSendStream;
        String           m_sStream;
        size_t           m_iStreamSended;

    public:
        HttpResponse();
        virtual ~HttpResponse();

        inline int getStatus() { return m_status; }
        inline uint64_t getSendedSize() { return m_iSendedSize; }

        sp<Retval> makeStream();

        virtual sp<Retval> appendHeader(const char* name, const char* value);
        virtual inline sp<Retval> appendHeader(const String& name
                                     , const char* value){
            return appendHeader(name.toChars(), value);
        }
        virtual inline sp<Retval> appendHeader(const char* name
                                     , const String& value){
            return appendHeader(name, value.toChars());
        }
        virtual inline sp<Retval> appendHeader(const String& name
                                     , const String& value){
            return appendHeader(name.toChars(), value.toChars());
        }

        friend class HttpdThread;
        friend class HttpdClient;
        friend class HttpdSender;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPRESPONSE_H */

