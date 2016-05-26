#include <dframework/http/HttpDigest.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/http/HttpListener.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpRound.h>
#include <dframework/http/HttpHeader.h>
#include <dframework/util/MD5.h>
#include <dframework/util/Time.h>
#include <dframework/lang/Long.h>

namespace dframework {

    HttpDigest::HttpDigest(){
    }

    HttpDigest::~HttpDigest(){
    }

    /* static  */
    void HttpDigest::test(
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

    dfw_httpstatus_t HttpDigest::check(
        const char *value, const char *id, const char *pw)
    {
        DFW_UNUSED(pw);
        if(!value) return DFW_HTTP_STATUS_401;

        const char *enc = strcasestr(value, "Digest ");
        if(!enc) return DFW_HTTP_STATUS_401;
        enc = (value + 7);

        bool bLast = false;
        dfw_httpdigest_t ctx;
        do{
            int olen = 0;
            const char *temp = enc;
            int templen = String::indexOf(temp, ',');
            if(-1==templen){
                bLast = true;
                templen = ::strlen(temp);
            }

            while(temp[0]==' '||temp[0]=='\t'){ temp++; }

            if( 0==strncasecmp("uri", temp, 3) ){
                test(&(ctx.uri), &olen, temp, templen, 4);
            }else if( 0==strncasecmp("username", temp, 8) ){
                test(&(ctx.user), &olen, temp, templen, 9);
            }else if( 0==strncasecmp("nonce", temp, 5) ){
                test(&(ctx.nonce), &olen, temp, templen, 6);
            }else if( 0==strncasecmp("response", temp, 8) ){
                test(&(ctx.response), &olen, temp, templen, 9);
            }

            enc += templen + 1;
        }while(!bLast);

        if(ctx.user.empty() || ctx.nonce.empty() || ctx.response.empty()){
            return DFW_HTTP_STATUS_401;
        }

        if( ::strcmp(id, ctx.user.toChars())!=0 ){
            return DFW_HTTP_STATUS_401;
        }
       
        return DFW_HTTP_STATUS_401;
    }



    void HttpDigest::CvtHex(DFW_IN HASH Bin, DFW_OUT HASHHEX Hex) {
        unsigned short i;
        unsigned char j;

        for (i = 0; i < HASHLEN; i++) {
            j = (Bin[i] >> 4) & 0xf;
            if (j <= 9)
                Hex[i*2] = (j + '0');
            else
                Hex[i*2] = (j + 'a' - 10);
            j = Bin[i] & 0xf;
            if (j <= 9)
                Hex[i*2+1] = (j + '0');
            else
                Hex[i*2+1] = (j + 'a' - 10);
        };
        Hex[HASHHEXLEN] = '\0';
    };

    /* calculate H(A1) as per spec */
    void HttpDigest::DigestCalcHA1(
            DFW_IN const char * pszAlg,
            DFW_IN const char * pszUserName,
            DFW_IN const char * pszRealm,
            DFW_IN const char * pszPassword,
            DFW_IN const char * pszNonce,
            DFW_IN const char * pszCNonce,
            DFW_OUT HASHHEX SessionKey)
    {
        MD5 md5;
        HASH HA1;

        md5.init();
        md5.update(pszUserName, strlen(pszUserName));
        md5.update(":", 1);
        md5.update(pszRealm, strlen(pszRealm));
        md5.update(":", 1);
        md5.update(pszPassword, strlen(pszPassword));
        md5.finalization(HA1);
        if (strcmp(pszAlg, "md5-sess") == 0) {
            md5.init();
            md5.update(HA1, HASHLEN);
            md5.update(":", 1);
            md5.update(pszNonce, strlen(pszNonce));
            md5.update(":", 1);
            md5.update(pszCNonce, strlen(pszCNonce));
            md5.finalization(HA1);
        };
        CvtHex(HA1, SessionKey);
    }

    /* calculate request-digest/response-digest as per HTTP Digest spec */
    void HttpDigest::DigestCalcResponse(
            DFW_IN HASHHEX HA1,           /* H(A1) */
            DFW_IN const char * pszNonce,       /* nonce from server */
            DFW_IN const char * pszNonceCount,  /* 8 hex digits */
            DFW_IN const char * pszCNonce,      /* client nonce */
            DFW_IN const char * pszQop,         /* qop-value: "", "auth", "auth-int" */
            DFW_IN const char * pszMethod,      /* method from the request */
            DFW_IN const char * pszDigestUri,   /* requested URL */
            DFW_IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
            DFW_OUT HASHHEX Response      /* request-digest or response-digest */
    )
    {
        MD5 md5;
        HASH HA2;
        HASH RespHash;
        HASHHEX HA2Hex;

        // calculate H(A2)
        md5.init();
        md5.update(pszMethod, strlen(pszMethod));
        md5.update(":", 1);
        md5.update(pszDigestUri, strlen(pszDigestUri));
        if (strcmp(pszQop, "auth-int") == 0) {
            md5.update(":", 1);
            md5.update(HEntity, HASHHEXLEN);
        };
        md5.finalization(HA2);
        CvtHex(HA2, HA2Hex);

        // calculate response
        md5.init();
        md5.update(HA1, HASHHEXLEN);
        md5.update(":", 1);
        md5.update(pszNonce, strlen(pszNonce));
        md5.update(":", 1);
        if (*pszQop) {
            md5.update(pszNonceCount, strlen(pszNonceCount));
            md5.update(":", 1);
            md5.update(pszCNonce, strlen(pszCNonce));
            md5.update(":", 1);
            md5.update(pszQop, strlen(pszQop));
            md5.update(":", 1);
        };
        md5.update(HA2Hex, HASHHEXLEN);
        md5.finalization(RespHash);
        CvtHex(RespHash, Response);
    }

    String HttpDigest::makeNonce(){
        MD5 md5;
        HASH hash;
        HASHHEX hex;
        String nonce = String::format("%llu", Time::currentTimeMillis());
        md5.init();
        md5.update(nonce.toChars(), nonce.length());
        md5.finalization(hash);
        CvtHex(hash, hex);
        nonce.set(hex, HASHHEXLEN);
        return nonce;
    }

    sp<Retval> HttpDigest::makeAuthorization(
            sp<HttpConnection>& conn, sp<HttpQuery>& pQ)
    {
        sp<Retval> retval;
        String sUser, sPass;
        sp<HttpRound> pR = pQ->getLastRound();
        sp<HttpHeader> header = pR->m_responseHeader;

        sp<NamedValue> nv = header->getHeader("WWW-Authenticate");
        if(!nv.has()) return DFW_RETVAL_NEW(DFW_E_NO_WWW_AUTHENTICATE, 0);
        const char *value = nv->m_sValue.toChars();
        const char *method = pQ->getMethod();
        const char *path = pR->m_oUri.getPath().toChars();
        const char *digest = strcasestr(value, "Digest ");
        if(!digest) return DFW_RETVAL_NEW(DFW_E_NO_HTTP_AUTH_TYPE, 0);
        digest = (digest + 7);

        if(DFW_RET(retval, pQ->onAuth(conn, sUser, sPass)))
            return DFW_RETVAL_D(retval);

        bool bLast = false;
        dfw_httpdigest_t ctx;

        do{
            int olen = 0;
            const char *temp = digest;
            int templen = String::indexOf(temp, ',');
            if(-1==templen){
                bLast = true;
                templen = ::strlen(temp);
            }

            while(temp[0]==' '||temp[0]=='\t'){ temp++; }
            if( 0==strncasecmp("realm", temp, 5) ){
                test(&(ctx.realm), &olen, temp, templen, 6);
            }else if( 0==strncasecmp("nonce", temp, 5) ){
                test(&(ctx.nonce), &olen, temp, templen, 6);
            }else if( 0==strncasecmp("algorithm", temp, 9) ){
                test(&(ctx.algorithm), &olen, temp, templen, 10);
            }else if( 0==strncasecmp("qop", temp, 3) ){
                test(&(ctx.qop), &olen, temp, templen, 4);
            }else if(!bLast){
                digest += (templen + 2);
                continue;
            }
            if(!olen){
                bLast = true;
            }
            digest += olen;
        }while(!bLast);

        String nc = String::format("%08d", 1);
        String cnonce = "0a4f113b";

        HASHHEX HA1;
        HASHHEX HA2;
        HASHHEX Response;

        DigestCalcHA1(
                  ctx.algorithm.toChars()
                , sUser.toChars()
                , ctx.realm.toChars()
                , sPass.toChars()
                , ctx.nonce.toChars()
                , cnonce.toChars()
                , HA1);

        DigestCalcResponse(
                  HA1
                , ctx.nonce.toChars()
                , nc.toChars()
                , cnonce.toChars()
                , ctx.qop.toChars()
                , method
                , path
                , HA2
                , Response);

        String text = String::format(
                "Digest "
                "username=\"%s\", "
                "realm=\"%s\", "
                "qop=\"%s\", "
                "algorithm=\"MD5\", "
                "uri=\"%s\", "
                "nonce=\"%s\", "
                "nc=%s, "
                "cnonce=\"%s\", "
                "response=\"%s\""
              , sUser.toChars()
              , ctx.realm.toChars()
              , ctx.qop.toChars()
              , pR->m_oUri.getPath().toChars()
              , ctx.nonce.toChars()
              , nc.toChars()
              , cnonce.toChars()
              , Response
        );

        pQ->m_requestHeader->appendHeader("Authorization", text.toChars());

        return NULL;
    }

    sp<Retval> HttpDigest::getAuthenticationInfo(
            /*IN*/  const char *value, 
            /*OUT*/ dfw_httpauth_t *ctx)
    {
        bool bLast = false;
        do{
            int olen = 0;
            const char *temp = value;
            int templen = String::indexOf(temp, ',');
            if(-1==templen){
                bLast = true;
                templen = ::strlen(temp);
            }

            while(temp[0]==' '||temp[0]=='\t'){ temp++; }

            if( 0==strncasecmp("rspauth", temp, 7) ){
                test(&(ctx->rspauth), &olen, temp, templen, 8);
            }else if( 0==strncasecmp("cnonce", temp, 6) ){
                test(&(ctx->cnonce), &olen, temp, templen, 7);
            }else if( 0==strncasecmp("nc", temp, 2) ){
                test(&(ctx->nc), &olen, temp, templen, 3);
            }else if( 0==strncasecmp("qop", temp, 3) ){
                test(&(ctx->qop), &olen, temp, templen, 4);
            }else if(!bLast){
                value += (templen + 2);
                continue;
            }
            if(!olen){
                bLast = true;
            }
            value += olen;
        }while(!bLast);

        ctx->unc = Long::parseLong(ctx->nc);

        return NULL;
    }

};

