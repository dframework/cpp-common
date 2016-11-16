#include <dframework/httpd/HttpResponse.h>
#include <dframework/httpd/HttpdSender.h>
#include <dframework/httpd/HttpdUtil.h>
#include <dframework/httpd/HttpdHost.h>
#include <dframework/httpd/HttpdConfigure.h>
#include <dframework/util/Regexp.h>
#include <dframework/io/Stat.h>
#include <dframework/io/File.h>
#include <dframework/log/Logger.h>

#include <time.h>

namespace dframework {

    HttpResponse::HttpResponse(){
        m_status = DFW_HTTP_STATUS_0;
        m_bKeepAlive = false;
    }

    HttpResponse::~HttpResponse(){
    }

    sp<Retval> HttpResponse::makeStream(){
        AutoLock _l(this);
        sp<Retval> retval;

        const char* title = HttpdSender::getStatusMsg(m_status);
        m_iSendedSize = 0;
        m_iStreamSended = 0;
        m_sStream = String::format("HTTP/1.1 %d %s\r\n"
                                      , m_status, title);

        if( m_sDate.empty() ) {
            time_t iTime = ::time(NULL);
            Time::getDateStringRfc822(m_sDate, iTime);
        }
        if( m_sServer.empty() ){
#ifdef __APPLE__
            m_sServer = "Dframework/1.0.2 (Darwin)";
#else
            m_sServer = "Dframework/1.0.2";
#endif
        }
        if( m_sLastModified.empty() ){
            m_sLastModified = m_sDate;
        }

        m_sStream.appendFmt("Date: %s\r\n", m_sDate.toChars());
        m_sStream.appendFmt("Server: %s\r\n", m_sServer.toChars());
        m_sStream.appendFmt("Last-Modified: %s\r\n", m_sLastModified.toChars());
        if( !m_sETag.empty() ){
            m_sStream.appendFmt("ETag: %s\r\n", m_sETag.toChars());
        }

        int size = getHeaderSize();
        for(int k=0; k<size; k++){
            sp<NamedValue> nv = getHeader(k);
            if(nv.has()){
                m_sStream.appendFmt("%s: %s\r\n"
                                , nv->m_sName.toChars()
                                , nv->m_sValue.toChars());
            }
        }

        if( !m_sKeepAlive.empty() ){
            m_sStream.appendFmt("Keep-Alive: %s\r\n", m_sKeepAlive.toChars());
            m_sStream.appendFmt("Connection: Keep-Alive\r\n");
        }else if( !m_sConnection.empty() ) {
            m_sStream.appendFmt("Connection: %s\r\n", m_sConnection.toChars());
        }else{
            m_sStream.appendFmt("Connection: Close\r\n");
        }

        if( !m_sContentType.empty() ){
            m_sStream.appendFmt("Content-Type: %s\r\n", m_sContentType.toChars());
        }

        m_sStream.appendFmt("\r\n");
        if( !m_sBody.empty() )
            m_sStream.append(m_sBody);

        return NULL;
    }

    sp<Retval> HttpResponse::appendHeader(const char* name, const char* value){
        sp<Retval> retval;
        String sName = name;

        if( sName.equals("Date") ){
            m_sDate = value;
        }else if( sName.equals("Server") ){
            m_sServer = value;
        }else if( sName.equals("Last-Modified") ){
            m_sLastModified = value;
        }else if( sName.equals("ETag") ){
            m_sETag = value;
        }else if( sName.equals("Keep-Alive") ){
            m_sKeepAlive = value;
        }else if( sName.equals("Content-Type") ){
            m_sContentType = value;
        }else if( sName.equals("Connection") ){
            m_sConnection= value;
        }else if( DFW_RET(retval, HttpHeader::appendHeader(name, value)) ){
            return DFW_RETVAL_D(retval); 
        }

        return NULL;
    }

};

