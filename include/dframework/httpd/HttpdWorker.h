#ifndef DFRAMEWORK_HTTPD_HTTPDWORKER_H
#define DFRAMEWORK_HTTPD_HTTPDWORKER_H

#include <dframework/base/Thread.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/util/Queue.h>
#include <dframework/net/Poll.h>
#include <dframework/httpd/HttpdClient.h>
#include <dframework/httpd/HttpdSendStream.h>
#include <dframework/httpd/HttpdSendLocalFile.h>
#include <dframework/httpd/HttpdConfigure.h>


#ifdef __cplusplus
namespace dframework {

    class HttpdService;

    class HttpdWorker : public Thread
    {
    private:
        bool m_bLive;

        sp<HttpdConfigure> m_configure;
        sp<HttpdSendStream> m_stream;
        sp<HttpdSendLocalFile> m_localfile;
        sp<Poll> m_poll;
        Queue<HttpdClient> m_queue;

    public:
        HttpdWorker();
        virtual ~HttpdWorker();

        virtual void run();
        virtual void stop();

        sp<Retval> appendSocket(sp<ClientSocket>& sock);
        sp<Retval> readPackets();

        dfw_httpstatus_t sendLocalFile(sp<HttpdClient>& client);

        sp<Retval> pollin(int pos, sp<HttpdClient>& client);

        sp<Retval> appendClient(sp<HttpdClient>& client);

        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDWORKER_H */

