#ifndef DFRAMEWORK_NET_POLL_H
#define DFRAMEWORK_NET_POLL_H

#include <dframework/base/Retval.h>


#ifdef _WIN32
# if 0
#  define DFW_USE_WSAPOLLFD
# else
#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */
#define POLLNVAL    0x0020    /* Invalid request: fd not open */
struct pollfd {
    //SOCKET fd;        /* file descriptor */
    int fd;        /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
# endif
#endif

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <mstcpip.h>
#else
# include <sys/poll.h>
#endif

#ifdef __cplusplus
namespace dframework {

    int win32_poll(struct pollfd *fds, int nfds, int timo);

    class Poll : public Object
    {
    private:
        short           m_events;
        int             m_used;
        int             m_memsize;
#ifdef DFW_USE_WSAPOLLFD
        PWSAPOLLFD      m_polls;
#else
	struct pollfd* 	m_polls;
#endif
	void**          m_ppsocks;
    public:
        Poll();
        virtual ~Poll();

        inline void setEvents(short revents){
            m_events = revents;
        }
        int size();

        sp<Retval> clear();
        sp<Retval> append(int* out_num, int fd, sp<Object> obj);
        inline sp<Retval> append(int fd, sp<Object> obj){
            return append(NULL, fd, obj);
        }
        sp<Retval> remove(int position);
        sp<Retval> setFd(int position, int fd);
        sp<Retval> getObject(sp<Object>& out, int position);
        sp<Retval> getEvents(int* out, int position);
        void resetRevents(int position);
        sp<Retval> poll(int* out_count, int timeout);

        int getFd(int position);
        int getPollErr(int position);
        bool isPollIn(int position);
        bool isPollOut(int position);

    private:
        sp<Retval> prepare();
    };

};
#endif

#endif /* DFRAMEWORK_NET_POLL_H */

