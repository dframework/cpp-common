#ifndef DFRAMEWORK_HTTPD_HTTPDTHREAD_H
#define DFRAMEWORK_HTTPD_HTTPDTHREAD_H

#include <dframework/base/Thread.h>
#include <dframework/net/ServerAccept.h>
#include <dframework/httpd/HttpdConfigure.h>
#include <dframework/httpd/HttpdClient.h>


#ifdef __cplusplus
namespace dframework {

    class HttpdAcceptor;

    class HttpdThread : public Thread
    {
    private:
        bool m_bLive;
        sp<HttpdConfigure> m_configure;
        sp<HttpdClient>    m_client;

        sp<Retval> run_2();

    public:
        HttpdThread();
        virtual ~HttpdThread();

        virtual void run();
        virtual void stop();

        bool isLive();
        sp<Retval> ready(sp<HttpdConfigure>& configure, sp<ClientSocket>& sock);

    private:
        sp<Retval> request();

        friend class HttpdAcceptor;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDTHREAD_H */

