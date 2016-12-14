#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpClient.h>
#include <dframework/http/HttpMakeClientBuffer.h>
#include <dframework/http/types.h>
#include <dframework/log/Logger.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(HttpQuery);

    HttpQuery::HttpQuery() {
        DFW_SAFE_ADD(HttpQuery, l);

        m_iBlockSize = HttpClient::MAX_BLOCK_SIZE;
        m_iMaxRetryRequestCount = HttpClient::MAX_RETRY_REQUEST;
        m_bOnlyHead = false;
        m_iVirtualPort = 0;
        m_sUserAgent = HttpClient::USER_AGENT;

        m_readerBuffer = NULL;
        m_readerBufferSize = 0;
        m_requestHeader = new HttpHeader();
    }

    HttpQuery::~HttpQuery(){
        DFW_SAFE_REMOVE(HttpQuery, l);
        close();
    }

    void HttpQuery::clear(){
        m_pFirstRound = NULL;
        m_pLastRound = NULL;
        m_aRoundList.clear();
        m_aFormDataList.clear();
        m_requestHeader->clear();
    }

    void HttpQuery::close(){
        if( m_conn.has() )
            m_conn = NULL;
    }

    // --------------------------------------------------------------

    sp<Retval> HttpQuery::query(const URI& uri, const char* method){
        sp<Retval> retval;
        sp<HttpQuery> thiz = this;
        if( !m_conn.has() )
            m_conn = new HttpConnection();

        if( method ){
            m_sMethod = method;
        }else{
            m_sMethod = "GET";
        }

        if( DFW_RET(retval, createRound(uri)) )
            return DFW_RETVAL_D(retval);

        m_pFirstRound = m_pLastRound;
        m_readerSavedBuffer = NULL;

        return m_conn->query(thiz);
    }

    sp<Retval> HttpQuery::query(const char* uri, const char* method){
        sp<Retval> retval;
        URI oUri = uri;
        return DFW_RET_C(retval, query(oUri, method));
    }

    // --------------------------------------------------------------

    sp<Retval> HttpQuery::request(const URI& uri, const char* method){
        sp<Retval> retval;
        sp<HttpQuery> thiz = this;
        if( !m_conn.has() )
            m_conn = new HttpConnection();

        if( method ){
            m_sMethod = method;
        }else{
            m_sMethod = "GET";
        }

        if( DFW_RET(retval, createRound(uri)) )
            return DFW_RETVAL_D(retval);

        m_pFirstRound = m_pLastRound;
        m_readerSavedBuffer = NULL;

        return m_conn->request(thiz);
    }

    sp<Retval> HttpQuery::request(const char* uri, const char* method){
        sp<Retval> retval;
        URI oUri = uri;
        return DFW_RET_C(retval, request(oUri, method));
    }

    sp<Retval> HttpQuery::read(unsigned* outsize, char* buf, unsigned size){
        sp<Retval> retval;

#if 0
        if( m_readerSavedBuffer.length() >= size ){
            *outsize = size;
            ::memcpy(buf, m_readerSavedBuffer.toChars(), size);
            m_readerSavedBuffer.shift(size);
            return NULL;
        }

        m_readerOutSize = outsize;
        m_readerBuffer = buf;
        m_readerBufferSize = size;
        return DFW_RET_C(retval, m_conn->response());
#else
        if( DFW_RET(retval, m_conn->read(outsize, buf, size)) )
            return DFW_RETVAL_D(retval); 
        return NULL;
#endif
    }

    sp<Retval> HttpQuery::response(){
        sp<Retval> retval;
        *m_readerOutSize = 0;
        m_readerBuffer = NULL;
        m_readerBufferSize = 0;
        m_readerSavedBuffer = NULL;
        return DFW_RET_C(retval, m_conn->response());
    }

    // --------------------------------------------------------------

    sp<Retval> HttpQuery::createRound(const URI& uri){
        sp<Retval> retval;

        int round = 0;
        if( m_pLastRound.has() ){
            round = m_pLastRound->m_iRound;
        }

        m_pLastRound = new HttpRound(++round);
        m_aRoundList.insert(m_pLastRound);

        if( DFW_RET(retval, m_pLastRound->ready(uri)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void HttpQuery::setUserAgent(const char *agent){
        m_sUserAgent.set(agent, ::strlen(agent));
    }

    void HttpQuery::appendUserAgent(const char *agent){
        if(!m_sUserAgent.empty()){
            m_sUserAgent.append(" ", 1);
        }
        m_sUserAgent.append(agent, strlen(agent));
    }

    sp<Retval> HttpQuery::makeRequestBuffer(sp<HttpRound>& round){
        sp<Retval> retval;
        sp<HttpQuery> thiz = this;
        HttpMakeClientBuffer make;
        return DFW_RET_C(retval, make.make(thiz, round));
    }

    sp<Retval> HttpQuery::onAuth(
            sp<HttpConnection>& conn, String &user, String& pass)
    {
        sp<Retval> retval;
        if(m_pHttpListener.has()){
            retval = m_pHttpListener->onAuth(conn, user, pass);
            if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
        }
        if( user.empty() || pass.empty() ){
            sp<HttpRound> round = m_aRoundList.get(0);
            if(round.has()){
                user = round->m_oUri.getUser();
                pass = round->m_oUri.getPass();
            }
        }
        if( user.empty() || pass.empty() ){
            return DFW_RETVAL_NEW(DFW_E_HTTPAUTH,0);
        }
        return NULL;
    }

    sp<Retval> HttpQuery::addFormData(const char* name, const char* val){
        sp<Retval> retval;
        sp<HttpFormData> hfd = new HttpFormData(name, val);
        if( DFW_RET(retval, m_aFormDataList.insert(hfd)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpQuery::onResponse(sp<HttpConnection>& c
                                   , const char *b, dfw_size_t s)
    {
        if( m_readerBuffer && m_readerBufferSize>0 ){
            if(0==s){
                *m_readerOutSize = m_readerSavedBuffer.length();
                ::memcpy(m_readerBuffer, m_readerSavedBuffer.toChars()
                       , m_readerSavedBuffer.length());
                m_readerSavedBuffer = NULL;
                return DFW_RETVAL_NEW(DFW_OK, RESPONSE_TYPE_READED);
            }

            m_readerSavedBuffer.append(b,s);
            if( m_readerSavedBuffer.length() >= m_readerBufferSize ){
                *m_readerOutSize = m_readerBufferSize;
                ::memcpy(m_readerBuffer, m_readerSavedBuffer.toChars()
                       , m_readerBufferSize);
                m_readerSavedBuffer.shift(m_readerBufferSize);
                return DFW_RETVAL_NEW(DFW_OK, RESPONSE_TYPE_READING);
            }

            return NULL;
        }
        return (m_pHttpListener.has()
                   ? m_pHttpListener->onResponse(c, b, s) : NULL);
    }


};

