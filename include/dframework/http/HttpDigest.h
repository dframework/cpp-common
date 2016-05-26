#ifndef DFRAMEWORK_HTTP_HTTPDIGEST_H
#define DFRAMEWORK_HTTP_HTTPDIGEST_H


#include <dframework/base/Retval.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/lang/String.h>



#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpQuery;

    class HttpDigest
    {
    public:
        static const int HASHLEN = 16;
        static const int HASHHEXLEN = 32;

        struct dfw_httpdigest_t {
            String user;
            String pass;
            String realm;
            String nonce;
            String uri;
            String response;
            String algorithm;
            String qop;
        };
        struct dfw_httpauth_t {
            String rspauth;
            String cnonce;
            String nc;
            String qop;
            dfw_ulong_t unc;
        };
        typedef struct dfw_httpdigest_t dfw_httpdigest_t;
        typedef struct dfw_httpauth_t dfw_httpauth_t;
        typedef unsigned char HASH[HASHLEN];
        typedef char HASHHEX[HASHHEXLEN+1];


    public:
        HttpDigest();
        virtual ~HttpDigest();

        static void test(
                String *out, int *o_len, 
                const char *str, int len, int prefix);
        dfw_httpstatus_t check(
                const char *headervalue, const char *id, const char *pw);

        /* This is rfc2617 */
        void CvtHex(DFW_IN HASH Bin, DFW_OUT HASHHEX Hex);
        /* This is rfc2617 */
        void DigestCalcHA1(
              DFW_IN const char * pszAlg,
              DFW_IN const char * pszUserName,
              DFW_IN const char * pszRealm,
              DFW_IN const char * pszPassword,
              DFW_IN const char * pszNonce,
              DFW_IN const char * pszCNonce,
              DFW_OUT HASHHEX SessionKey
        );
        /* This is rfc2617 */
        void DigestCalcResponse(
              DFW_IN HASHHEX HA1,                 /* H(A1) */
              DFW_IN const char * pszNonce,       /* nonce from server */
              DFW_IN const char * pszNonceCount,  /* 8 hex digits */
              DFW_IN const char * pszCNonce,   /* client nonce */
              DFW_IN const char * pszQop,      /* qop-value: "", "auth", "auth-int" */
              DFW_IN const char * pszMethod,   /* method from the request */
              DFW_IN const char * pszDigestUri,/* requested URL */
              DFW_IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
              DFW_OUT HASHHEX Response      /* request-digest or response-digest */
        );

        String makeNonce();
        sp<Retval> makeAuthorization(
                       sp<HttpConnection>& conn, sp<HttpQuery>& pQ);
        sp<Retval> getAuthenticationInfo(
                /*IN*/  const char *value, 
                /*OUT*/ dfw_httpauth_t *ctx);

    };


};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPDIGEST_H */

