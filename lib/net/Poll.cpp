#include <dframework/net/Poll.h>


#define DFW_POLL_PIECE_SIZE 1024


namespace dframework {

    int win32_poll(struct pollfd *fds, unsigned int nfds, int timo)
    {
        struct timeval timeout, *toptr;
        fd_set ifds, ofds, efds, *ip, *op;
        unsigned int i, rc;

        /* Set up the file-descriptor sets in ifds, ofds and efds. */
        FD_ZERO(&ifds);
        FD_ZERO(&ofds);
        FD_ZERO(&efds);
        for (i = 0, op = ip = 0; i < nfds; ++i) {
            fds[i].revents = 0;
            if(fds[i].events & (POLLIN|POLLPRI)) {
                ip = &ifds;
                FD_SET(fds[i].fd, ip);
            }
            if(fds[i].events & POLLOUT) {
                op = &ofds;
                FD_SET(fds[i].fd, op);
            }
            FD_SET(fds[i].fd, &efds);
        }

        /* Set up the timeval structure for the timeout parameter */
        if(timo < 0) {
             toptr = 0;
        } else {
            toptr = &timeout;
            timeout.tv_sec = timo / 1000;
            //timeout.tv_usec = (timo - timeout.tv_sec * 1000) * 1000;
            timeout.tv_usec = ((timo % 1000) * 1000);
        }

#ifdef DEBUG_POLL
        printf("Entering select() sec=%ld usec=%ld ip=%lx op=%lx\n",
            (long)timeout.tv_sec, (long)timeout.tv_usec, (long)ip, (long)op);
#endif
        rc = ::select(0, ip, op, &efds, toptr);
#ifdef DEBUG_POLL
        printf("Exiting select rc=%d\n", rc);
#endif

        if(rc == 0)
            return rc;
        else if(rc < 0)
            return -1;

        if(rc > 0) {
            for ( i = 0; i < nfds; ++i) {
                int fd = fds[i].fd;
                if(fds[i].events & (POLLIN|POLLPRI) && FD_ISSET(fd, &ifds))
                    fds[i].revents |= POLLIN;
                if(fds[i].events & POLLOUT && FD_ISSET(fd, &ofds))
                    fds[i].revents |= POLLOUT;
                if(FD_ISSET(fd, &efds))
                    /* Some error was detected ... should be some way to know. */
                    fds[i].revents |= POLLHUP;
#ifdef DEBUG_POLL
                printf("%d %d %d revent = %x\n",
                    FD_ISSET(fd, &ifds), FD_ISSET(fd, &ofds), FD_ISSET(fd, &efds),
                    fds[i].revents
                  );
#endif
            }
        }
        return rc;
    }


    Poll::Poll(){
        m_used = 0;
        m_memsize = 0;
        m_polls = NULL;
        m_ppsocks = NULL;
    }

    Poll::~Poll(){
        {
        AutoLock _l(this);
        for(int k=0; k<m_used; k++){
            Object* pobj = (Object*)m_ppsocks[k];
            if( DFW_SOCK_ENABLE(m_polls[k].fd) )
               DFW_SOCK_CLOSE(m_polls[k].fd);
            if(pobj!=NULL)
               sp<Object>::docking( pobj );
        }
        m_used = 0;
        m_memsize = 0;
        DFW_FREE(m_polls);
        DFW_FREE(m_ppsocks);
        }
    }

    int Poll::size(){
        AutoLock _l(this);
        return m_used;
    }

    sp<Retval> Poll::prepare()
    {
        AutoLock _l(this);
        static const int pollsize = sizeof(struct pollfd);
        struct pollfd* polls = NULL;
        void** ppsocks = NULL;
        int memsize = m_memsize;
        if(0==memsize) {
            memsize = DFW_POLL_PIECE_SIZE;
            if( !(polls = (struct pollfd*)::malloc(pollsize*memsize)) )
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
            if( !(ppsocks = (void**)::malloc(sizeof(void*)*memsize)) ){
                ::free(polls);
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
            }
            ::memset(polls, -1, pollsize*memsize);
            ::memset(ppsocks, 0, sizeof(void*)*memsize);
	}else{
            memsize += DFW_POLL_PIECE_SIZE;
            if( !(polls = (struct pollfd*)::malloc(pollsize*memsize)) )
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
            if( !(ppsocks = (void**)::malloc(sizeof(void*)*memsize)) ){
                ::free(polls);
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
            }
            ::memset(polls, -1, pollsize*memsize);
            ::memset(ppsocks, 0, sizeof(void*)*memsize);
            ::memcpy(polls, m_polls, pollsize*m_memsize);
            ::memcpy(ppsocks, m_ppsocks, sizeof(void*)*m_memsize);
        }
        m_polls = polls;
        m_ppsocks = ppsocks;
        m_memsize = memsize;
        return NULL;
    }

    sp<Retval> Poll::append(int* out_num, int fd, sp<Object> obj)
    {
        AutoLock _l(this);
        sp<Retval> retval;

        if( m_used >= (m_memsize-1)  ) {
            if( DFW_RET(retval, prepare()) )
                return DFW_RETVAL_D(retval);
        }

        void* sock = obj.has() ? obj.get() : NULL;
        if( obj.has() )
            sp<Object>::undocking( obj.get() );

        m_polls[m_used].fd = fd;
        m_polls[m_used].events = m_events;
        m_polls[m_used].revents = 0;
        m_ppsocks[m_used] = sock;

        if(out_num) *out_num = m_used;
        m_used++;
        return NULL;
    }

    sp<Retval> Poll::remove(int position) {
        AutoLock _l(this);

        int last;
        if( !m_used ) return NULL;
        if( position<0 ) return NULL;
        if( m_used <= position ) return NULL;

        Object* pobj = (Object*)m_ppsocks[position];
        last = m_used - 1;

        m_polls[position] = m_polls[last];
        m_ppsocks[position] = m_ppsocks[last];
        m_polls[last].fd = -1;
        m_ppsocks[last] = NULL;
        m_used--;

        sp<Object>::docking( pobj );
        return NULL;
    }

    sp<Retval> Poll::setFd(int position, int fd){
        AutoLock _l(this);
        if( !m_used )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "used is zero.");
        if( position<0 )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "position(%d)<0", position);
        if( m_used <= position )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "used(%d) <= position(%d)", m_used, position);
        m_polls[position].fd = fd;
        return NULL;
    }

    sp<Retval> Poll::getObject(sp<Object>& out, int position){
        AutoLock _l(this);
        if( !m_used )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "used is zero.");
        if( position<0 )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "position(%d)<0", position);
        if( m_used <= position )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "used(%d) <= position(%d)", m_used, position);

        out = (Object*)m_ppsocks[position];
        return NULL;
    }

    sp<Retval> Poll::getEvents(int* out, int position){
        AutoLock _l(this);
        if( !m_used )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "used is zero.");
        if( position<0 )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "position(%d)<0", position);
        if( m_used <= position )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "used(%d) <= position(%d)", m_used, position);
        *out = m_polls[position].events;
        return NULL;
    }

    void Poll::resetRevents(int position){
        AutoLock _l(this);
        if( m_used && (m_used > position) )
            m_polls[position].revents = 0;
    }

    sp<Retval> Poll::poll(int* out_count, int timeout)
    {
        AutoLock _l(this);
        int count;
        *out_count = 0;
        if( m_used < 0 ) return NULL;

#ifdef DFW_USE_WSAPOLLFD
        count = ::WSAPoll(m_polls, m_used, timeout);
#elif defined(_WIN32)
        count = win32_poll( m_polls, m_used, timeout);
#else
        count = ::poll( m_polls, m_used, timeout);
#endif

        if( count >= 0 ) {
            *out_count = count;
            return NULL;
        }

        dfw_retno_t rno;
#ifdef DFW_USE_WSAPOLLFD
        int eno = WSAGetLastError();
        const char* emsg = Retval::wsa2value(&rno, eno, "No Poll");
#else
        int eno = errno;
        const char* emsg = Retval::errno_short(&rno, eno, "No Poll");
#endif

        if(rno==DFW_ERROR) rno = DFW_E_POLL;
        return DFW_RETVAL_NEW_MSG(rno, eno, emsg);
    }

    int Poll::getFd(int position){
        AutoLock _l(this);
        return m_polls[position].fd;
    }

    int Poll::getPollErr(int position){
        AutoLock _l(this);
        if( m_polls[position].revents & POLLERR )
            return POLLERR;
        if( m_polls[position].revents & POLLNVAL )
            return POLLNVAL;
        if( m_polls[position].revents & POLLHUP )
            return POLLHUP;
        return 0;
    }

    bool Poll::isPollIn(int position){
        AutoLock _l(this);
        if( m_polls[position].revents & (POLLIN|POLLPRI) )
            return true;
        return false;
    }

    bool Poll::isPollOut(int position){
        AutoLock _l(this);
        if( m_polls[position].revents & POLLOUT )
            return true;
        return false;
    }



};

