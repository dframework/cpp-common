#include <dframework/http/HttpBasic.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpRound.h>
#include <dframework/http/HttpHeader.h>
#include <dframework/util/Base64.h>


namespace dframework {

    HttpBasic::HttpBasic(){
    }

    HttpBasic::~HttpBasic(){
    }

    /* static  */
    void HttpBasic::test(
            String *out, int *o_len, const char *str, int slen, int prefix)
    {
        char *test = (char*)(str + prefix);
        int len = ::strlen(test);
        slen -= prefix;
        if(len>slen) len=slen;
        if(len<=2){
            *o_len = prefix+len;
            out->set(test, len);
        }else if(test[0]!='\"' || test[len-1]!='\"'){
            *o_len = prefix+len;
            out->set(test, len);
        }else{
            *o_len = prefix+len + 2;
            out->set(test+1, len-2);
        }
    }

    sp<Retval> HttpBasic::makeAuthorization(
            sp<HttpConnection>& conn, sp<HttpQuery>& pQ)
    {
        sp<Retval> retval;
        String sUser, sPass;
        sp<HttpRound> pR = pQ->getLastRound();
        sp<HttpHeader> header = pR->m_responseHeader;

        sp<NamedValue> nv = header->getHeader("WWW-Authenticate");
        if(!nv.has()) return DFW_RETVAL_NEW(DFW_E_NO_WWW_AUTHENTICATE,0);
        const char *value = nv->m_sValue.toChars();
        const char *basic = ::strcasestr(value, "Basic ");
        if(!basic) return DFW_RETVAL_NEW(DFW_E_NO_HTTP_AUTH_TYPE, 0);
        basic = (basic + 6);

        if(DFW_RET(retval, pQ->onAuth(conn, sUser, sPass)))
            return DFW_RETVAL_D(retval);

        bool bLast = false;
        dfw_httpbasic_t ctx;

        do{
            int olen = 0;
            const char *temp = basic;
            int templen = String::indexOf(temp, ',');
            if(-1==templen){
                bLast = true;
                templen = ::strlen(temp);
            }

            while(temp[0]==' '||temp[0]=='\t'){ temp++; }
            if( 0==strncasecmp("realm", temp, 5) ){
                test(&(ctx.realm), &olen, temp, templen, 6);
            }else if(!bLast){
                basic += (templen + 2);
                continue;
            }
            if(!olen){
                bLast = true;
            }
            basic += olen;
        }while(!bLast);

        Base64 base;

        String temp = String::format("%s:%s"
                          , sUser.toChars(), sPass.toChars());
        String enc = base.encode(temp.toBytes(), temp.length());
        String text = String::format("Basic %s", enc.toChars());

        pQ->m_requestHeader->appendHeader("Authorization", text.toChars());

        return NULL;
    }

};

