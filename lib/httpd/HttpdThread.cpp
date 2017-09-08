#include <dframework/httpd/HttpdThread.h>
#include <dframework/log/Logger.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(HttpdThread);

    HttpdThread::HttpdThread(){
        DFW_SAFE_ADD(HttpdThread, l);
    }

    HttpdThread::~HttpdThread(){
        DFW_SAFE_REMOVE(HttpdThread, l);
    }

    void HttpdThread::stop(){
        Thread::stop();
        m_client->stop();
    }

    void HttpdThread::run(){
        sp<Retval> retval;

        if( DFW_RET(retval, run_2()) ){
            DFWLOG_CR(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID)
                        , m_client.get()
                        , retval
                        , "exit thread.");
        }else{
            DFWLOG_C(DFWLOG_D|DFWLOG_ID(DFWLOG_HTTPD_ID)
                        , m_client.get()
                        , "exit thread.");
        }
        m_client->close();
        m_configure = NULL;
        m_client = NULL;
    }

    sp<Retval> HttpdThread::run_2(){
        sp<Retval> retval;
        do{
            if( isstop() )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "stop httpd thread.");

            if( DFW_RET(retval, request()) ){
                return DFW_RETVAL_D(retval);
            }

            if( m_client->isKeepAlive() ){
                m_client->ready();
                continue;
            }

#if !defined(__APPLE__) && !defined(_WIN32)
            // left send packet.
#else
            if(!isstop()){
                sleep(2);
            }
#endif
            return DFW_RETVAL_NEW_MSG(DFW_OK, 0, "exit thread(no keepalive)");
        }while(true);
    }

    sp<Retval> HttpdThread::ready(sp<HttpdConfigure>& configure
                                , sp<ClientSocket>& sock)
    {
        AutoLock _l(this);
        sp<Retval> retval;
        m_configure = configure;
        sp<Object> configure_ = configure;
        m_client = new HttpdClient();
        m_client->setSocket(sock);
        m_client->setConfigure(configure_);
        return NULL;
    }

    sp<Retval> HttpdThread::request(){
        sp<Retval> retval;

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::request, handle=%d", m_client->getHandle());
        if( DFW_RET(retval, m_client->parseRequest()) ){
            //DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::complete-request 1");
            return DFW_RETVAL_D(retval);
        }

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::ready-reqeust, handle=%d", m_client->getHandle());
        if( DFW_RET(retval, m_client->readyRequest()) ){
            //DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::complete-request 2");
            return DFW_RETVAL_D(retval);
        }

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::send-response, handle=%d", m_client->getHandle());
        if( DFW_RET(retval, m_client->sendResponse()) ){
            //DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::complete-request 3");
            return DFW_RETVAL_D(retval);
        }

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "HttpdThread::complete-request, handle=%d", m_client->getHandle());
        return NULL;
    }

#if 0
    sp<Retval> HttpdThread::sendStream(){
        sp<Retval> retval;

        dfw_time_t s_time = Time::currentTimeMillis();
        int iscomplete = 0;
        do{
            iscomplete = 0;
            dfw_time_t c_time = Time::currentTimeMillis();
            if( (c_time-s_time) > (1000*60*10) ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT,0
                           , "handle=%d, Timeout send stream."
                           , m_client->getHandle());
            }

            if( DFW_RET(retval, m_client->sendStream(&iscomplete)) ){
                if( retval->value() != DFW_E_AGAIN )
                    return DFW_RETVAL_D(retval);
                continue;
            }

            if(iscomplete){
                DFWLOG_C(DFWLOG_L|DFWLOG_ID(DFWLOG_HTTPD_ID), m_client.get(), "l:");
                return NULL;
            }
        }while(true);
    }
#endif

};

