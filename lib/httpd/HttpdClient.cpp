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

    DFW_DECLARATION_SAFE_COUNT(HttpdClient);

    HttpdClient::HttpdClient(){
        DFW_SAFE_ADD(HttpdClient, l);
        m_req = new HttpRequest();
        m_resp = new HttpResponse();
        m_request_count = 0;
        m_bStop = false;
        m_bEnableResizeableFile = false;
    }

    HttpdClient::~HttpdClient(){
        close();
        DFW_SAFE_REMOVE(HttpdClient, l);
    }

    void HttpdClient::ready(){
        AutoLock _l(this);
        m_req = new HttpRequest();
        m_resp = new HttpResponse();
    }

    void HttpdClient::stop(){
        AutoLock _l(this);
        m_bStop = true;
        if(m_sock.has()){
            m_sock->stop();
        }
    }

    void HttpdClient::close(){
        AutoLock _l(this);
        if(m_sock.has()){
            m_sock->close();
        }
    }

    void HttpdClient::setSocket(sp<ClientSocket>& sock){
        AutoLock _l(this);
        m_sock = sock;
    }

#define MAX_TIMEOUT (60*120)
#define MAX_READ_PACKET 102400
    sp<Retval> HttpdClient::parseRequest(){
        sp<Retval> retval;

        sp<HttpdConfigure> configure = m_configure;
        size_t size;
        char buffer[MAX_READ_PACKET];
        dfw_time_t c_time;
        dfw_time_t s_time = Time::currentTimeMillis();

        do{
            if(m_bStop){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                                        , "Stop httpd thread. handle=%d"
                                        , getHandle());
            }

            size_t rsize = 0;
            size_t length = m_req->m_sBuffer.length();
            if( 0 == (size = (MAX_READ_PACKET - length)) ){
                return DFW_RETVAL_NEW(DFW_ERROR, HTTPD_STATUS_500);
            }

            c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*MAX_TIMEOUT/* FIXME: */) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout read request. handle=%d"
                                        , getHandle());
            }

            if( DFW_RET(retval, m_sock->wait_recv()) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                    sleep(0);
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            if( DFW_RET(retval, m_sock->recv(buffer, &rsize, size)) ){
                if( 0 == rsize ){
                    int rvalue = retval->value();
                    if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                        sleep(0);
                        continue;
                    }
                    return DFW_RETVAL_D(retval);
                }
            }

            s_time = Time::currentTimeMillis();

            if( DFW_RET(retval, m_req->parseRequest(buffer, rsize)) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                    continue;
                }
                return DFW_RETVAL_D(retval);
            }

            m_request_count++;
            if( m_request_count == (uint64_t)-1 ){
                m_request_count = 1;
            }
            if( configure->isPrintRequestHeader() ){
                DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this
                       , "Request:: handle=%d\n%s", getHandle()
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
            if(m_bStop){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                                        , "Stop httpd thread. handle=%d"
                                        , getHandle());
            }

            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*60/* FIXME: */) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout read request. handle=%d"
                                        , getHandle());
            }

            int comp = 0;
            if( DFW_RET(retval, sendStream(&comp)) ){
                int rvalue = retval->value();
                if( rvalue == DFW_E_AGAIN || rvalue == DFW_E_TIMEOUT ){
                    sleep(0);
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

        m_ssTime = Time::currentTimeMillis();
        do{
            if(m_bStop){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                                        , "Stop httpd thread. handle=%d"
                                        , getHandle());
            }

            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-m_ssTime) > (1000*MAX_TIMEOUT) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0 
                                        , "Timeout response. handle=%d"
                                        , getHandle());
            }

            if( DFW_RET(retval, sendLocalFile()) ){
                switch(retval->value()){
                case DFW_E_AGAIN:
                case DFW_E_TIMEOUT:
                    usleep(20000);
                    continue;
                case DFW_T_CONTINUE:
                    continue;
                default:
                    return DFW_RETVAL_D(retval);
                }
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
            if(DFW_RET(retval,m_resp->appendHeader("Keep-Alive", "timeout=2400")))
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
            DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this
                  , "Response:: handle=%d\n%s", getHandle() 
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
        sp<Retval> retval;
        dfw_httpstatus_t status = DFW_HTTP_STATUS_0;
        String sModify;
        String sRange, sAcceptRange, sContentRange;
        String sStart;
        String sEnd;
        int minus = 0;
        uint64_t iStart, iEnd, iLength;
        uint64_t iFileSize = m_resp->m_iFileSize;

//DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "send local file ready");
        AutoLock _l(this);
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

#ifdef _WIN32
        int flags = O_RDONLY | O_BINARY;
#else
        int flags = O_RDONLY;
#endif

        if( DFW_RET(retval, m_resp->m_originFs->open(
                                             m_req->m_sFile.toChars()
                                             //m_req->m_sRequest.toChars()
                                            , flags)) ){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, m_resp->m_originFs->getContentType(
                                           m_resp->m_sFileContentType)) ){
        }

        Time::getDateStringRfc822(sModify, m_resp->m_iFileMTime);
        if( DFW_RET(retval, HttpdUtil::checkRangeBytes(m_req.get()
                                             , sRange, sAcceptRange
                                             , sStart, sEnd, &minus)) ){
                                             //, &pStart, &pEnd, &minus)) ){
            return DFW_RETVAL_D(retval);
        }

        switch( minus ){
        case 0 :
            status = DFW_HTTP_STATUS_200;
            iStart = 0;
            iEnd = iFileSize-1; //iEnd = m_resp->m_iFileSize-1;
            iLength = iFileSize; // iLength = m_resp->m_iFileSize;
            break;

        case 1 :
            status = DFW_HTTP_STATUS_206;
            if(sStart.empty()){
                iStart = 0;
            }else{
                iStart = ::atoll(sStart.toChars());
            }
            if(sEnd.empty()){
                iEnd = iFileSize-1;
            }else{
                iEnd = ::atoll(sEnd.toChars());
            }
            if(iFileSize <= iEnd){
                if( !m_bEnableResizeableFile ){
                    iEnd = iFileSize - 1;
                }else{
                    struct stat st; ::memset(&st, 0, sizeof(struct stat));
                    sp<OriginFs> fs = new OriginFs();
                    if( DFW_RET(retval, fs->ready(m_req->m_host)) ){
                        return DFW_RETVAL_D(retval);
                    }
                    if( DFW_RET(retval, fs->getattr(m_req->m_sRequest.toChars(), &st)) ){
                        if( retval->value()==DFW_E_NOENT){
                            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not found %s", m_req->m_sRequest.toChars());
                        }
                        return DFW_RETVAL_D(retval);
                    }
                    if(iFileSize != st.st_size){
                        uint64_t newSize = st.st_size;
                        DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this
                            , "Update Filesize: old-size=%llu, new-size=%lu"
                            , iFileSize, newSize);
                        m_resp->m_iFileSize = iFileSize = (size_t)st.st_size;
                    }else{
                        DFWLOG_C(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), this
                            , "Warring Filesize: old-size=%llu, range-end-postion=%llu"
                            , iFileSize, iEnd);
                    }
                }
            }
            iLength = iEnd - iStart + 1;

            sContentRange = String::format("bytes %llu-%llu/%llu"
                                 , iStart, iEnd, iFileSize); // , iStart, iEnd, m_resp->m_iFileSize);
            break;

        case 2 :
            status = DFW_HTTP_STATUS_206;
            if(sStart.empty()){
                iStart = iFileSize; //iStart = m_resp->m_iFileSize;
            }else{
                iStart = iFileSize - ::atoll(sEnd.toChars()); // iStart = m_resp->m_iFileSize - ::atoll(sEnd.toChars());
            }
            iEnd = iFileSize - 1; // iEnd = m_resp->m_iFileSize - 1;
            iLength = iEnd - iStart + 1;
            sContentRange = String::format("bytes %llu-%llu/%llu"
                                 , iStart, iEnd, iFileSize); // , iStart, iEnd, m_resp->m_iFileSize);
            break;

        case 3 :
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not support multipart. Range(%s):%d"
                       , sRange.toChars(), minus);

        case 4 :
            status = DFW_HTTP_STATUS_206;
            if(sStart.empty()){
                iStart = 0;
            }else{
                iStart = ::atoll(sStart.toChars());
            }
            iEnd = iFileSize - 1; // iEnd = m_resp->m_iFileSize - 1;
            iLength = iEnd - iStart + 1;
            sContentRange = String::format("bytes %llu-%llu/%llu"
                                 , iStart, iEnd, iFileSize); // , iStart, iEnd, m_resp->m_iFileSize);
            break;

        default:
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Unknown Range(%s):%d", sRange.toChars(), minus);
        } // end switch(minus)

        if( iEnd < iStart ){
            return DFW_RETVAL_NEW(DFW_ERROR, 0);
        }
        if( iFileSize <= iStart ){ // if( m_resp->m_iFileSize <= iStart ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "m_iFileSize:%llu, iStart:%llu"
                       , iFileSize, iStart); // , m_resp->m_iFileSize, iStart);
        }

        if( DFW_RET(retval, setResponse(status, (size_t)iLength, m_req->m_bKeepAlive)) ){
            return DFW_RETVAL_D(retval);
        }
        if(DFW_RET(retval,m_resp->appendHeader("Accept-Ranges","bytes"))){
            return DFW_RETVAL_D(retval);
        }
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

        if(m_bStop){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                                    , "Stop httpd thread. handle=%d"
                                    , getHandle());
        }

        switch(m_resp->m_iFileStatus){
        case 0 :
            if( DFW_RET(retval, sendLocalFile_ready()) ){
                return DFW_RETVAL_D(retval);
            }
            return DFW_RETVAL_NEW(DFW_T_CONTINUE, 0);

        case 1 :
          {
              int iscomplete = 0;
              if( DFW_RET(retval, sendStream(&iscomplete)) ){
                  return DFW_RETVAL_D(retval);
              }
              m_resp->m_iFileStatus = 2;
          }
          return DFW_RETVAL_NEW(DFW_T_CONTINUE, 0);

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

          if(m_req->m_sMethod.equals("HEAD")){
              m_resp->m_iFileStatus = 3;
              return DFW_RETVAL_NEW(DFW_T_CONTINUE, 0);
          }

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

              if( 0 == out_fsize){
                  return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                             , "Not read file."
                               " readable_size=%u, offset=%llu at %s"
                             , readable_size
                             , fileOffset
                             , m_req->m_sRequest.toChars());
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

          m_ssTime = Time::currentTimeMillis();

          if( retval.has() )
              return DFW_RETVAL_D(retval);
          return DFW_RETVAL_NEW(DFW_T_CONTINUE, 0);
        }

        case 3:
#if !defined(__APPLE__) && !defined(_WIN32)
          {
              int leftsize = 0;
              if( DFW_RET(retval,m_sock->getSendBufferLeftSize(&leftsize))){
                  return DFW_RETVAL_D(retval);
              }
              if( leftsize > 0 ){
                  return DFW_RETVAL_NEW(DFW_E_AGAIN, EAGAIN);
              }
          }
#endif
          return NULL;
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown send file status.");
    }

};

