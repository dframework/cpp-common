#ifndef DFRAMEWORK_HTTPD_HTTPDSENDER_H
#define DFRAMEWORK_HTTPD_HTTPDSENDER_H

#include <dframework/base/Thread.h>
#include <dframework/util/Queue.h>
#include <dframework/httpd/HttpdClient.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdWorker;
    class HttpdService;

    class HttpdSender : public Thread
    {
    public:
        static const char* HTML_ERRFMT;
        static const char* HTML_ERRFMT_LOCATION;

        static const char* STATUS_TITLE_301;
        static const char* STATUS_TITLE_304;
        static const char* STATUS_TITLE_403;
        static const char* STATUS_TITLE_404;

        static sp<Retval> makeError(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title, bool bKeepAlive=false);

        static sp<Retval> makeNoBodyError(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title, bool bKeepAlive=false);

        static sp<Retval> makeLocation(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title
                              , const char* location);

        static const char* getStatusMsg(dfw_httpstatus_t status);

    protected:
        sp<Poll> m_poll;
        Queue<HttpdClient> m_queue;
        sp<Object> m_worker;

    public:
        HttpdSender();
        virtual ~HttpdSender();

        virtual sp<Retval> appendClient(sp<HttpdClient>& client);

    protected:
        virtual void queueClient();

    public:
        friend class HttpdWorker;
        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDSENDER_H */

