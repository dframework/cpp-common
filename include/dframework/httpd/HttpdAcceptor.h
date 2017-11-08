#ifndef DFRAMEWORK_HTTPD_HTTPDACCEPTOR_H
#define DFRAMEWORK_HTTPD_HTTPDACCEPTOR_H

#include <dframework/net/ServerAccept.h>
#include <dframework/httpd/HttpdWorker.h>
#include <dframework/httpd/HttpdConfigure.h>

namespace dframework {

    class HttpdService;

    class HttpdAcceptor : public ServerAccept
    {
    private:
        sp<HttpdWorker> m_worker;
        sp<HttpdConfigure> m_configure;

    public:
        HttpdAcceptor();
        virtual ~HttpdAcceptor();

        virtual void stop();
        virtual sp<Retval> onAccept(sp<ClientSocket>& sock);
        inline virtual void onCleanup();

        inline void setWorker(sp<HttpdWorker>& worker){
            m_worker = worker; 
        }

        friend class HttpdService;
    };

};

#endif /* DFRAMEWORK_HTTPD_HTTPDACCEPTOR_H */

