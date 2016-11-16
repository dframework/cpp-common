#include <dframework/httpd/HttpRequest.h>
#include <dframework/httpd/HttpdUtil.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpRequest::HttpRequest(){
        m_cstatus = HTTPD_CSTATUS_READY;
        m_bIfRange = false;
        m_bKeepAlive = false;
        m_bCacheControl = false;
        m_iCacheControlMaxAge = 0;
        m_iUpgradeInsecureRequests = 0;
    }

    HttpRequest::~HttpRequest(){
    }

    sp<Retval> HttpRequest::addContext(const char* name, const char* value){
        sp<Retval> retval;
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name, new String(value));
        if( DFW_RET(retval, m_aContexts.insert(no)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpRequest::addContext(const char* name, sp<Object> value){
        sp<Retval> retval;
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name, value);
        if( DFW_RET(retval, m_aContexts.insert(no)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<NamedObject> HttpRequest::removeContext(const char* name){
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name);
        return m_aContexts.remove(no);
    }

    sp<NamedObject> HttpRequest::getContext(const char* name){
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name);
        return m_aContexts.get(no);
    }

    sp<NamedObject> HttpRequest::getContext(unsigned position){
        AutoLock _l(&m_aContexts);
        return m_aContexts.get(position);
    }

    sp<Object> HttpRequest::getContextObject(const char* name){
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name);
        sp<NamedObject> search = m_aContexts.get(no);
        if( search.has() ){
            return search->m_object;
        }
        return NULL;
    }

    sp<String> HttpRequest::getContextString(const char* name){
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name);
        sp<NamedObject> search = m_aContexts.get(no);
        if( search.has() ){
            sp<String> rs = search->m_object;
            return rs;
        }
        return NULL;
    }

    int HttpRequest::getContextInt(const char* name, int defaultVal){
        AutoLock _l(&m_aContexts);
        sp<NamedObject> no = new NamedObject(name);
        sp<NamedObject> search = m_aContexts.get(no);
        if( search.has() ){
            sp<Integer> rs = search->m_object;
            return rs->value();
        }
        return defaultVal;
    }

    sp<Retval> HttpRequest::parseRequestReady(){
        sp<Retval> retval;

        String sTest;
        String sQuery;
        char* test;
        const char* buf = m_sBuffer.toChars();
        if( !(test = ::strstr((char*)buf, "\r\n")) ){
            return DFW_RETVAL_NEW(DFW_E_AGAIN, 0);
        }

        m_sRequestLine.set(buf, test-buf);
        buf = m_sRequestLine.toChars();

        char* blank = ::strstr((char*)buf, " ");
        if( !blank || (blank==buf) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, HTTPD_STATUS_500
                                    , "Wrong request line : %s"
                                    , m_sRequestLine.toChars());
        }

        m_sMethod.set(buf, blank-buf);
        buf = blank+1;

        blank = ::strrchr((char*)buf, ' ');
        if( !blank || (blank==buf) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, HTTPD_STATUS_500
                                  , "Wrong request line : %s"
                                  , m_sRequestLine.toChars());
        }

        sTest.set(buf, blank-buf);
        buf = blank+1;

        if( DFW_RET(retval, HttpdUtil::urldecode(sQuery, sTest.toChars())) ){
            return DFW_RETVAL_D(retval);
        }

        const char* query = sQuery.toChars();
        m_sRequest.set(query);
        char* query_q = ::strstr((char*)query, "?");
        if(query_q) {
            m_sQuery.set(query_q+1);
            query_q[0]='\0';
        }

        m_sFile.set(query);
        unsigned extindex = m_sFile.lastIndexOf('.');
        if(extindex!=(unsigned)-1){
            m_sFileExt.set(query+extindex+1);
        }

        blank = ::strstr((char*)buf, "/");
        if( !blank || (blank==buf) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, HTTPD_STATUS_500
                            , "Wrong request line : [%s], method=[%s], query=[%s], buf=%s"
                            , m_sRequestLine.toChars()
                            , m_sMethod.toChars()
                            , m_sQuery.toChars()
                            , buf
                        );
        }
        m_sProtocol.set(buf, blank-buf);
        buf = blank+1;

        m_sVersion.set(buf);
        m_sBuffer.shift(test-m_sBuffer.toChars()+2);
        m_cstatus = HTTPD_CSTATUS_HEADER;

        return NULL;
    }

    sp<Retval> HttpRequest::parseRequestHeaders(){
        sp<Retval> retval;

        String sName, sVal;
        const char* buf = m_sBuffer.toChars();
        char* test = ::strstr((char*)buf, "\r\n");
        if( !test ) {
            return DFW_RETVAL_NEW(DFW_E_AGAIN, 0);
        }

        if( test == buf ) {
            m_sBuffer.shift(2);
            m_cstatus = HTTPD_CSTATUS_BODY;
            return NULL;
        }

        char* colon = ::strstr((char*)buf, ":");
        if( !colon || (colon==buf) ){
            String sError;
            sError.set(buf, test-buf);
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, HTTPD_STATUS_500
                                , "Wrong request header : %s"
                                , sError.toChars());
        }

        sName.set(buf, colon-buf);
        buf = colon+1;
        sVal.set(buf, test-buf);
        m_sBuffer.shift(test-m_sBuffer.toChars()+2);
        sName.trim();
        sVal.trim();

        if( DFW_RET(retval, appendHeader(sName, sVal)) ){
            return DFW_RETVAL_D_SET_ERR(retval, HTTPD_STATUS_500);
        }

        if( sName.equals("Host") ){
            m_sHost = sVal;
        }else if( sName.equals("User-Agent") ){
            m_sUserAgent = sVal;
        }else if( sName.equals("Connection") ){
            m_sConnection = sVal;
            if(sVal.equals("keep-alive"))
                m_bKeepAlive = true;
            else if(sVal.equals("close"))
                m_bKeepAlive = false;
        }else if( sName.equals("Accept-Language") ){
            m_sAcceptLanguage = sVal;
        }else if( sName.equals("Accept-Encoding") ){
            m_sAcceptEncoding = sVal;
        }else if( sName.equals("If-Modified-Since") ){
            m_sIfModifiedSince = sVal;
        }else if( sName.equals("If-None-Match") ){
            m_sIfNoneMatch = sVal;
        }else if( sName.equals("If-Range") ){
            m_sIfRange = sVal;
        }else if( sName.equals("Cache-Control") ){
            m_bCacheControl = true;
            const char* val = sVal.toChars();
            int index = sVal.indexOf('=');
            if(index>0){
                if( strstr(val, "max-age") == 0 ){
                    m_iCacheControlMaxAge = Integer::parseInt(val+index+1);
                }
            }
        }else if( sName.equals("Upgrade-Insecure-Requests") ){
            m_iUpgradeInsecureRequests = Integer::parseInt(sVal.toChars());
        }

        return DFW_RETVAL_NEW(DFW_OK,0);
    }

    sp<Retval> HttpRequest::parseRequest(const char* buf, size_t rsize){
        sp<Retval> retval;

        if(rsize == 0){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Read size is zero.");
        }

        m_sBuffer.append(buf, rsize);
        m_sRequestHeaders.append(buf, rsize);

        switch(m_cstatus){
        case HTTPD_CSTATUS_READY:
            if( DFW_RET(retval, parseRequestReady()) ){
                return DFW_RETVAL_D(retval);
            }

        case HTTPD_CSTATUS_HEADER:
            do{
                if( DFW_RET(retval, parseRequestHeaders()) ){
                    if(retval->value()==DFW_OK)
                        continue;
                    return DFW_RETVAL_D(retval);
                }
                break;
            }while(true);

        case HTTPD_CSTATUS_BODY:
            return NULL;
        }

        return NULL;
    }

};

