#ifndef DFRAMEWORK_HTTP_HTTPCONNECTION_H
#define DFRAMEWORK_HTTP_HTTPCONNECTION_H


#include <dframework/base/Retval.h>
#include <dframework/net/Socket.h>
#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpRound.h>


#ifdef __cplusplus
namespace dframework {

    class IHttpReader;

    class HttpConnection : public Object
    {
    public:
        friend class IHttpReader;

    private:
        sp<HttpQuery>   m_pQuery;
        sp<Socket>      m_socket;
        String          m_sLastUri;

    private:
        sp<Retval> readStatusAndHeader(sp<HttpRound>& round);
        sp<Retval> readLine(sp<HttpRound>& round);
        sp<Retval> readResponseHeader(sp<HttpRound>& round);

        sp<Retval> connect_real(sp<HttpRound>& round);
        sp<Retval> request_real(sp<HttpRound>& round);
        sp<Retval> request_raw(sp<HttpRound>& round);
        sp<Retval> response_real(sp<HttpRound>& round, bool bComplete);
        sp<Retval> response(bool bComplete);

    protected:
        sp<Retval> connect();
        sp<Retval> request();

    public:
        HttpConnection();
        virtual ~HttpConnection();

        inline bool isConnect() {
            return (m_socket.has() ? m_socket->isConnect() : false);
        }

        sp<Retval> query(sp<HttpQuery>& query); 
        sp<Retval> request(sp<HttpQuery>& query); 
        sp<Retval> connect(sp<HttpQuery>& query); 
        sp<Retval> response();
        sp<Retval> read(unsigned* out, char* b, unsigned s);

        sp<Retval> onResponse(const char *b, dfw_size_t s);

    };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPCONNECTION_H */

