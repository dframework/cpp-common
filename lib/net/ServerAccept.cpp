#include <dframework/net/ServerAccept.h>
#include <dframework/net/Net.h>
#include <dframework/log/Logger.h>

namespace dframework {

    ServerSocket::ServerSocket()
    {
        m_iPort = 0;
        m_iStartPort = 0;
        m_iEndPort = 0;
        m_iPollNum = -1;
        m_bReuseAddr = true;
        DFW_SOCK_INIT(m_iHandle);
    }

    ServerSocket::~ServerSocket(){
        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "####");
        close();
    }

    void ServerSocket::setReuseAddr(bool bReuseAddr){
        m_bReuseAddr = bReuseAddr;
    }

    sp<Retval> ServerSocket::ready(int port){
        AutoLock _l(this);
        sp<Retval> retval;
        return DFW_RET_C(retval, ready(port, port));
    }

    sp<Retval> ServerSocket::ready(int sport, int eport){
        AutoLock _l(this);
        sp<Retval> retval;

        m_iStartPort = sport;
        m_iEndPort = eport;

        if( DFW_RET(retval, create(sport)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void ServerSocket::close(){
        AutoLock _l(this);
        if(m_iHandle!=-1){
            DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "serversocket::close : %d, %d, pid=%d", m_iHandle, m_iPort, getpid());
        }
        DFW_SOCK_CLOSE(m_iHandle);
    }

    sp<Retval> ServerSocket::create(int iport){
        AutoLock _l(this);
        sp<Retval> retval;

        m_iPort = 0;

        int mport = iport;
        if( mport >= m_iEndPort )
            mport = m_iStartPort;
        if( mport < m_iStartPort )
            mport = m_iStartPort;

        int port;
        for(port=mport; port<=m_iEndPort; port++){
            DFW_SOCK_CLOSE(m_iHandle);
            if(DFW_RET(retval, Net::socket(&m_iHandle, AF_INET, SOCK_STREAM))){
                continue;
            }
            if(DFW_RET(retval, Net::setLinger(m_iHandle, 1, 0)))
                continue;
            if(DFW_RET(retval, Net::setKeepAlive(m_iHandle, 1)))
                continue;
            
            if( m_bReuseAddr ){
                if(DFW_RET(retval, Net::setSockOptInt(m_iHandle, SOL_SOCKET, SO_REUSEADDR, 1)))
                    continue;
            }

#ifdef __FREEBSD__
            if(DFW_RET(retval, Net::setSockOptInt(m_iHandle, SOL_SOCKET, SO_REUSEPORT, 1)))
                continue;
#endif
            if(DFW_RET(retval, Net::setNonBlock(m_iHandle, true)))
                continue;
            if(DFW_RET(retval, Net::bind(m_iHandle, AF_INET, port)))
                continue;
            if(DFW_RET(retval, Net::listen(m_iHandle, 1024)))
                continue;
            m_iPort = port;
            DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "serversocket::create : %d, %d, pid=%d", m_iHandle, m_iPort, getpid());
            return NULL;
        }

        if( retval.has() ){
            DFW_SOCK_CLOSE(m_iHandle);
            return DFW_RETVAL_D(retval);
        }
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not create serversocket. port=%d", iport);
    }

    // --------------------------------------------------------------

    ClientSocket::ClientSocket(int sockfd, String& sIp, int port) 
            : Socket()
    {
        m_iHandle = sockfd;
        m_sIp = sIp;
        m_iServerPort = port;
    }

    ClientSocket::~ClientSocket(){
        DFW_SOCK_CLOSE(m_iHandle);
    }

    // --------------------------------------------------------------

    ServerAccept::ServerAccept(){
        m_bStarted = false;
        m_bReuseAddr = true;
        m_poll.setEvents(POLLIN|POLLERR|POLLNVAL|POLLHUP);
    }

    ServerAccept::~ServerAccept(){
    }

    void ServerAccept::setReuseAddr(bool bReuseAddr){
        m_bReuseAddr = bReuseAddr;
    }

    sp<Retval> ServerAccept::appendPort(int port){
        AutoLock _l(this);
        sp<Retval> retval;

        for(int k=0; k<m_aSockList.size(); k++){
            sp<ServerSocket> test = m_aSockList.get(k);
            if( test->m_iPort == port )
                return NULL;
        }

        sp<ServerSocket> sock = new ServerSocket();
        sock->setReuseAddr(m_bReuseAddr);
        if( DFW_RET(retval, sock->ready(port)) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, m_aSockList.insert(sock)) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, m_poll.append(&sock->m_iPollNum
                                        , sock->m_iHandle, sock.get()))){
            m_aSockList.remove(sock);
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> ServerAccept::appendServerSocket(sp<ServerSocket>& sock){
        AutoLock _l(this);
        sp<Retval> retval;

        sp<ServerSocket> test = m_aSockList.get(sock);
        if(test.has())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "already append socket. port=%d", sock->getPort());

        if( DFW_RET(retval, m_aSockList.insert(sock)) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, m_poll.append(&sock->m_iPollNum
                                        , sock->m_iHandle, sock.get()))){
            m_aSockList.remove(sock);
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> ServerAccept::start(){
        sp<Retval> retval;
        {
            AutoLock _l(this);
            if( m_bStarted ){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "This is started.");
            }
        }
        return DFW_RET_C(retval, Thread::start());
    }

    void ServerAccept::run(){
        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "ServerAccept Start");
        run_l();
        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "ServerAccept Stopped");
    }

    void ServerAccept::stop(){
        sp<Retval> retval;
        int size;
        {
            AutoLock _l(&m_poll);
            size = m_poll.size();
        }

        for(int k=size-1; k>-1; k--){
            sp<Object> obj;
            if( DFW_RET(retval, m_poll.getObject(obj, k)) ){
                continue;
            }
            sp<ServerSocket> ssock = obj;
            ssock->close();
        }

        Thread::stop();
    }

    void ServerAccept::run_l(){
        sp<Retval> retval;

        while(true){
            {
                AutoLock _l(this);
                if( isstop() ) {
                    DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_SERVER_ID), "serveraccept is stop.");
                    return;
                }
            }
            if( DFW_RET(retval, accept()) ){
                if(retval->error() || retval->value()){
                    DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID), retval, "failed accept !!!");
                }
                continue;
            }
        }
    }

    sp<Retval> ServerAccept::accept(){
        sp<Retval> retval;
        if( DFW_RET(retval, accept_poll()) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> ServerAccept::accept_poll(){
        sp<Retval> retval;
        int poll_count = 0;

        {
        AutoLock _l(&m_poll);
        int size = m_poll.size();
        if( size == 0 ){
            ::usleep(200);
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not server socket.");
        }

        if( DFW_RET(retval, m_poll.poll(&poll_count, 1000)) ){
            return DFW_RETVAL_D(retval);
        }
        if( poll_count == 0 ) {
            return DFW_RETVAL_NEW_MSG(DFW_OK, 0, "Poll count zero.");
        }

        int iserror = 0;
        for(int k=size-1; k>-1; k--){
            {
                AutoLock _l(this);
                if( isstop() ) {
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "serveraccept is stop.");
                }
            }

            iserror = 0;
            sp<Object> obj;
            if( DFW_RET(retval, m_poll.getObject(obj, k)) ){
                DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID), retval
                       , "Not found poll object.");
                continue;
            }
            sp<ServerSocket> ssock = obj;

            if( (!(iserror = m_poll.getPollErr(k))) && m_poll.isPollIn(k) ){
                int osock;
                DFW_SOCK_INIT(osock);
                String sIp;
                int fd = m_poll.getFd(k);
                if( !DFW_RET(retval, Net::accept(&osock, sIp, fd)) ){
                    sp<ClientSocket> scsock = new ClientSocket(osock, sIp
                                                     , ssock->getPort());
                    if( DFW_RET(retval, onAccept(scsock)) ){
                        DFWLOG_CR(DFWLOG_D|DFWLOG_ID(DFWLOG_SERVER_ID)
                           , scsock.get(), retval
                           , "Not onAccept. serv-port=%d, ip=%s, sock=%d"
                           , ssock->getPort(), sIp.toChars(), osock);
                    }else{
                        DFWLOG_C(DFWLOG_D|DFWLOG_ID(DFWLOG_SERVER_ID)
                           , scsock.get()
                           , "OnAccept. serv-port=%d, ip=%s, sock=%d"
                           , ssock->getPort(), sIp.toChars(), osock);
                    }
                }else if(retval->value()!=DFW_E_AGAIN){
                    DFWLOG_R(DFWLOG_E|DFWLOG_ID(DFWLOG_SERVER_ID), retval
                           , "Not accept. serv-port=%d", ssock->getPort());
                }
            }

            if(iserror){
                DFWLOG(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID)
                       , "poll-error. serv-port=%d, error=%d"
                       , ssock->getPort(), iserror);
                m_poll.remove(k);
                m_aSockList.remove(ssock);
                if( DFW_RET(retval, repairServer(ssock)) ){
                    DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID), retval
                       , "repair-server-error. serv-port=%d, error=%d"
                       , ssock->getPort(), iserror);
                    continue;
                }
                if( DFW_RET(retval, m_aSockList.insert(ssock)) ){
                    DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID), retval
                       , "insert-repair-server-error. serv-port=%d, error=%d"
                       , ssock->getPort(), iserror);
                    continue;
                }

                if( DFW_RET(retval, m_poll.append(&ssock->m_iPollNum
                                        , ssock->m_iHandle, ssock.get()))){
                    m_aSockList.remove(ssock);
                    DFWLOG_R(DFWLOG_F|DFWLOG_ID(DFWLOG_SERVER_ID), retval
                       , "insert-poll-repair-server-error. "
                         "serv-port=%d, error=%d"
                       , ssock->getPort(), iserror);
                    continue;
                }
            }

            m_poll.resetRevents(k);
        }
        } // end AutoLock

        return NULL;
    }

    sp<Retval> ServerAccept::onAccept(sp<ClientSocket>& sock){
        DFW_UNUSED(sock);
        return NULL;
    }

    sp<Retval> ServerAccept::repairServer(sp<ServerSocket>& sock){
        sp<Retval> retval;
        if( DFW_RET(retval, sock->create(sock->getPort())) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

};

