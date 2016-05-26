#include <dframework/httpd/HttpdSender.h>

namespace dframework {

    const char* HttpdSender::STATUS_TITLE_301 = "Moved Permanently";
    const char* HttpdSender::STATUS_TITLE_304 = "Not Modified";
    const char* HttpdSender::STATUS_TITLE_403 = "Forbidden";
    const char* HttpdSender::STATUS_TITLE_404 = "Not Found";

    const char* HttpdSender::HTML_ERRFMT =
        "<html>\n<head><title>%d %s</title></head>\n"
        "<body>\n"
        "<h1>%s</h1>\n"
        "<address><!-- --></address>\n"
        "</body>\n</html>\n";

    const char* HttpdSender::HTML_ERRFMT_LOCATION =
        "<html>\n<head><title>%d %s</title></head>\n"
        "<body>\n"
        "<h1>%s</h1>\n"
        "<p>The document has moved "
        "<a href=\"%s\">"
        "here</a>.</p>\n"
        "<address><!-- --></address>\n"
        "</body>\n</html>\n";

    const char* HttpdSender::getStatusMsg(dfw_httpstatus_t status){
        switch(status){
        case DFW_HTTP_STATUS_200:
            return "OK";
        case DFW_HTTP_STATUS_206:
            return "Partial Content";
        case DFW_HTTP_STATUS_301:
            return STATUS_TITLE_301;
        case DFW_HTTP_STATUS_304:
            return STATUS_TITLE_304;
        case DFW_HTTP_STATUS_403:
            return STATUS_TITLE_403;
        case DFW_HTTP_STATUS_404:
            return STATUS_TITLE_404;
        default :
            return "-";
        }
    }


    DFW_STATIC
    sp<Retval> HttpdSender::makeError(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title, bool bKeepAlive)
    {
        sp<Retval> retval;
        sp<HttpResponse> resp = client->getResponse();
        resp->m_sBody = String::format(HttpdSender::HTML_ERRFMT
                                    , status
                                    , title, title);
        if( resp->m_sBody.empty() )
            return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
        if( DFW_RET(retval, client->setResponse(
                                          status
                                        , resp->m_sBody.length()
                                        , bKeepAlive)) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> HttpdSender::makeNoBodyError(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title, bool bKeepAlive)
    {
        DFW_UNUSED(title);
        sp<Retval> retval;
        sp<HttpResponse> resp = client->getResponse();
        resp->m_sBody = NULL;
        if( DFW_RET(retval, client->setResponse(status, 0, bKeepAlive)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    DFW_STATIC
    sp<Retval> HttpdSender::makeLocation(sp<HttpdClient>& client
                              , dfw_httpstatus_t status
                              , const char* title
                              , const char* location)
    {
        sp<Retval> retval;
        sp<HttpResponse> resp = client->getResponse();
        resp->m_sBody = String::format(HttpdSender::HTML_ERRFMT_LOCATION
                                    , status
                                    , title, title
                                    , location);
        if( resp->m_sBody.empty() )
            return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
        if( DFW_RET(retval, client->setResponseLocation(
                                          status
                                        , resp->m_sBody.length()
                                        , location, true)) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    // --------------------------------------------------------------

    HttpdSender::HttpdSender(){
        m_poll = new Poll();
    }

    HttpdSender::~HttpdSender(){
    }

    sp<Retval> HttpdSender::appendClient(sp<HttpdClient>& client){
        AutoLock _l(this);
        sp<Retval> retval;
        if( DFW_RET(retval, m_queue.push(client)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void HttpdSender::queueClient(){
        AutoLock _l(this);
        sp<Retval> retval;
        while(true){
            sp<HttpdClient> client = m_queue.pop();
            if(!client.has()) break;

            int pollnum = -1;
            sp<Object> obj = client;
            if( DFW_RET(retval, m_poll->append(&pollnum
                          , client->getHandle(), obj)) ){
                // FIXME:
            }
        } // end while(true)
    }

};

