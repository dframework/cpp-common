#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpRound.h>
#include <dframework/http/HttpReader.h>
#include <dframework/http/HttpChunked.h>
#include <dframework/http/HttpLocation.h>
#include <dframework/http/HttpAuth.h>
#include <dframework/util/MicroTimeDepth.h>
#include <dframework/log/Logger.h>
#ifndef _WIN32
# include <sys/socket.h>
#else
# include <winsock2.h>
#endif

namespace dframework {


    HttpConnection::HttpConnection(){
        m_socket = new Socket();
    }

    HttpConnection::~HttpConnection(){
    }

    sp<Retval> HttpConnection::query(sp<HttpQuery>& query){
        sp<Retval> retval;
        m_pQuery = query;
        if(DFW_RET(retval, connect())) return DFW_RETVAL_D(retval);
        if(DFW_RET(retval, request())) return DFW_RETVAL_D(retval);
        if( m_pQuery->isOnlyHead() )
            return NULL;
        if(DFW_RET(retval, response())) return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpConnection::request(sp<HttpQuery>& query){
        sp<Retval> retval;
        m_pQuery = query;
        if(DFW_RET(retval, connect())) return DFW_RETVAL_D(retval);
        if(DFW_RET(retval, request())) return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpConnection::connect(sp<HttpQuery>& query){
        sp<Retval> retval;
        m_pQuery = query;
        if(DFW_RET(retval, connect())) return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpConnection::onResponse(const char *b, dfw_size_t s)
    {
        if(m_pQuery.has()){
            sp<HttpRound> round = m_pQuery->getLastRound();
            if(round.has() && !round->m_bOnRecvContents){
                return NULL;
            }
            sp<HttpConnection> thiz = this;
            sp<Retval> retval;
            if(DFW_RET(retval, m_pQuery->onResponse(thiz, b, s)))
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpConnection::connect(){
        sp<Retval> retval;
        MicroTimeDepth dep;
        sp<HttpConnection> thiz = this; 
        sp<HttpRound> round = m_pQuery->getLastRound();

        if(!m_pQuery.has() && !round.has()){
            retval = DFW_RETVAL_NEW(DFW_E_INVAL,0);
            m_pQuery->onError(thiz, retval);
            return DFW_RETVAL_D(retval);
        }

        dep.start();

        if(m_socket.has() && m_socket->isConnect()){
            if( m_sLastUri.equals(round->m_sUri) ){
                //round->m_tConnectTime = 0; FIXME:
                return NULL;
            }
            m_socket->close();
            m_socket = NULL;
            m_socket = new Socket();
        }
        m_sLastUri = round->m_sUri;

        if(!m_socket.has())
            m_socket = new Socket();

        retval = connect_real(round);

        //dep.stop(&round->m_tConnectTime); // FIXME

        if(DFW_RETVAL_H(retval)){
            m_pQuery->onError(thiz, retval);
            return DFW_RETVAL_D(retval);
        }
 
        if(DFW_RET(retval, m_pQuery->onConnect(thiz)))
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    sp<Retval> HttpConnection::connect_real(sp<HttpRound>& round){
        sp<Retval> retval;
        int num = -1;
        int opentype = -1;
        int port = round->m_oUri.getPort();
        sp<Hostname::Result> ips;

        retval = DFW_RETVAL_NEW(DFW_E_HOST_NOT_FOUND,0);
        do{
            num++;
            ips = round->m_oHostname.getResult(num);
            if( !ips.has() ){
                if(retval.has())
                    return DFW_RETVAL_D(retval);
                return DFW_RETVAL_NEW(DFW_E_CONNECT,0);
            }

            const char *ip = ips->m_sIp.toChars();
            int atype = ips->m_iAddrType;
            if(atype!=opentype){
                if( DFW_RET(retval, m_socket->open(atype, SOCK_STREAM)) )
                    return DFW_RETVAL_D(retval);
                opentype = atype;
            }
            if(!DFW_RET(retval, m_socket->connectbyip(atype, ip, port, false)))
                return NULL;
        }while(true);
    }

    sp<Retval> HttpConnection::request(){
        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "request");

        sp<Retval> retval;
        sp<HttpRound> round = m_pQuery->getLastRound();

        if(DFW_RET(retval, request_real(round))){
            sp<HttpConnection> thiz = this; 
            m_pQuery->onError(thiz, retval);
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpConnection::request_real(sp<HttpRound>& round){
        sp<Retval> retval;
        int retry_count = 0;
        int retry_auth = 0;

        do{
            retry_count++;
            if( DFW_RET(retval, request_raw(round)) ){
                if( retry_count>m_pQuery->getMaxRetryRequestCount() )
                    return DFW_RETVAL_D(retval);
                if( retval->value() == DFW_E_PIPE ){
                    sp<Retval> retval2;
                    if(DFW_RET(retval2, connect()))
                        return DFW_RETVAL_D(retval);
                    continue;
                }else{
                    return DFW_RETVAL_D(retval);
                }
            }

            switch(round->m_iStatus){
                case DFW_HTTP_STATUS_300:
                case DFW_HTTP_STATUS_301:
                case DFW_HTTP_STATUS_302:
                case DFW_HTTP_STATUS_303:
                case DFW_HTTP_STATUS_305:
                case DFW_HTTP_STATUS_307:
                case DFW_HTTP_STATUS_308:
                case DFW_HTTP_STATUS_401:
                    if( m_pQuery->isOnlyHead() ){
                        return NULL;
                    }
                    break;
                default:
                    return NULL;
            }

            if( DFW_RET(retval, response(false)) )
                return DFW_RETVAL_D(retval);

            if( retry_count>m_pQuery->getMaxRetryRequestCount() )
                return DFW_RETVAL_NEW(DFW_E_MAX_REQUESTCOUNT, 0);

            if(DFW_HTTP_STATUS_401==round->m_iStatus){
                if(retry_auth>0)
                    return DFW_RETVAL_NEW(DFW_E_HTTPAUTH,0);

                HttpAuth auth;
                sp<HttpConnection> thiz = this;
                retval = auth.auth(thiz, m_pQuery);
                if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

                retval = m_pQuery->createRound(round->m_oUri);
                if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

                round = m_pQuery->getLastRound();
                retry_auth++;
            }else if(DFW_HTTP_STATUS_300<=round->m_iStatus
                         && round->m_iStatus<DFW_HTTP_STATUS_400){
                HttpLocation location;
                URI reUri(NULL);
                if( DFW_RET(retval, location.parse(
                        reUri, round->m_oUri, 
                        round->m_sLocation, m_socket->getIp())) ){
                    if(DFW_T_RECONNECT != retval->value()){
                        return DFW_RETVAL_D(retval);
                    }
                }
                if( DFW_RET(retval, m_pQuery->createRound(reUri)) )
                    return DFW_RETVAL_D(retval);

                bool bReconnect = false;
                if( reUri.getPort()==0 ) reUri.setPort(80);
                if( round->m_oUri.getPort()==0 ) round->m_oUri.setPort(80);
                if( (!reUri.getHost().equals( round->m_oUri.getHost()))
                        || reUri.getPort()!=round->m_oUri.getPort() ){
                    bReconnect = true;
                }

                round = m_pQuery->getLastRound();
                if(bReconnect){
                    if( DFW_RET(retval, connect_real(round)) )
                        return DFW_RETVAL_D(retval);
                }
            }else{
                return DFW_RETVAL_NEW(DFW_E_HTTP_REQUEST,0);
            }
        }while(true);
    }

    sp<Retval> HttpConnection::request_raw(sp<HttpRound>& round)
    {
        sp<Retval> retval;
        MicroTimeDepth dep;
        size_t sendlen = 0;

        dep.start();

        if(DFW_RET(retval, m_pQuery->makeRequestBuffer(round))){
            return DFW_RETVAL_D(retval);
        }

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID)
                       , "request_raw send : %s\n"
                       , round->m_sHeadBuffer.toChars());

        if(DFW_RET(retval, m_socket->send_wait(
                           &sendlen
                         , round->m_sHeadBuffer.toChars()
                         , round->m_sHeadBuffer.length()))){
            return DFW_RETVAL_D(retval);
        }

        //dep.stop(&round->m_tRequestTime); FIXME

        sp<HttpConnection> thiz = this;
        if(DFW_RET(retval, m_pQuery->onRequest(thiz)))
            return DFW_RETVAL_D(retval);

        if(DFW_RET(retval, readStatusAndHeader(round)))
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    sp<Retval> HttpConnection::readStatusAndHeader(sp<HttpRound>& round){
        sp<Retval> retval;
        MicroTimeDepth dep;

        dep.start();

        if(DFW_RET(retval, readLine(round)))
            return DFW_RETVAL_D(retval);

        if(DFW_RET(retval, round->parseStatusLine()))
            return DFW_RETVAL_D(retval);

        if(DFW_RET(retval, readResponseHeader(round)))
            return DFW_RETVAL_D(retval);

        //dep.stop(&round->m_tResponseHeaderTime); // FIXME

        if(m_pQuery->m_pHttpListener.has()){
            bool bCallOnStatus = true;
            if(round->m_bUseSpecialStatusCode){
                switch(round->m_iStatus){
                case DFW_HTTP_STATUS_100:
                case DFW_HTTP_STATUS_101:
                case DFW_HTTP_STATUS_102:
                case DFW_HTTP_STATUS_300:
                case DFW_HTTP_STATUS_301:
                case DFW_HTTP_STATUS_302:
                case DFW_HTTP_STATUS_303:
                case DFW_HTTP_STATUS_305:
                case DFW_HTTP_STATUS_307:
                case DFW_HTTP_STATUS_308:
                case DFW_HTTP_STATUS_401:
                    bCallOnStatus = false;
                    break;
                }
            }
            if(bCallOnStatus){
                sp<HttpConnection> thiz = this;
                if(DFW_RET(retval,m_pQuery->onStatus(
                        thiz, round->m_iStatus)))
                {
                    return DFW_RETVAL_D(retval);
                }
            }
        }

        return NULL;
    }

    sp<Retval> HttpConnection::readLine(sp<HttpRound>& round)
    {
        sp<Retval> retval;
        size_t size = 51200;
        size_t r_size = 0;
        size_t offset = 0;
        char buf[51232];
        char* test;
        int status;

        if(!DFW_RET(retval, round->readLine())) return NULL;

        do{
            if( DFW_RET(retval, m_socket->wait_recv()) )
                return DFW_RETVAL_D(retval);

            r_size = 0;
            if( DFW_RET(retval, m_socket->recv(buf+offset, &r_size, size)) ){
                if( (status = retval->value()) ){
                    if( DFW_E_AGAIN!=status && DFW_E_DISCONNECT!=status)
                        return DFW_RETVAL_D(retval);
                    if(r_size==0 && DFW_E_AGAIN==status){
                        continue;
                    }
                }
            }

            if( DFW_E_DISCONNECT==status){
                return DFW_RETVAL_NEW_MSG(DFW_E_DISCONNECT, 0
                           , "Disconnected for reading"
                             " in HttpConnection's readLine.");
            }

            size -= r_size;
            offset += r_size;
            buf[offset] = '\0';

            if( !(test = ::strstr(buf, "\r\n")) ){
                continue;
            }

            round->m_sRecvBuffer.append(buf, offset);
            return round->readLine();

        }while(true);
    }

    sp<Retval> HttpConnection::readResponseHeader(sp<HttpRound>& round){
        sp<Retval> retval;

        do{
            if(DFW_RET(retval, readLine(round)))
                return DFW_RETVAL_D(retval);
            if( round->m_sLineBuffer.empty() ) 
                return NULL;
            round->parseHeaderLine();
        }while(true);

        if(round->m_sCharset.empty() 
                && round->m_sContentType.equals("text"))
        {
            if(round->m_sContentSubType.equals("xml"))
                round->m_sCharset = "us-ascii";
            else
                round->m_sCharset = "ISO-8859-1";
        }

        return NULL;
    }

    sp<Retval> HttpConnection::response(){
        sp<Retval> retval;
        if( DFW_RET(retval, response(true)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpConnection::response(bool bComplete){
        sp<Retval> retval;
        sp<HttpRound> round = m_pQuery->getLastRound();
        round->m_bOnRecvContents = bComplete;

        if(DFW_RET(retval, response_real(round, bComplete))){
            if(DFW_T_CHUNKED_END==DFW_RETVAL_V(retval))
                return NULL;
            sp<HttpConnection> thiz = this;
            m_pQuery->onError(thiz, retval);
            return DFW_RETVAL_D(retval);
        }

        if(bComplete){
            sp<HttpConnection> thiz = this;
            if(DFW_RET(retval, m_pQuery->onComplete(thiz)))
                return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> HttpConnection::response_real(
            sp<HttpRound>& round, bool bComplete)
    {
        DFW_UNUSED(bComplete);
        sp<Retval> retval;

        const char* method = m_pQuery->getMethod();
        if( ::strlen(method)==4 && ::strstr(method, "HEAD")==method )
            return NULL;

        sp<HttpConnection> thiz = this;
        MicroTimeDepth dep;
        //FIXME: unused: dfw_size_t asize = 0;

        dep.start();

        if(round->m_bChunked){
            HttpChunked chunked;
            chunked.reset(thiz, round);                       
            if( DFW_RET(retval, chunked.readBlocks()) ){
                if(DFW_T_CHUNKED_END==DFW_RETVAL_V(retval))
                    return NULL;
                return retval;
            }
        }else {
            if( 0==round->m_uContentLength ) return NULL;
            HttpReader reader;
            reader.reset(thiz, round);                       
            if( DFW_RET(retval, reader.readBlocks()) )
                return DFW_RETVAL_D(retval);
        }

        //dep.stop(&round->m_tResponseContentTime); // FIXME

        return NULL;
    }

    sp<Retval> HttpConnection::read(unsigned* out, char* b, unsigned s){
        sp<Retval> retval;

        *out = 0;
        const char* method = m_pQuery->getMethod();
        if( ::strlen(method)==4 && ::strstr(method, "HEAD")==method ){
            return NULL;
        }

        sp<HttpConnection> thiz = this;
        sp<HttpRound> round = m_pQuery->getLastRound();

        if(round->m_bChunked){
            HttpChunked chunked;
            chunked.reset(thiz, round);                       
            if( DFW_RET(retval, chunked.read(out, b, s)) ){
                if(DFW_T_CHUNKED_END==DFW_RETVAL_V(retval))
                    return NULL;
                return retval;
            }
        }else {
            if( 0==round->m_uContentLength ) return NULL;
            HttpReader reader;
            reader.reset(thiz, round);
            if( DFW_RET(retval, reader.read(out, b, s)) )
                return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

};

