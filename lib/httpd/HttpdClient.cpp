#include <dframework/httpd/HttpdClient.h>
#include <dframework/httpd/HttpdSender.h>
#include <dframework/httpd/HttpdUtil.h>
#include <dframework/httpd/HttpdHost.h>
#include <dframework/httpd/HttpdConfigure.h>
#include <dframework/util/Regexp.h>
#include <dframework/io/Stat.h>
#include <dframework/io/File.h>
#include <dframework/log/Logger.h>


namespace dframework {

    HttpdClient::HttpdClient(){
        m_req = new HttpRequest();
        m_resp = new HttpResponse();
        m_request_count = 0;
    }

    HttpdClient::~HttpdClient(){
    }

    void HttpdClient::ready(){
        AutoLock _l(this);
        m_req = new HttpRequest();
        m_resp = new HttpResponse();
    }

    void HttpdClient::setSocket(sp<ClientSocket>& sock){
        AutoLock _l(this);
        m_sock = sock;
    }

#define MAX_READ_PACKET 10240
    sp<Retval> HttpdClient::parseRequest(){
        sp<Retval> retval;

        sp<HttpdConfigure> configure = m_configure;
        size_t size;
        char buffer[MAX_READ_PACKET];
        dfw_time_t s_time = Time::currentTimeMillis();

        do{
            size_t rsize = 0;
            size_t length = m_req->m_sBuffer.length();
            if( 0 == (size = (MAX_READ_PACKET - length)) ){
                return DFW_RETVAL_NEW(DFW_ERROR, HTTPD_STATUS_500);
            }

            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*600/* FIXME: */) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout read request. handle=%d"
                                        , getHandle());
            }

            if( DFW_RET(retval, m_sock->wait_recv()) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            if( DFW_RET(retval, m_sock->recv(buffer, &rsize, size)) ){
                if( 0 == rsize ){
                    int rvalue = retval->value();
                    if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                        continue;
                    }
                    return DFW_RETVAL_D(retval);
                }
            }

            if( DFW_RET(retval, m_req->parseRequest(buffer, rsize)) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            m_request_count++;
            if( m_request_count == (uint64_t)-1 )
                m_request_count = 1;

            if( configure->isPrintRequestHeader() ){
                DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this, "test:: handle=%d, %s"
                       , getHandle()
                       , m_req->m_sRequestHeaders.toChars());
            }

            return NULL;
        }while(true);
    }

    sp<Retval> HttpdClient::readyRequest(){
        sp<Retval> retval;

        sp<HttpdConfigure> configure = m_configure;
        sp<HttpdHost> host = configure->getHost(m_req->m_sHost, m_sock->getServerPort());
        if( !host.has() ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, HTTPD_STATUS_500
                                    , "Not found host. host=%s:%d"
                                    , m_req->m_sHost.toChars(), m_sock->getServerPort());
        }

        sp<Object> hostobj = host;
        setHost(hostobj);

        DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), this, "r:");

        // ----------------------------------------------------------
        // module
        sp<HttpdClient> thiz = this;
        for(int k=0; k<host->getModSize(); k++){
            dfw_httpstatus_t outstatus = DFW_HTTP_STATUS_0;
            sp<HttpdMod> mod = host->getMod(k);
            if( DFW_RET(retval, mod->request(thiz, &outstatus))){
                if(outstatus!=DFW_HTTP_STATUS_0){
                    return DFW_RETVAL_D_SET_ERR(retval, HTTPD_STATUS_500);
                }
            }
        }
        // module
        // ----------------------------------------------------------

        return NULL;
    }

    sp<Retval> HttpdClient::sendResponseStatus(){
        sp<Retval> retval;

        if( DFW_RET(retval, makeStream()) ){
            return DFW_RETVAL_D_SET_ERR(retval, HTTPD_STATUS_500);
        }

        dfw_time_t s_time = Time::currentTimeMillis();
        do{
            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*600/* FIXME: */) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout read request. handle=%d"
                                        , getHandle());
            }

            int comp = 0;
            if( DFW_RET(retval, sendStream(&comp)) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT){
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            DFWLOG_CR(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), this, retval, "l:");
            return NULL;
        }while(true);
    }

    sp<Retval> HttpdClient::sendResponse(){
        sp<Retval> retval;

        if( !isLocalFile() ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, DFW_HTTP_STATUS_404, "Not support cgi.");
        }

        dfw_httpstatus_t outstatus = DFW_HTTP_STATUS_0;
        if( DFW_RET(retval, se_checkLocalFile(&outstatus)) ){
            sp<Retval> retval2;
            if( DFW_RET(retval2, sendResponseStatus()) )
                return DFW_RETVAL_D(retval2);
            if( retval->value() == DFW_OK )
                return NULL;
            return DFW_RETVAL_D(retval);
        }

        dfw_time_t s_time = Time::currentTimeMillis();
        do{
            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*600/* FIXME: */) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout read request. handle=%d"
                                        , getHandle());
            }

            if( DFW_RET(retval, sendLocalFile()) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT){
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), this, "l:");
            return NULL;
        }while(true);
    }

    bool HttpdClient::isLocalFile(){
        // FIXME
        return true;
    }

    String HttpdClient::getFilename(){
        AutoLock _l(this);
        return m_req->m_sFile;
    }

    sp<Retval> HttpdClient::set301Response(dfw_httpstatus_t* out
                                         , String& sLocation)
    {
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdClient> thiz = this;
        if( DFW_RET(retval, HttpdSender::makeLocation(thiz
                                    , DFW_HTTP_STATUS_301
                                    , HttpdSender::STATUS_TITLE_301
                                    , sLocation.toChars())) ){
            return DFW_RETVAL_D(retval);
        }
        if(out) *out = DFW_HTTP_STATUS_301;
        return NULL;
    }

    sp<Retval> HttpdClient::set304Response(dfw_httpstatus_t* out){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdClient> thiz = this;
        if( DFW_RET(retval, HttpdSender::makeNoBodyError(thiz
                                , DFW_HTTP_STATUS_304
                                , HttpdSender::STATUS_TITLE_304, true)) ){
            return DFW_RETVAL_D(retval);
        }
        if( !m_resp->m_sFileETag.empty() ){
            m_resp->appendHeader("ETag",m_resp->m_sFileETag);
        }
        if(out) *out = DFW_HTTP_STATUS_304;
        return NULL;
    }

    sp<Retval> HttpdClient::set403Response(dfw_httpstatus_t* out){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdClient> thiz = this;
        if( DFW_RET(retval, HttpdSender::makeError(thiz
                                    , DFW_HTTP_STATUS_403
                                    , HttpdSender::STATUS_TITLE_403)) ){
            return DFW_RETVAL_D(retval);
        }
        if(out) *out = DFW_HTTP_STATUS_403;
        return NULL;
    }

    sp<Retval> HttpdClient::set404Response(dfw_httpstatus_t* out){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdClient> thiz = this;
        if( DFW_RET(retval, HttpdSender::makeError(thiz
                                    , DFW_HTTP_STATUS_404
                                    , HttpdSender::STATUS_TITLE_404)) ){
            return DFW_RETVAL_D(retval);
        }
        if(out) *out = DFW_HTTP_STATUS_404;
        return NULL;
    }

    sp<Retval> HttpdClient::se_checkLocalFile(dfw_httpstatus_t* out){
        AutoLock _l(this);
        sp<Retval> retval, retval2;

        sp<OriginFs> fs = new OriginFs();
        if( DFW_RET(retval, fs->ready(m_req->m_host)) )
            return DFW_RETVAL_D(retval);

        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));
        if( DFW_RET(retval, fs->getattr(m_req->m_sRequest.toChars(), &st)) ){
            switch( retval->value() ){
            case DFW_E_NOENT:
                if( DFW_RET(retval2, set404Response(out)) )
                    return DFW_RETVAL_D(retval2);
                return DFW_RETVAL_D_SET_VAL(retval, DFW_OK);
            default:
                break;
            }
            return DFW_RETVAL_D(retval);
        }

        if( st.st_mode & S_IFDIR ){
            char ch = m_req->m_sFile.toChars()[m_req->m_sFile.length()-1];
            if( ch == '/' ){
                if( DFW_RET(retval2, set403Response(out)) )
                    return DFW_RETVAL_D(retval2);
            }else{
                String sLocation = String::format("%s/"
                                                , m_req->m_sFile.toChars());
                if( DFW_RET(retval2, set301Response(out, sLocation)) )
                    return DFW_RETVAL_D(retval2);
            }
            return DFW_RETVAL_NEW_MSG(DFW_OK, 0, "Not found file.");
        }

        if( !(st.st_mode & S_IFREG) ){
            if( DFW_RET(retval2, set403Response(out)) )
                return DFW_RETVAL_D(retval2);
            return DFW_RETVAL_NEW_MSG(DFW_OK, 0, "Not found file.");
        }

        m_resp->m_iFileStatus = 0;
        m_resp->m_iFileSize = (size_t)st.st_size;
        m_resp->m_iFileMTime = st.st_mtime;
        m_resp->m_sFileETag = String::format("\"%llx-%llx\""
                             , st.st_size, st.st_mtime);

        if( !m_req->m_sIfModifiedSince.empty() ){
            time_t since = 0;
            const char* psince = m_req->m_sIfModifiedSince.toChars();
            if( DFW_RET(retval, HttpdUtil::parseToTime(&since, psince)) ){
                DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_HTTPD_ID), retval
                    , "check if-modified-since : %s" , psince);
            }else if( (since!=0) && (since==st.st_mtime) ){
                if( DFW_RET(retval2, set304Response(out)) )
                    return DFW_RETVAL_D(retval2);
                return DFW_RETVAL_NEW_MSG(DFW_OK, 0, "If-Modified-Since");
            }
        }

        if( !m_req->m_sIfRange.empty() ){
            if( m_resp->m_sFileETag.equals(m_req->m_sIfRange) ){
                m_req->m_bIfRange = true;
            }
        }

        return NULL;
    }

    sp<Retval> HttpdClient::setResponse(dfw_httpstatus_t status
                                      , bool keepAlive)
    {
        AutoLock _l(this);
        sp<Retval> retval;

        if( keepAlive ){
            if(DFW_RET(retval,m_resp->appendHeader("Keep-Alive","timeout=2400")))
                return DFW_RETVAL_D(retval);
            if(DFW_RET(retval,m_resp->appendHeader("Connection","Keep-Alive")))
                  return DFW_RETVAL_D(retval);
            m_resp->m_bKeepAlive = true;
        }else{
            if(DFW_RET(retval,m_resp->appendHeader("Connection","close")))
                  return DFW_RETVAL_D(retval);
            m_resp->m_bKeepAlive = false;
        }

        m_resp->m_status = status;
        
        return NULL;
    }

    sp<Retval> HttpdClient::setResponse(dfw_httpstatus_t status
                                      , size_t contentLength
                                      , bool keepAlive)
    {
        AutoLock _l(this);
        sp<Retval> retval;

        String sContentLength = String::format("%lu", contentLength);
        if( DFW_RET(retval, m_resp->appendHeader("Content-Length"
                                               , sContentLength)) ){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, setResponse(status, keepAlive)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpdClient::setResponseLocation(dfw_httpstatus_t status
                                      , size_t contentLength
                                      , const char* location
                                      , bool keepAlive)
    {
        AutoLock _l(this);
        sp<Retval> retval;
        if(DFW_RET(retval, m_resp->appendHeader("Location", location)) )
            return DFW_RETVAL_D(retval);
        if(DFW_RET(retval, setResponse(status, contentLength, keepAlive)))
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpdClient::makeStream(){
        AutoLock _l(this);
        sp<Retval> retval;
        if( DFW_RET(retval, m_resp->makeStream()) )
            return DFW_RETVAL_D(retval);

        sp<HttpdConfigure> configure = m_configure;
        if( configure->isPrintResponseHeader() ){
            DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this, "test:: handle=%d, %s"
                  , getHandle() 
                  , m_resp->m_sStream.toChars());
        }
        return NULL;
    }

    sp<Retval> HttpdClient::sendStream(int* iscomplete){
        AutoLock _l(this);
        sp<Retval> retval;
        size_t osize = 0;
        size_t sended = m_resp->m_iStreamSended;
        size_t buflen = m_resp->m_sStream.length();
        const char* buf = m_resp->m_sStream.toChars();

        *iscomplete = 0;

        if( buflen != m_resp->m_iStreamSended ){
            if( DFW_RET(retval, m_sock->wait_send()) ){
                return DFW_RETVAL_D(retval);
            }

            if(DFW_RET(retval,m_sock->send(&osize, buf+sended, buflen-sended))){
                if( osize == 0 ){
                    return DFW_RETVAL_D(retval);
                }
            }

            m_resp->m_iSendedSize += osize;
            m_resp->m_iStreamSended += osize;
        }

        if( buflen == m_resp->m_iStreamSended ){
//#if !defined(__APPLE__) && !defined(_WIN32)
//            int leftsize = 0;
//            if( DFW_RET(retval,m_sock->getSendBufferLeftSize(&leftsize)))
//                return DFW_RETVAL_D(retval);
//
//printf("sendStream: handle=%d, equals, leftsize=%d\n", getHandle(), leftsize);
//            if( leftsize == 0 ){
//                *iscomplete = 1;
//                return NULL;
//            }
//#else
            *iscomplete = 1;
            return NULL;
//#endif
        }

        return DFW_RETVAL_NEW(DFW_E_AGAIN, 0);
    }

    sp<Retval> HttpdClient::sendLocalFile_ready(){
        AutoLock _l(this);
        sp<Retval> retval;

        // ----------------------------------------------------------
        // modules
        sp<HttpdClient> thiz = this;
        sp<HttpdConfigure> config = m_configure;
        sp<HttpdHost> host = config->getHost(m_req->m_sHost, getServerPort());
        if( !host.has() ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not host.");
        }
        for(int k=0; k<host->getModSize(); k++){
            sp<HttpdMod> mod = host->getMod(k);
            if( DFW_RET(retval, mod->open(thiz, m_req->m_sFile.toChars()))){
                switch( retval->error() ){
                case ZONEHTTPD_MOD_OK:
                    break;
                case ZONEHTTPD_MOD_AGAIN:
                    return DFW_RETVAL_D_SET(retval, DFW_E_AGAIN, EAGAIN);
                case ZONEHTTPD_MOD_ERROR:
                default :
                    return DFW_RETVAL_D(retval);
                }
            }
        }
        // modules:
        // --------------------------------------------------------

        // ----------------------------------------------------------
        // FIXME: orginfs
        m_resp->m_originFs = new OriginFs();

        if( DFW_RET(retval, m_resp->m_originFs->ready(m_req->m_host)) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, m_resp->m_originFs->open(
                                             m_req->m_sRequest.toChars()
                                            , O_RDONLY)) ){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, m_resp->m_originFs->getContentType(
                                           m_resp->m_sFileContentType)) ){
        }

        dfw_httpstatus_t status = DFW_HTTP_STATUS_0;
        String sModify;
        String sRange, sAcceptRange, sContentRange;
        char *pStart, *pEnd;
        int minus = 0;
        Time::getDateStringRfc822(sModify, m_resp->m_iFileMTime);
        if( DFW_RET(retval, HttpdUtil::checkRangeBytes(m_req.get()
                                             , sRange, sAcceptRange
                                             , &pStart, &pEnd, &minus)) ){
            return DFW_RETVAL_D(retval);
        }

        uint64_t iStart, iEnd, iLength;
        switch( minus ){
        case 0 :
            status = DFW_HTTP_STATUS_200;
            iStart = 0;
            iEnd = m_resp->m_iFileSize-1;
            iLength = m_resp->m_iFileSize;
            break;

        case 1 :
            status = DFW_HTTP_STATUS_206;
            iStart = ::atoll(pStart);
            iEnd = ::atoll(pEnd);
            if(m_resp->m_iFileSize <= iEnd)
                iEnd = m_resp->m_iFileSize - 1;
            iLength = iEnd - iStart + 1;
            sContentRange = String::format("bytes %ld-%ld/%ld"
                                 , iStart, iEnd, m_resp->m_iFileSize);
            break;

        case 2 :
            status = DFW_HTTP_STATUS_206;
            iStart = m_resp->m_iFileSize - ::atoll(pEnd);
            iEnd = m_resp->m_iFileSize - 1;
            iLength = iEnd - iStart + 1;
            sContentRange = String::format("bytes %ld-%ld/%ld"
                                 , iStart, iEnd, m_resp->m_iFileSize);
            break;

        case 3 :
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not support multipart. Range(%s):%d"
                       , sRange.toChars(), minus);
        case 4 :
            status = DFW_HTTP_STATUS_206;
            iStart = ::atoll(pStart);
            iEnd = m_resp->m_iFileSize - 1;
            iLength = iEnd - iStart + 1;
            sContentRange = String::format("bytes %ld-%ld/%ld"
                                 , iStart, iEnd, m_resp->m_iFileSize);
            break;

        default:
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Unknown Range(%s):%d", sRange.toChars(), minus);
        } // end switch(minus)

        if( iEnd < iStart )
            return DFW_RETVAL_NEW(DFW_ERROR, 0);
        if( m_resp->m_iFileSize <= iStart ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "m_iFileSize:%lu, iStart:%lu"
                       , m_resp->m_iFileSize, iStart);
        }

        if( DFW_RET(retval, setResponse(status, (size_t)iLength, true)) )
            return DFW_RETVAL_D(retval);
        if(DFW_RET(retval,m_resp->appendHeader("Accept-Ranges","bytes")))
            return DFW_RETVAL_D(retval);
        if( !sContentRange.empty() ){
            if(DFW_RET(retval,m_resp->appendHeader(
                                          "Content-Range",sContentRange)))
                return DFW_RETVAL_D(retval);
        }
        if( !m_resp->m_sFileContentType.empty() ){
            if(DFW_RET(retval,m_resp->appendHeader(
                                          "Content-Type"
                                          ,m_resp->m_sFileContentType)))
                return DFW_RETVAL_D(retval);
        }

        if( !m_resp->m_sFileETag.empty() && (DFW_RET(
                retval,m_resp->appendHeader("ETag",m_resp->m_sFileETag)))){
            return DFW_RETVAL_D(retval);
        }

        if(DFW_RET(retval,m_resp->appendHeader("Last-Modified",sModify)))
            return DFW_RETVAL_D(retval);

#if 0
        if( m_req->m_bCacheControl ) {
DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "cache-control true");
            if( 0==m_req->m_iCacheControlMaxAge ){
            }else{
            }
            //String sExpires;
            //Time::getDateStringRfc822(sExpires, 0);
            if(DFW_RET(retval,m_resp->appendHeader("Expires","Fri 01 Jan 1900 00:00:00")))
                return DFW_RETVAL_D(retval);
            if(DFW_RET(retval,m_resp->appendHeader("Cache-Control"
                                                  ,"no-cache,must-revalidate")))
                return DFW_RETVAL_D(retval);
            if(DFW_RET(retval,m_resp->appendHeader("Pragma","no-cache")))
                return DFW_RETVAL_D(retval);
        }else{
DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "cache-control false");
        }
#endif

        m_resp->m_sBody = NULL;
        m_resp->m_iBufferLen = 0;
        m_resp->m_iFileSendLength = iLength;
        m_resp->m_iFileOffset = iStart;
        m_resp->m_iFileSended = 0;
        int iscomplete = 0;

        if( DFW_RET(retval, makeStream()) ){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, sendStream(&iscomplete)) ){
            m_resp->m_iFileStatus = 1;
            return DFW_RETVAL_D(retval);
        }
        m_resp->m_iFileStatus = 2;
        return NULL;
    }

    sp<Retval> HttpdClient::sendLocalFile(){
        AutoLock _l(this);
        sp<Retval> retval;

        switch(m_resp->m_iFileStatus){
        case 0 :
            if( DFW_RET(retval, sendLocalFile_ready()) ){
                return DFW_RETVAL_D(retval);
            }
            return DFW_RETVAL_NEW(DFW_E_AGAIN, EAGAIN);

        case 1 :
          {
              int iscomplete = 0;
              if( DFW_RET(retval, sendStream(&iscomplete)) ){
                  return DFW_RETVAL_D(retval);
              }
              m_resp->m_iFileStatus = 2;
          }
          return DFW_RETVAL_NEW(DFW_E_AGAIN, EAGAIN);

        case 2 :
          {
#define MAX_BUFFER 102400
          size_t s_osize = 0;
          unsigned out_fsize = 0;
          unsigned snd_fsize = 0;
          unsigned fbuflen = m_resp->m_iBufferLen;
          uint64_t thiz_length = m_resp->m_iFileSendLength - m_resp->m_iFileSended;
          char fbuf[MAX_BUFFER];
          unsigned readable_size = MAX_BUFFER;

//printf("#1 **************************************** : handle=%d, sended=%lld, offset=%lld, thiz-length=%lld, fbuflen=%d\n" , getHandle(), m_resp->m_iFileSended, m_resp->m_iFileOffset, thiz_length, fbuflen);

          if( DFW_RET(retval, m_sock->wait_send()) ){
              return DFW_RETVAL_D(retval);
          }

          if( fbuflen == 0 ){
              uint64_t fileOffset = m_resp->m_iFileOffset;

              if( ((uint64_t)readable_size) > (thiz_length) )
                  readable_size = (unsigned)thiz_length;

              if( DFW_RET(retval, m_resp->m_originFs->read(
                                         m_req->m_sRequest.toChars()
                                       , &out_fsize
                                       , fbuf
                                       , readable_size, fileOffset)) ){
                  return DFW_RETVAL_D(retval);
              }
                          
              m_resp->m_iBufferLen = out_fsize;
              m_resp->m_iFileOffset += out_fsize;

              // -- modules ------------------------------------------------------
              sp<HttpdClient> thiz = this;
              sp<HttpdConfigure> conf = m_configure;
              sp<HttpdHost> host=conf->getHost(m_req->m_sHost,getServerPort());
              if( !host.has() ){
                  ::memcpy(m_resp->m_buffer, fbuf, m_resp->m_iBufferLen);
                  return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not host.");
              }
              for(int k=0; k<host->getModSize(); k++){
                  sp<HttpdMod> mod = host->getMod(k);
                  if( DFW_RET(retval, mod->read(thiz, fbuf, out_fsize, fileOffset))){
                      switch( retval->error() ){
                      case ZONEHTTPD_MOD_OK:
                          break;
#if 0
                      case ZONEHTTPD_MOD_AGAIN:
                          ::memcpy(m_resp->m_buffer, fbuf, m_resp->m_iBufferLen);
                          return DFW_RETVAL_D_SET(retval, DFW_E_AGAIN, EAGAIN);
#endif
                      case ZONEHTTPD_MOD_ERROR:
                      default :
                          ::memcpy(m_resp->m_buffer, fbuf, m_resp->m_iBufferLen);
                          return DFW_RETVAL_D(retval);
                      }
                  }
              }
              // -- modules ------------------------------------------------------
              snd_fsize = out_fsize;
          }else{
              ::memcpy(fbuf, m_resp->m_buffer, fbuflen);
              snd_fsize = fbuflen;
          }

          retval = m_sock->send(&s_osize, fbuf, snd_fsize);
          if( s_osize > 0 ){
              m_resp->m_iSendedSize += s_osize;
              m_resp->m_iFileSended += s_osize;
              m_resp->m_iBufferLen = snd_fsize - s_osize;
          }else{
              m_resp->m_iBufferLen = snd_fsize;
          }

          if( m_resp->m_iBufferLen>0 )
              ::memcpy(m_resp->m_buffer, fbuf+s_osize, m_resp->m_iBufferLen);
          if( !(m_resp->m_iFileSended < m_resp->m_iFileSendLength) )
              m_resp->m_iFileStatus = 3;
          if( retval.has() )
              return DFW_RETVAL_D(retval);
          return DFW_RETVAL_NEW(DFW_E_AGAIN, EAGAIN);
        }

        case 3:
//          {
//#if !defined(__APPLE__) && !defined(_WIN32)
//              int leftsize = 0;
//              if( DFW_RET(retval,m_sock->getSendBufferLeftSize(&leftsize)))
//                  return DFW_RETVAL_D(retval);
//              if( leftsize == 0 )
//                  return NULL;
//#else
              return NULL;
//#endif
//          }// end case 3: {
//          return DFW_RETVAL_NEW(DFW_E_AGAIN, EAGAIN);
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Unknown send file status.");
    }

};

