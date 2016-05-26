#include <dframework/httpd/HttpdWorker.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpdWorker::HttpdWorker(){
        m_bLive = true;
        m_poll = new Poll();
        m_poll->setEvents(POLLERR|POLLNVAL|POLLHUP|POLLIN|POLLPRI);
    }

    HttpdWorker::~HttpdWorker(){
    }

    sp<Retval> HttpdWorker::appendSocket(sp<ClientSocket>& sock){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<Object> configure = m_configure;
        sp<HttpdClient> client = new HttpdClient();
        client->setSocket(sock);
        client->setConfigure(configure);
        if( DFW_RET(retval, m_queue.push(client)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void HttpdWorker::run(){
        sp<Retval> retval;
        while(true){
            {
                AutoLock _l(this);
                if( !m_bLive ) {
                    DFWLOG(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), "worker is stop");
                    return;
                }
            }
            if( DFW_RET(retval, readPackets()) ){
                DFWLOG_R(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), retval, "");
            }

            while(true){
                {
                    AutoLock _l2(this);
                    if( !m_bLive ) {
                        DFWLOG(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), "worker is stop");
                        return;
                    }
                }
                sp<HttpdClient> client = m_queue.pop();
                if(!client.has())
                    break;
                int pollnum = -1;
                sp<Object> obj = client;
                if( DFW_RET(retval, m_poll->append(&pollnum
                              , client->getHandle(), obj)) ){
                    DFWLOG_CR(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID)
                       , client->m_sock.get(), retval
                       , "insert-worker-poll-error. "
                         "serv-port=%d", client->getServerPort());
                }
            } //end while(true)
        }// end while(true)
    }

    void HttpdWorker::stop(){
        AutoLock _l(this);
        m_bLive = false;
    }

    sp<Retval> HttpdWorker::readPackets(){
        sp<Retval> retval;
        int poll_count = 0;
        int size = 0;

        {
            AutoLock _l(m_poll.get());
            if( (size = m_poll->size()) == 0 ){
                ::usleep(200);
                return NULL;
            }
            if( DFW_RET(retval, m_poll->poll(&poll_count, 200)) )
                return DFW_RETVAL_D(retval);
            if( poll_count == 0 ) return NULL;
        }

        int iserror = 0;
        for(int k=size-1; k>-1; k--){
            {
                AutoLock _l(this);
                if( !m_bLive ) return NULL;
            }
            iserror = 0;
            sp<Object> obj;
            if( DFW_RET(retval, m_poll->getObject(obj, k)) ){
                m_poll->remove(k);
                DFWLOG_R(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID), retval
                    , "Not found worker object.");
                continue;
            }

            sp<HttpdClient> client = obj;
            if((!(iserror = m_poll->getPollErr(k)))&&m_poll->isPollIn(k)){
                if( DFW_RET(retval, pollin(k, client)) ){
                    DFW_RETVAL_D(retval);
                    iserror = retval->value();
                }
            }

            if(iserror){
                if( retval.has() ){
                    DFWLOG_CR(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID)
                        , client.get(), retval
                        , "remove-client. serv-port=%d, ip=%s, k=%d, error=%d"
                        , client->getServerPort()
                        , client->getIp()
                        , k, iserror);
                }else{
                    DFWLOG_C(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID)
                        , client.get()
                        , "remove-client. serv-port=%d, ip=%s, k=%d, error=%d"
                        , client->getServerPort()
                        , client->getIp()
                        , k, iserror);
                }
                m_poll->remove(k);
            }else{
                m_poll->resetRevents(k);
            }
        }//end for
        return NULL;
    }

    sp<Retval> HttpdWorker::pollin(int pos, sp<HttpdClient>& client){
        sp<Retval> retval;
        dfw_httpstatus_t outstatus = DFW_HTTP_STATUS_0;

#if 0
        if( DFW_RET(retval, client->readPackets()) )
            return DFW_RETVAL_D(retval);

        if( client->getCStatus()!=HTTPD_CSTATUS_BODY ){
            return NULL;
        }
#else
        if( DFW_RET(retval, client->parseRequest()) )
            return DFW_RETVAL_D(retval);
#endif

        sp<HttpdHost> host;
        {
            AutoLock _l(this);
            AutoLock _l2(client.get());
            sp<HttpRequest> req = client->getRequest();
            host = m_configure->getHost(req->m_sHost
                                  , client->m_sock->getServerPort());
            if( !host.has() )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not host.");
            sp<Object> hostobj = host;
            client->setHost(hostobj);
        }// end autolock

        DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), client.get(), "r:");

        // ----------------------------------------------------------
        // module
        for(int k=0; k<host->getModSize(); k++){
            sp<HttpdMod> mod = host->getMod(k);
            if( DFW_RET(retval, mod->request(client, &outstatus)))
                return DFW_RETVAL_D(retval);
        }
        // module
        // ----------------------------------------------------------

        if( !client->isLocalFile() )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not support cgi.");

        if( DFW_RET(retval, client->se_checkLocalFile(&outstatus)) ){
            sp<Retval> retval2;
            if( DFW_RET(retval2, client->makeStream()) )
                return DFW_RETVAL_D(retval2);
            if( DFW_RET(retval2, m_stream->appendClient(client)) )
                return DFW_RETVAL_D(retval2);

            return DFW_RETVAL_D(retval);
        }else{
            if(DFW_RET(retval, m_localfile->appendClient(client)))
                return DFW_RETVAL_D(retval);
        }

        m_poll->remove(pos);

        return NULL;
    }

    sp<Retval> HttpdWorker::appendClient(sp<HttpdClient>& client){
        AutoLock _l(this);
        sp<Retval> retval;
        client->ready();
        if( DFW_RET(retval, m_queue.push(client)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

};

