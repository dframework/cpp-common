#include <dframework/net/Socket.h>
#include <dframework/net/Net.h>
#include <dframework/net/Hostname.h>
#include <dframework/net/Poll.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <netinet/tcp.h>
# ifdef __APPLE__
#  include <stdlib.h>
#  include <string.h>
# else
#  include <linux/sockios.h>
# endif
#else
# include <winsock2.h>
# include <ws2tcpip.h>
#endif
#include <dframework/log/Logger.h>

namespace dframework {

    Socket::Socket()
    {
        DFW_SOCK_INIT(m_iHandle);
        m_iAddrType = AF_INET;
        m_ConnTimeMSec = 10000;
        m_TimeoutMSec = 10000;
        m_iSockType = SOCK_STREAM;
        m_RecvSoSize = 102400;
        m_SendSoSize = 102400;
        m_iPort = 0;
        m_useTime = 0;
    }
    
    Socket::~Socket(){
        close();
    }
   
    sp<Retval> Socket::setFcntl(int v){
#ifndef _WIN32
        sp<Retval> retval;
        int flags = 0;
        if( DFW_RET(retval, fcntl_k(&flags, F_GETFL, 0)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, fcntl_k(NULL, F_SETFL, flags|v));
#else
        DFW_UNUSED(v);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support fcntl for windows.");
#endif
    }

    sp<Retval> Socket::removeFcntl(int v){
#ifndef _WIN32
        sp<Retval> retval;
        int flags = 0;
        if( DFW_RET(retval, fcntl_k(&flags, F_GETFL, 0)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, fcntl_k(NULL, F_SETFL, flags&(~v)));
#else
        DFW_UNUSED(v);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support fcntl for windows.");
#endif
    }

    sp<Retval> Socket::setLinger(int onoff, int linger){
        sp<Retval> retval;
        struct linger nLinger;
        nLinger.l_onoff = onoff;
        nLinger.l_linger = linger;
        return DFW_RET_C(retval
                   , setSockOpt(SOL_SOCKET, SO_LINGER, (void*)&nLinger, sizeof(nLinger)));
    }

    sp<Retval> Socket::setKeepAlive(int alive){
        return setSockOpt(SOL_SOCKET, SO_KEEPALIVE, (void*)&alive, sizeof(int));
    }

    sp<Retval> Socket::setNonBlock(bool nonblock){
#ifdef _WIN32
        int stat;
        long unsigned imode = (int)nonblock;
        if( (stat=::ioctlsocket(m_iHandle, FIONBIO, &imode)) != 0 )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not ioctlsocket(FIOBIO), value=%d", stat);
        return NULL;
#else
        if(nonblock)
            return setFcntl(O_NONBLOCK);
        return removeFcntl(O_NONBLOCK);
#endif
    }

    sp<Retval> Socket::getSockOpt(int level, int name, void* val, socklen_t* len){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::getSockOpt(m_iHandle, level, name, val, len)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::setSockOpt(int level, int name, const void* val, socklen_t len)
    {
        sp<Retval> retval;
        if( DFW_RET(retval, Net::setSockOpt(m_iHandle, level, name, val, len)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::getSockOptError(int *eno){
        sp<Retval> retval;
        socklen_t err_len = sizeof(int);
        return DFW_RET_C(retval, Net::getSockOpt(m_iHandle, SOL_SOCKET, SO_ERROR, eno, &err_len));
    }

    sp<Retval> Socket::open(int addrtype, int socktype)
    {
        sp<Retval> retval;
       
        close(); 
        
        m_iHandle = ::socket(addrtype, socktype, 0);
        if(-1==m_iHandle){
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No setsockopt");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No setsockopt");
#endif
            if(rno==DFW_ERROR) rno = DFW_E_SOCKET;
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "handle=%d, addrtype=%d, socktype=%d %s"
                       , m_iHandle, addrtype, socktype, emsg);
        }
 
        m_iAddrType = addrtype;
        m_iSockType = socktype;

        if( DFW_RET(retval, setTcpNoDelay(true)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }
    
    sp<Retval> Socket::close(){
        DFW_SOCK_CLOSE(m_iHandle);
        return NULL;
    }

    sp<Retval> Socket::fcntl_k(int *out, int cmd, int arg)
    {
#ifdef _WIN32
        DFW_UNUSED(out);
        DFW_UNUSED(cmd);
        DFW_UNUSED(arg);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not support fcntl_k for windows.");
#else
        int stat;
        int retry = 0;
        do{
            if( (-1!=(stat=::fcntl(m_iHandle, cmd/*F_GETFL*/, arg)))){
                if(out) *out = stat;
                return NULL;
            }

            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
#else
            int eno = errno;
#endif
            switch(eno){
            case EAGAIN :
                if(retry>10){
                    return DFW_RETVAL_NEW(DFW_E_AGAIN,0);
                }
                usleep(1000*100);
                retry++;
                continue;
            }

#ifdef _WIN32
            const char* emsg = Retval::wsa2value(&rno, eno, "No fcntl");
#else
            const char* emsg = Retval::errno_short(&rno, eno, "No fcntl");
#endif
            if( rno == DFW_ERROR ) rno = DFW_E_FCNTL;
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "cmd=%d, arg=%d, %s", cmd, arg, emsg);
        }while(true);
#endif
    }
    
    
    sp<Retval> Socket::connect(const char *uri){
        URI urik = uri;
        return Socket::connect(urik);
    }
    
    sp<Retval> Socket::connect(URI& uri){
        sp<Retval> retval;
        Hostname host;
        if(DFW_RET(retval, host.get(uri)) )
            return DFW_RETVAL_D(retval);
        if( host.size()<=0 )
            return DFW_RETVAL_NEW(DFW_E_HOST_NOT_FOUND,0);

        int num = 0;
        int opentype = -1;
        int port = uri.getPort();
        sp<Hostname::Result> ips;

        do{
            if(DFW_RET(ips, host.getResult(num))){
                const char *ip = ips->m_sIp.toChars();
                int atype = ips->m_iAddrType;
                if(atype!=opentype){
                    if(DFW_RET(retval
                        , Socket::open(atype, SOCK_STREAM)) ){
                        return DFW_RETVAL_D(retval);
                    }
                    opentype = atype;
                }
                if(!DFW_RET(retval
                    , Socket::connectbyip(atype, ip, port, false)) ){
                    return NULL;
                }
            }

            if(ips.has()){
                num++;
                continue;
            }
            break;
        }while(true);

        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
        return DFW_RETVAL_NEW(DFW_E_CONNECT,0);
    }

    sp<Retval> Socket::setTcpNoDelay(bool val){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::setTcpNoDelay(m_iHandle, val)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::connectbyip(
            int addrType, const char *ip, int port, bool isopen)
    {
        sp<Retval> retval;
        m_iAddrType = addrType;
        m_sIp = ip;
        m_iPort = port;

        if(isopen){
            if(DFW_RET(retval,Socket::open(addrType, m_iSockType)) )
                return DFW_RETVAL_D(retval);
        }else{
            if(-1==m_iHandle)
                return DFW_RETVAL_NEW(DFW_E_CONNECT,0);
        }

        if( DFW_RET(retval, connect_real()) )
            return DFW_RETVAL_D(retval);

        // FIXME:
        //if(DFW_RET(retval, setFcntl(FD_CLOEXEC)))
        //    return DFW_RETVAL_D(retval);

#if !defined(_WIN32) && defined(__APPLE__)
        if(DFW_RET(retval, Net::setSigPipe(m_iHandle, 1)) )
            return DFW_RETVAL_D(retval);
#endif

#if !defined(__APPLE__) && !defined(_WIN32)
        if( DFW_RET(retval, setTcpNoDelay(true)) )
            return DFW_RETVAL_D(retval);
#endif

        if(DFW_RET(retval, setLinger(1,0)))
            return DFW_RETVAL_D(retval);

        if(DFW_RET(retval, setKeepAlive(1)))
            return DFW_RETVAL_D(retval);

        if(DFW_RET(retval, setRecvBufferSize(m_RecvSoSize)))
            return DFW_RETVAL_D(retval);
        
        if(DFW_RET(retval, setSendBufferSize(m_SendSoSize)))
            return DFW_RETVAL_D(retval);

#if 0
        struct timeval tv;
        tv.tv_sec = 120;
        tv.tv_usec = 0;
        if(DFW_RET(retval, setOpt(SO_RCVTIMEO, &tv, sizeof(tv), SOL_SOCKET)))
            return DFW_RETVAL_D(retval);
        if(DFW_RET(retval, setOpt(SO_SNDTIMEO, &tv, sizeof(tv), SOL_SOCKET)))
            return DFW_RETVAL_D(retval);
#endif

        return NULL;
    }

    sp<Retval> Socket::wait(int rdwr, int msec/*1/1000sec*/)
    {
        int ret;
        int eno;
        struct pollfd fds;
        fds.fd = m_iHandle;
        fds.events = ((rdwr==0)?POLLIN:POLLOUT)|POLLERR|POLLHUP|POLLNVAL;
        do{
            fds.revents = 0;
#ifdef _WIN32
            if( -1 == (ret = win32_poll(&fds, 1, msec)) )
#else
            if( -1 == (ret = ::poll(&fds, 1, msec)) )
#endif
            {
                eno = errno;
                switch(eno){
                case EINTR:
                    return DFW_RETVAL_NEW(DFW_E_INTR,eno);
                case EFAULT :
                    return DFW_RETVAL_NEW(DFW_E_FAULT,eno);
                case EINVAL:
                    return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
                case ENOMEM:
                    return DFW_RETVAL_NEW(DFW_E_NOMEM,eno);
                }
                return DFW_RETVAL_NEW(DFW_E_POLL,eno);
            }else if( 0 == ret ){
                return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT,0
                           , "handle=%d, rdwr=%d, timeout=%d, TIMEOUT"
                           , m_iHandle, rdwr, msec);
            }

            if( (fds.revents & POLLIN)==POLLIN && rdwr==0){
                return NULL;
            }
            if( (fds.revents & POLLOUT)==POLLOUT && rdwr==1){
                return NULL;
            }

            if( (fds.revents & POLLERR) == POLLERR ){
                return DFW_RETVAL_NEW_MSG(DFW_E_POLLERR,0
                           , "handle=%d, rdwr=%d, timeout=%d, POLLERR"
                           , m_iHandle, rdwr, msec);
            }else if( (fds.revents & POLLHUP) == POLLHUP ){
                return DFW_RETVAL_NEW_MSG(DFW_E_POLLHUP,0
                           , "handle=%d, rdwr=%d, timeout=%d, POLLHUP"
                           , m_iHandle, rdwr, msec);
            }else if( (fds.revents & POLLNVAL) == POLLNVAL ){
                return DFW_RETVAL_NEW_MSG(DFW_E_POLLNVAL,0
                           , "handle=%d, rdwr=%d, timeout=%d, POLLNVAL"
                           , m_iHandle, rdwr, msec);
            }else{
                return DFW_RETVAL_NEW_MSG(DFW_E_POLL,0
                           , "handle=%d, rdwr=%d, timeout=%d"
                           , m_iHandle, rdwr, msec);
            }
        }while(true);

        /*
        // FIXME:
        fd_set rdfds, wrfds;
        struct timeval timeout, *ptimeout = (msec>=0 ? &timeout:NULL);
        FD_ZERO(&rdfds);
        FD_ZERO(&wrfds);
        if( rdwr == 0 ){
            FD_SET(m_iHandle, &rdfds);
        }else{
            FD_SET(m_iHandle, &wrfds);
        }
        if( ptimeout ){
            ptimeout->tv_sec = msec/1000;
            ptimeout->tv_usec = (msec%1000)*100;
        }
        do{
            if(-1==(ret=select(m_iHandle+1, &rdfds, &wrfds, NULL, ptimeout)) ){
                eno = errno;
                switch(eno){
                    case EINTR:
                        continue;
                    case EBADF:
                        return DFW_RETVAL_NEW(DFW_E_BADF,eno);
                    case EINVAL:
                        return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
                    case ENOMEM:
                        return DFW_RETVAL_NEW(DFW_E_NOMEM,eno);
                }
                return DFW_RETVAL_NEW(DFW_E_SELECT,eno);
            }else if( 0 == ret ){
                return DFW_RETVAL_NEW(DFW_E_TIMEOUT,0);
            }
            break;
        }while(true);
        */

        return NULL;
    }

    sp<Retval> Socket::connect_raw(){
        sp<Retval> retval;
        struct sockaddr_in sa;

        ::memset( (void *)&sa, 0, sizeof(sa));
        sa.sin_family = m_iAddrType;
        sa.sin_addr.s_addr = inet_addr(m_sIp.toChars());
        sa.sin_port = htons(m_iPort);

        do{
            if( (-1 == ::connect(m_iHandle
                               , (struct sockaddr*)&sa
                               , sizeof(sa))) )
            {
#ifdef _WIN32
		int eno = WSAGetLastError();
#else
                int eno = errno;
#endif
                /*if(eno==EINTR){
                    continue;
                }*/

                if(Net::isInprogress(eno)){
                    if(DFW_RET(retval, wait(1, m_ConnTimeMSec)) ){
                        return DFW_RETVAL_D(retval);
                    }

                    //FIXME: unused: socklen_t len = sizeof(eno);
                    eno = 0;
                    if(DFW_RET(retval, getSockOptError(&eno)))
                        return DFW_RETVAL_D(retval);
                    if( eno ){
                        switch(eno){
                        case EBADF:
                            return DFW_RETVAL_NEW(DFW_E_BADF,eno);
                        case EFAULT:
                            return DFW_RETVAL_NEW(DFW_E_FAULT,eno);
                        case EINVAL:
                            return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
                        case ENOPROTOOPT:
                            return DFW_RETVAL_NEW(DFW_E_NOPROTOOPT,eno);
                        case ENOTSOCK:
                            return DFW_RETVAL_NEW(DFW_E_NOTSOCK,eno);
                       }
                       return DFW_RETVAL_NEW(DFW_E_CONNECT,eno);
                    }
                    return NULL;
                }

                dfw_retno_t rno;
#ifdef _WIN32
                const char* emsg = Retval::wsa2value(&rno, eno, "No fcntl");
#else
                const char* emsg = Retval::errno_short(&rno, eno, "No fcntl");
#endif
                if( rno == DFW_ERROR ) rno = DFW_E_CONNECT;
                return DFW_RETVAL_NEW_MSG(rno, eno
                       , "addr=%s, port=%d, type=%d, %s"
                       , m_sIp.toChars(), m_iPort, m_iAddrType, emsg);
            }
            return NULL;
        }while(true);
    }

    sp<Retval> Socket::connect_real(){
        sp<Retval> retval;

        int flags = 0;
#ifndef _WIN32
        bool restoreFlags = false;
        if(DFW_RET(retval, fcntl_k(&flags, F_GETFL, 0)))
            return DFW_RETVAL_D(retval);

        if(m_ConnTimeMSec>0){
            if( !(flags & O_NONBLOCK) ){
                if(DFW_RET(retval, setNonBlock(true)))
                    return DFW_RETVAL_D(retval);
                restoreFlags = true;
            }
        }else{
            if( flags & O_NONBLOCK ){
                if(DFW_RET(retval, setNonBlock(false)))
                    return DFW_RETVAL_D(retval);
                restoreFlags = true;
            }
        }

        if(DFW_RET(retval, connect_raw()))
            return DFW_RETVAL_D(retval);

        if(restoreFlags){
            if(DFW_RET(retval, fcntl_k(NULL, F_SETFL, flags)))
                return DFW_RETVAL_D(retval);
        }

        if(m_TimeoutMSec>0){
            if( !(flags & O_NONBLOCK) ){
                if(DFW_RET(retval, setNonBlock(true)))
                    return DFW_RETVAL_D(retval);
            }
        }
#else
        if(m_ConnTimeMSec>0){
            flags = 1;
            if(DFW_RET(retval, setNonBlock(true)))
                return DFW_RETVAL_D(retval);
        }else{
            flags = 0;
            if(DFW_RET(retval, setNonBlock(false)))
                return DFW_RETVAL_D(retval);
        }

        if(DFW_RET(retval, connect_raw()))
            return DFW_RETVAL_D(retval);

        if(m_TimeoutMSec>0){
            if( flags == 0 ){
                if(DFW_RET(retval, setNonBlock(true)))
                    return DFW_RETVAL_D(retval);
            }
        }else{
            if( flags == 1 ){
                if(DFW_RET(retval, setNonBlock(false)))
                    return DFW_RETVAL_D(retval);
            }
        }
#endif
        return NULL;
    }
    
    sp<Retval> Socket::send_raw(
                   size_t* o_size, const char *data
                 , size_t size
                 , bool iswait)
    {
        sp<Retval> retval;
        size_t offset = 0;
        size_t l_size = size;

        if(o_size){ *o_size = 0; }
        do{
            if(0==l_size) {
                return NULL;
            }
#if defined(MSG_NOSIGNAL)
            int s_size = ::send(m_iHandle, data+offset, l_size, MSG_NOSIGNAL);
#else
            int s_size = ::send(m_iHandle, data+offset, l_size, 0);
#endif
            if(-1==s_size){
#if defined(_WIN32)
                int eno = WSAGetLastError();
#else
                int eno = errno;
#endif

                if( Net::isInprogress(eno) ){
                    if(!iswait)
                        return DFW_RETVAL_NEW(DFW_E_AGAIN,eno);
                    if(!DFW_RET(retval, wait_send())){
                        continue;
                    }
                    return DFW_RETVAL_D(retval); // DFW_E_ETIMEDOUT
                }

                dfw_retno_t rno;
#ifdef _WIN32
                const char* emsg = Retval::wsa2value(&rno, eno, "No write");
#else
                const char* emsg = Retval::errno_short(&rno, eno, "No write");
#endif
                if( rno == DFW_ERROR ) rno = DFW_E_WRITE;
                return DFW_RETVAL_NEW_MSG(rno, eno
                       , "handle=%d, size=%ld, osize=%ld,"
                         " addr=%s, port=%d, type=%d, %s"
                       , m_iHandle, size, *o_size
                       , m_sIp.toChars(), m_iPort, m_iAddrType, emsg);
            }else if(s_size>0){
                l_size -= s_size;
                offset += s_size;
                if(o_size){ *o_size = offset; }
                continue;
            }

            return DFW_RETVAL_NEW_MSG(DFW_E_DISCONNECT, 0
                   , "handle=%d, size=%ld, osize=%ld,"
                     " addr=%s, port=%d, type=%d, Disconnect in write"
                   , m_iHandle, size, *o_size
                   , m_sIp.toChars(), m_iPort, m_iAddrType);
        }while(true);
    }

    sp<Retval> Socket::recv_raw(char* o_data, size_t *o_size, size_t size, bool iswait){
        sp<Retval> retval;
        size_t offset = 0;
        size_t l_size = size;

        if(o_size){ *o_size = 0; }

        do{
            if(0==l_size){
                return NULL;
            }

            size_t r_size = ::recv(m_iHandle, o_data+offset, l_size, 0);

            if(((size_t)-1)==r_size){
#if defined(_WIN32)
                int eno = WSAGetLastError();
#else
                int eno = errno;
#endif

                if( Net::isInprogress(eno) ){
                    if(!iswait) {
                        return DFW_RETVAL_NEW(DFW_E_AGAIN,eno);
                    }
                    if(!DFW_RET(retval, wait_recv())) {
                        continue;
                    }
                    return DFW_RETVAL_D(retval); // with DFW_E_ETIMEDOUT
                }

                dfw_retno_t rno;
#ifdef _WIN32
                const char* emsg = Retval::wsa2value(&rno, eno, "No read");
#else
                const char* emsg = Retval::errno_short(&rno, eno, "No read");
#endif
                if( rno == DFW_ERROR ) rno = DFW_E_RECV;
                return DFW_RETVAL_NEW_MSG(rno, eno
                       , "handle=%d, size=%ld, osize=%ld,"
                         " addr=%s, port=%d, type=%d, %s"
                       , m_iHandle, size, *o_size
                       , m_sIp.toChars(), m_iPort, m_iAddrType, emsg);
            }else if(r_size>0){
                l_size -= r_size;
                offset += r_size;
                if(o_size){ *o_size = offset; }
                continue;
            }

            return DFW_RETVAL_NEW_MSG(DFW_E_DISCONNECT, 0
                   , "handle=%d, size=%ld, osize=%ld,"
                     " addr=%s, port=%d, type=%d, Disconnect in read"
                   , m_iHandle, size, *o_size
                   , m_sIp.toChars(), m_iPort, m_iAddrType);
        }while(true);
    }

    sp<Retval> Socket::setRecvBufferSize(int size){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::setSockOptInt(m_iHandle, SOL_SOCKET, SO_RCVBUF, size)) )
            return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, getRecvBufferSize(&m_RecvSoSize)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::setSendBufferSize(int size){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::setSockOptInt(m_iHandle, SOL_SOCKET, SO_SNDBUF, size)) )
            return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, getSendBufferSize(&m_SendSoSize)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::getRecvBufferSize(int* size){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::getSockOptInt(m_iHandle, SOL_SOCKET, SO_RCVBUF, size)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::getSendBufferSize(int* size){
        sp<Retval> retval;
        if( DFW_RET(retval, Net::getSockOptInt(m_iHandle, SOL_SOCKET, SO_SNDBUF, size)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Socket::getSendBufferLeftSize(int *size){
#ifndef _WIN32
        AutoLock _l(this);
        sp<Retval> retval;
#ifdef __APPLE__
        //FIONWRITE
        if( -1 == ::ioctl(m_iHandle, TIOCOUTQ, size))
#elif defined(_WIN32)
        if( -1 == ::ioctlsocket(m_iHandle, SIOCOUTQ, size))
#else
        if( -1 == ::ioctl(m_iHandle, SIOCOUTQ, size))
#endif
        {
            int eno = errno;
            switch(eno){
            case EBADF:
                return DFW_RETVAL_NEW_MSG(DFW_E_BADF, eno
                          , "%d is not avalid descriptor."
                          , m_iHandle);
            case EFAULT: 
                return DFW_RETVAL_NEW_MSG(DFW_E_FAULT, eno
                        , "argp references an inaccessible memory area.");
            case EINVAL: 
                return DFW_RETVAL_NEW_MSG(DFW_E_FAULT, eno
                          , "request or argp is not valid");
            case ENOTTY: 
                return DFW_RETVAL_NEW_MSG(DFW_E_FAULT, eno, "ENOTTY");
            case ENXIO: 
                return DFW_RETVAL_NEW_MSG(DFW_E_FAULT, eno, "No such device or address");
            }
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, eno, "Not ioctl.");
        }
        return NULL;
#else /* ifndef _WIN32 */
        DFW_UNUSED(size);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not support SIOCOUTQ.");
#endif /* ifndef _WIN32 */
    }

};

