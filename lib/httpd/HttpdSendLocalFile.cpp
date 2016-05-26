#include <dframework/httpd/HttpdSendLocalFile.h>
#include <dframework/httpd/HttpdWorker.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpdSendLocalFile::HttpdSendLocalFile(){
        m_bLive = true;
        m_poll->setEvents(POLLERR|POLLNVAL|POLLHUP|POLLOUT);
    }

    HttpdSendLocalFile::~HttpdSendLocalFile(){
    }

    void HttpdSendLocalFile::run(){
        sp<Retval> retval;
        while(true){
            {
                AutoLock _l(this);
                if( !m_bLive ) {
                    DFWLOG(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), "send local file is stoped");
                    return;
                }
            }
            if( DFW_RET(retval, sendPackets()) ){
                DFWLOG_R(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), retval, "");
            }
            queueClient();
        }//end while(true)
    }

    void HttpdSendLocalFile::stop(){
        AutoLock _l(this);
        m_bLive = false;
    }

    sp<Retval> HttpdSendLocalFile::sendPackets(){
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
        sp<Object> obj;
        sp<HttpdClient> client;
        for(int k=size-1; k>-1; k--){
            {
                AutoLock _l(this);
                if( !m_bLive ) {
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR,0,"send loal file is stoped.");
                }
            }

            iserror = 0;
            obj = NULL;
            client = NULL;

            if( DFW_RET(retval, m_poll->getObject(obj, k)) ){
                m_poll->remove(k);
                DFWLOG_R(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID), retval
                    , "Not found worker object.");
                continue;
            }

            client = (HttpdClient*)obj.get();
            if((!(iserror=m_poll->getPollErr(k))) && m_poll->isPollOut(k)){
                if( DFW_RET(retval, sendLocalFile(k, client)) ){
                    if( retval->value() != DFW_E_AGAIN ){
                        iserror = -1;
                    }
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

        } // end for(int k=size-1; k>-1; k--)
        return NULL;
    }

    sp<Retval> HttpdSendLocalFile::sendLocalFile(int pos
                                               , sp<HttpdClient>& client)
    {
        sp<Retval> retval;

        if( DFW_RET(retval, client->sendLocalFile()) )
            return DFW_RETVAL_D(retval);

        DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), client.get(), "l:");

        m_poll->remove(pos);

        if( client->isKeepAlive() ){
            sp<HttpdWorker> worker = m_worker;
            if(worker.has()){
                if( DFW_RET(retval, worker->appendClient(client)) ){
                }
            }else{
            }
        }

        return NULL;
    }

};

