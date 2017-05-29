#include <dframework/base/ThreadManager.h>
#include <dframework/httpd/HttpdAcceptor.h>
#include <dframework/net/Net.h>
#include <dframework/httpd/HttpdService.h>
#include <dframework/httpd/HttpdThread.h>
#include <dframework/log/Logger.h>

#include <fcntl.h>
#ifdef __APPLE__
#include <netinet/tcp.h>
#endif

namespace dframework {

    HttpdAcceptor::HttpdAcceptor(){
    }

    HttpdAcceptor::~HttpdAcceptor(){
    }

    void HttpdAcceptor::stop(){
        sp<Retval> retval;

        ServerAccept::stop();

        int type = m_configure->getServerType();
        switch(type){
        case HttpdService::SERVER_TYPE_POLL :
            break;

        case HttpdService::SERVER_TYPE_THREAD :
            {
                sp<ThreadManager> tm = ThreadManager::instance();

                DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "stop httpd client threads (count: %d)", tm->size());
                for(int k=(tm->size()-1); k>=0; k--){
                    sp<Thread> thread = tm->get(k);
                    if(thread.has()){
                        if(thread.get()!=this){
                            thread->stop();
                            thread->join();
                        }
                    }
                }
                DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID), "stop httpd client threads complete (count: %d)", tm->size());
            }
            break;
        }
    }

    sp<Retval> HttpdAcceptor::onAccept(sp<ClientSocket>& sock){
        sp<Retval> retval;

        int handle = sock->getHandle();

        if( DFW_RET(retval, Net::setLinger(handle, 1, 0)) ){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, Net::setKeepAlive(handle, 1)) ){
            return DFW_RETVAL_D(retval);
        }

#define  RECV_SIZE 51200
#define  SEND_SIZE 102400

        if(DFW_RET(retval, sock->setRecvBufferSize(RECV_SIZE))){
            return DFW_RETVAL_D(retval);
        }
                
        if(DFW_RET(retval, sock->setSendBufferSize(SEND_SIZE))){
            return DFW_RETVAL_D(retval);
        }

        if( DFW_RET(retval, Net::setNonBlock(handle, true)) ){
            return DFW_RETVAL_D(retval);
        }

#if defined(__APPLE__)
        if(DFW_RET(retval, Net::setSigPipe(handle, 1)) ){
            return DFW_RETVAL_D(retval);
        }
#endif

        if( DFW_RET(retval, sock->setTcpNoDelay(true)) ){
            return DFW_RETVAL_D(retval);
        }

#if 0
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if(DFW_RET(retval, sock->setSockOpt(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))))
            return DFW_RETVAL_D(retval);
        if(DFW_RET(retval, sock->setSockOpt(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))))
            return DFW_RETVAL_D(retval);
#endif

        int type = m_configure->getServerType();
        switch(type){
        case HttpdService::SERVER_TYPE_POLL :
            if( DFW_RET(retval, m_worker->appendSocket(sock)) )
                return DFW_RETVAL_D(retval);
            break;

        case HttpdService::SERVER_TYPE_THREAD :
            {
                sp<HttpdThread> thd = new HttpdThread();
                if( DFW_RET(retval, thd->ready(m_configure, sock)) )
                    return DFW_RETVAL_D(retval);
                if( DFW_RET(retval, thd->start()) )
                    return DFW_RETVAL_D(retval);
            }
            break;

        default :
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "server type is wrong. type=%d", type);
        }

        return NULL;
    }

};

