#include <dframework/httpd/HttpdSendStream.h>
#include <dframework/httpd/HttpdWorker.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpdSendStream::HttpdSendStream(){
        m_bLive = true;
        m_poll->setEvents(POLLERR|POLLNVAL|POLLHUP|POLLOUT);
    }

    HttpdSendStream::~HttpdSendStream(){
    }

    void HttpdSendStream::run(){
        sp<Retval> retval;
        while(true){
            {
                AutoLock _l(this);
                if( !m_bLive ) {
                    DFWLOG(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), "send stream is stop");
                    return;
                }
            }
            if( DFW_RET(retval, sendPackets()) ){
                DFWLOG_R(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID), retval, "");
            }
            queueClient();
        }//end while(true)
    }

    void HttpdSendStream::stop(){
        AutoLock _l(this);
        m_bLive = false;
    }

    sp<Retval> HttpdSendStream::sendPackets(){
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
                if( !m_bLive ) {
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR,0,"send stream is stop.");
                }
            }
            iserror = 0;
            sp<Object> obj;
            if( DFW_RET(retval, m_poll->getObject(obj, k)) ){
                m_poll->remove(k);
                DFWLOG_R(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID), retval
                    , "Not found worker object.");
                continue;
            }

            sp<HttpdClient> client = (HttpdClient*)obj.get();
            if( (!(iserror = m_poll->getPollErr(k))) && m_poll->isPollOut(k) ){
                int iscomplete = 0;
                if( DFW_RET(retval, client->sendStream(&iscomplete)) ){
                    if( retval->value() == DFW_E_AGAIN ){
                        retval = NULL;
                    }else{
                        iserror = -1;
                    }
                }else if(iscomplete){
                    DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), client.get(), "");
                    m_poll->remove(k);
                    if( client->isKeepAlive() ){
                        sp<HttpdWorker> worker = m_worker;
                        if(worker.has()){
                          if( DFW_RET(retval, worker->appendClient(client)) ){
                              DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_HTTPD_ID)
                                    , retval
                                    , "Not insert worker."
                                      " serv-port=%d, ip=%s, k=%d"
                                    , client->getServerPort()
                                    , client->getIp()
                                    , k);
                          }
                        }else{
                            DFWLOG(DFWLOG_F|DFWLOG_ID(DFWLOG_HTTPD_ID)
                                , "Not found worker."
                                  " serv-port=%d, ip=%s, k=%d"
                                , client->getServerPort()
                                , client->getIp()
                                , k);
                        }
                    }
                    continue;
                }
            }

            if(iserror){
                if( retval.has() ){
                    DFWLOG_R(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID), retval
                        , "remove-client. serv-port=%d, ip=%s"
                          ", k=%d, error=%d"
                        , client->getServerPort()
                        , client->getIp()
                        , k, iserror);
                }else{
                    DFWLOG(DFWLOG_E|DFWLOG_ID(DFWLOG_HTTPD_ID)
                        , "remove-client. serv-port=%d, ip=%s"
                          ", k=%d, error=%d"
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

};

