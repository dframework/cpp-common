#ifndef DFRAMEWORK_HTTPD_HTTPDSENDLOCALFILE_H
#define DFRAMEWORK_HTTPD_HTTPDSENDLOCALFILE_H

#include <dframework/httpd/HttpdSender.h>
#include <dframework/httpd/HttpdConfigure.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdService;

    class HttpdSendLocalFile : public HttpdSender
    {
    private:
        bool m_bLive;
        sp<HttpdConfigure> m_configure;

    public:
        HttpdSendLocalFile();
        virtual ~HttpdSendLocalFile();

        virtual void run();
        virtual void stop();
        sp<Retval> sendPackets();

    private:
        sp<Retval> sendLocalFile(int k, sp<HttpdClient>& client);

    public:
        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDSENDLOCALFILE_H */

