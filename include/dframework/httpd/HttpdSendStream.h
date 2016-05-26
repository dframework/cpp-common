#ifndef DFRAMEWORK_HTTPD_HTTPDSENDSTEAM_H
#define DFRAMEWORK_HTTPD_HTTPDSENDSTEAM_H

#include <dframework/httpd/HttpdSender.h>
#include <dframework/httpd/HttpdConfigure.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdService;

    class HttpdSendStream : public HttpdSender
    {
    private:
        bool m_bLive;
        sp<HttpdConfigure> m_configure;

    public:
        HttpdSendStream();
        virtual ~HttpdSendStream();

        virtual void run();
        virtual void stop();

    private:
        sp<Retval> sendPackets();

    public:
        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDSENDSTEAM_H */

