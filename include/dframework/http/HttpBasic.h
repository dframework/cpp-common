#ifndef DFRAMEWORK_HTTP_HTTPBASIC_H
#define DFRAMEWORK_HTTP_HTTPBASIC_H


#include <dframework/base/Retval.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/lang/String.h>


#define HASHLEN 16
#define HASHHEXLEN 32
#define IN
#define OUT


#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpQuery;

    class HttpBasic
    {
    public:
        struct dfw_httpbasic_t {
            String user;
            String pass;
            String realm;
            String nonce;
            String uri;
            String response;
            String algorithm;
            String qop;
        };
        typedef struct dfw_httpbasic_t dfw_httpbasic_t;

    private:

    public:
        HttpBasic();
        virtual ~HttpBasic();

        static void test(
                String *out, int *o_len, 
                const char *str, int len, int prefix);

        sp<Retval> makeAuthorization(sp<HttpConnection>& conn, sp<HttpQuery>& pQ);

    };


};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPBASIC_H */

