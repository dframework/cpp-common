#include <dframework/http/HttpAuth.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpRound.h>
#include <dframework/http/HttpHeader.h>
#include <dframework/http/HttpDigest.h>
#include <dframework/http/HttpBasic.h>


namespace dframework {


    HttpAuth::HttpAuth(){
    }

    HttpAuth::~HttpAuth(){
    }

    dfw_httpauthtype_t HttpAuth::getType(const char *p){
        while(p && (' '==*p||'\t'==*p)) p++;
        if(::strncmp(p, "Basic ", 6)==0){
            return DFW_HTTPAUTHTYPE_BASIC;
        }else if(::strncmp(p, "Digest ", 7)==0){
            return DFW_HTTPAUTHTYPE_DIGEST;
        }
        return DFW_HTTPAUTHTYPE_UNKNOWN;
    }

    sp<Retval> HttpAuth::auth(sp<HttpConnection>& conn
                            , sp<HttpQuery>& pQ)
    {
        sp<Retval> retval;
        sp<HttpRound> pR = pQ->getLastRound();
        sp<HttpHeader> header = pR->m_responseHeader;

        sp<NamedValue> nv = header->getHeader("WWW-Authenticate");
        if(!nv.has()) return DFW_RETVAL_NEW(DFW_ERROR, 0);
        const char *value = nv->m_sValue.toChars();
        if(!value) return DFW_RETVAL_NEW(DFW_ERROR, 0);

        dfw_httpauthtype_t type = getType(value);
        switch(type){
        case DFW_HTTPAUTHTYPE_BASIC:
            {
                HttpBasic basic;
                return basic.makeAuthorization(conn,pQ);
            }
        case DFW_HTTPAUTHTYPE_DIGEST:
            {
                HttpDigest digest;
                return digest.makeAuthorization(conn,pQ);
            }
        default:
            break;
        }

        return DFW_RETVAL_NEW(DFW_E_NO_HTTP_AUTH_TYPE, 0);
    }

};

