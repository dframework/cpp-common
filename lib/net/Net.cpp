#include <dframework/net/Net.h>
#include <dframework/log/Logger.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

extern "C" {
#ifdef _WIN32
    const char* inet_ntop(int af, const void* src, char* dst, int cnt);
#endif
}

#ifdef _WIN32
const char* inet_ntop2(int af, const void* src, char* dst, int cnt){
    struct sockaddr_in srcaddr;

    memset(&srcaddr, 0, sizeof(struct sockaddr_in));
    memcpy(&(srcaddr.sin_addr), src, sizeof(srcaddr.sin_addr));

    srcaddr.sin_family = af;
    if (WSAAddressToString((struct sockaddr*) &srcaddr, sizeof(struct sockaddr_in), 0, dst, (LPDWORD) &cnt) != 0) {
        DWORD rv = WSAGetLastError();
        DFW_UNUSED(rv);
        return NULL;
    }
    return dst;
}
#endif

namespace dframework {

    DFW_STATIC
    sp<Retval> Net::socket(int* out_sock, int family, int type){
        int handle;

        if( !out_sock )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "out_sock parameter is null.");

        if( (handle=::socket(family , type , 0))==-1){
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No socket");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No socket");
#endif
            if( rno == DFW_ERROR ) rno = DFW_E_SOCKET;
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "family=%d, type=%d, %s"
                       , family, type, emsg);
        }
        *out_sock = handle;
        return NULL;
    }

    DFW_STATIC
    sp<Retval> Net::bind(int sock, int family, int port){
        sp<Retval> retval;

        struct sockaddr_in sa;
        ::memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = family;
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        sa.sin_port = htons(port);

        if( ::bind(sock, (struct sockaddr*)&sa , sizeof(sa)) == -1 ){
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No bind");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No bind");
#endif
            if( rno == DFW_ERROR ) rno = DFW_E_BIND;
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "sock=%d, port=%d, family=%d, %s"
                       , sock, port, family, emsg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> Net::listen(int sock, int backlog){
        if( ::listen(sock, backlog) == -1 ){
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No listen");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No listen");
#endif
            if( rno == DFW_ERROR ) rno = DFW_E_LISTEN;
            return DFW_RETVAL_NEW_MSG(rno, eno, "sock=%d, backlog=%d, %s"
                       , sock, backlog, emsg);
        }
        return NULL;
    }

    sp<Retval> Net::accept(int* out_client_sock, String& sIp
                         , int server_sock){
        int fd;
        struct sockaddr_in saddr;
#if defined(_WIN32)
        int addrlen = sizeof(struct sockaddr_in);
#elif defined(__ANDROID__)
        socklen_t addrlen = sizeof(struct sockaddr_in);
#else
        unsigned addrlen = sizeof(struct sockaddr_in);
#endif
        if(!out_client_sock)
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "out_client_sock is null.");

        fd = ::accept(server_sock, (struct sockaddr*)&(saddr), &addrlen);
        if( fd == -1 ){
#if defined(_WIN32)
            int eno = WSAGetLastError();
#else
            int eno = errno;
#endif
            switch( eno )
            {
            case EAGAIN : // blocking retry.
#if defined(_WIN32)
            case WSAEWOULDBLOCK :
#endif
                return DFW_RETVAL_NEW(DFW_E_AGAIN, eno);
            }

            dfw_retno_t rno;
#ifdef _WIN32
            const char* emsg = Retval::wsa2value(&rno, eno, "No accept");
#else
            const char* emsg = Retval::errno_short(&rno, eno, "No accept");
#endif
            if( rno==DFW_ERROR ) rno = DFW_E_ACCEPT;
            return DFW_RETVAL_NEW_MSG(rno, eno
                     , "handle=%d, %s", server_sock, emsg);
        }

        *out_client_sock = fd;
        sIp = ::inet_ntoa(saddr.sin_addr);
        return NULL;
    }


    sp<Retval> Net::getSockOpt(int sock, int level, int name, void* value, socklen_t* len){
#ifdef _WIN32
        if( -1 == ::getsockopt(sock, level, name, (char*)value, len) )
#else
        if( -1 == ::getsockopt(sock, level, name, value, len) )
#endif
        {
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No getsockopt");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No getsockopt");
#endif
            if(rno==DFW_ERROR) rno = DFW_E_GETSOCKOPT;
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "handle=%d, name=%d, level=%d, %s"
                       , sock, name, level, emsg);
        }
        return NULL;
    }

    sp<Retval> Net::getSockOptInt(int sock, int level, int name, int* val){
        sp<Retval> retval;
        socklen_t len = sizeof(int);
        if( DFW_RET(retval, getSockOpt(sock, level, name, (void*)&val, &len)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Net::setSockOpt(int sock, int level, int name
                             , const void* val, socklen_t len)
    {
#ifdef _WIN32
        if( -1==::setsockopt(sock, level, name, (const char*)val, len) )
#else
        if( -1==::setsockopt(sock, level, name, val, len) )
#endif
        {
            dfw_retno_t rno;
#ifdef _WIN32
            int eno = WSAGetLastError();
            const char* emsg = Retval::wsa2value(&rno, eno, "No setsockopt");
#else
            int eno = errno;
            const char* emsg = Retval::errno_short(&rno, eno, "No setsockopt");
#endif
            if( rno==DFW_ERROR ) rno = DFW_E_SETSOCKOPT;
            return DFW_RETVAL_NEW_MSG(rno, eno
                     , "handle=%d, name=%d, %s"
                     , sock, name, emsg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> Net::setSockOptInt(int sock, int level, int name, int val)
    {
        sp<Retval> retval;
        if( DFW_RET(retval, setSockOpt(sock, level, name, (void*)&val, sizeof(int))) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    DFW_STATIC
    sp<Retval> Net::setLinger(int sock, int onoff, int linger){
        sp<Retval> retval;
        struct linger nLinger;
        nLinger.l_onoff = onoff;
        nLinger.l_linger = linger;
        return DFW_RET_C(retval, setSockOpt(sock, SOL_SOCKET, SO_LINGER
                                          , (void*)&nLinger, sizeof(nLinger)));
    }

    DFW_STATIC
    sp<Retval> Net::setKeepAlive(int sock, int alive){
        sp<Retval> retval;
        if( DFW_RET(retval, setSockOptInt(sock, SOL_SOCKET, SO_KEEPALIVE, alive)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Net::setFcntl(int sock, int v){
#ifdef _WIN32
        DFW_UNUSED(sock);
        DFW_UNUSED(v);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support fcntl for windows.");
#else
        sp<Retval> retval;
        int flags = 0;
        if( DFW_RET(retval, fcntl_k(&flags, sock, F_GETFL, 0)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, fcntl_k(NULL, sock, F_SETFL, flags|v));
#endif
    }

    sp<Retval> Net::removeFcntl(int sock, int v){
#ifdef _WIN32
        DFW_UNUSED(sock);
        DFW_UNUSED(v);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support fcntl for windows.");
#else
        sp<Retval> retval;
        int flags = 0;
        if( DFW_RET(retval, fcntl_k(&flags, sock, F_GETFL, 0)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, fcntl_k(NULL, sock, F_SETFL, flags&(~v)));
#endif
    }

    sp<Retval> Net::fcntl_k(int *out, int sock, int cmd, int arg)
    {
#ifdef _WIN32
        DFW_UNUSED(out);
        DFW_UNUSED(sock);
        DFW_UNUSED(cmd);
        DFW_UNUSED(arg);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not support fcntl_k for windows.");
#else
        int stat;
        int retry = 0;
        do{
            if( (-1!=(stat=::fcntl(sock, cmd/*F_GETFL*/, arg)))){
                if(out) *out = stat;
                return NULL;
            }

            int eno = errno;
            dfw_retno_t rno;
            const char* emsg = Retval::errno_short(&rno, eno, "No fcntl");
            if( rno==DFW_ERROR ) rno = DFW_E_FCNTL;

            switch(eno){
            case EAGAIN :
                if(retry>10){
                    return DFW_RETVAL_NEW(DFW_E_AGAIN,0);
                }
                usleep(1000*100);
                retry++;
                continue;
            /*case EINTR:
                continue;*/
            }
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "handle=%d, cmd=%d, arg=%d, %s"
                       , sock, cmd, arg, emsg);
        }while(true);
#endif
    }


    sp<Retval> Net::setNonBlock(int sock, bool nonblock){
#ifdef _WIN32
        u_long imode = (int)nonblock;
        if( ::ioctlsocket(sock, FIONBIO, &imode) != 0 )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not ioctlsocket(FIOBIO)");
        return NULL;
#else
        if(nonblock)
            return setFcntl(sock, O_NONBLOCK);
        return removeFcntl(sock, O_NONBLOCK);
#endif
    }

    sp<Retval> Net::setSigPipe(int sock, bool bPipe){
#if defined(__APPLE__)
        sp<Retval> retval;
        if(bPipe){
            if(DFW_RET(retval, setSockOptInt(sock, SOL_SOCKET, SO_NOSIGPIPE, 1)))
                return DFW_RETVAL_D(retval);
        }else{
            if(DFW_RET(retval, setSockOptInt(sock, SOL_SOCKET, SO_NOSIGPIPE, 0)))
                return DFW_RETVAL_D(retval);
        }
        return NULL;
#else
        DFW_UNUSED(sock);
        DFW_UNUSED(bPipe);
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not support set sig pipe.");
#endif
    }

    sp<Retval> Net::setTcpNoDelay(int sock, bool val){
        sp<Retval> retval;
        if(val){
            if(DFW_RET(retval, setSockOptInt(sock, IPPROTO_TCP, TCP_NODELAY, 1)))
                return DFW_RETVAL_D(retval);
        }else{
            if(DFW_RET(retval, setSockOptInt(sock, IPPROTO_TCP, TCP_NODELAY, 0)))
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    DFW_STATIC
    bool Net::isInprogress(int eno){
        switch(eno){
        case EINPROGRESS :
#ifdef _WIN32
        case WSAENOTCONN :
        case WSAEINPROGRESS : 
        case 10035 :
#endif
            return true;
        }
        return false;
    }



    String Net::hexstringToIp(const char* hexstring){
        String s = hexstring;
        if( s.indexOf("0x")==0 || s.indexOf("0X")==0 ){
            s.shift(2);
        }

        if(s.length()==8){
            char str[20];
            struct in_addr addr;
            uint32_t v = ::strtoul(s.toChars(), NULL, 16);
            ::memset(str, 0, 20);
            addr.s_addr = v;
            ::inet_ntop(AF_INET, &addr, str, 20);
            s = str;
        }else if(s.length()==32){
            char str[60]; ::memset(str, 0, 60);
            char test[9]; test[8] = '\0';
            uint32_t a6[4];
            const char* p = s.toChars();
            for(int i=0; i<4; i++){
                ::memcpy(test, p+(i*8), 8);
                a6[i] = ::strtoul(test, NULL, 16);
            }
            ::inet_ntop(AF_INET6, &a6[0], str, 60);
            s = str;
        }
        return s;
    }

};

