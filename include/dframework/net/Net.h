#ifndef DFRAMEWORK_NET_NET_H
#define DFRAMEWORK_NET_NET_H

#include <dframework/base/Retval.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <signal.h>
#else
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#ifdef _WIN32
#ifndef DFW_D_SOCKLEN_T
#define DFW_D_SOCKLEN_T
typedef int socklen_t;
#endif
#endif

#ifdef __cplusplus
namespace dframework {

    class Net
    {
    public:
        static sp<Retval> socket(int* out_sock, int family, int type);
        static sp<Retval> bind(int sock, int family, int port);
        static sp<Retval> listen(int sock, int backlog);
        static sp<Retval> accept(int* out_client_sock, String& sIp
                         , int server_sock);

        static sp<Retval> getSockOpt(int sock, int level, int name
                                   , void* val, socklen_t* len);
        static sp<Retval> getSockOptInt(int sock, int level, int name, int* val);

        static sp<Retval> setSockOpt(int sock, int level, int name
                                   , const void* val, socklen_t len);
        static sp<Retval> setSockOptInt(int sock, int level, int name, int val);

        static sp<Retval> setLinger(int sock, int onoff, int linger);
        static sp<Retval> setKeepAlive(int sock, int alive);
        static sp<Retval> setSigPipe(int sock, bool bPipe);
        static sp<Retval> setTcpNoDelay(int sock, bool val);

        static sp<Retval> setFcntl(int sock, int v);
        static sp<Retval> removeFcntl(int sock, int v);
        static sp<Retval> setNonBlock(int sock, bool nonblock);

        static bool isInprogress(int eno);

    private:
        static sp<Retval> fcntl_k(int *out, int sock, int cmd, int arg);


    };

};
#endif

#endif /* DFRAMEWORK_NET_NET_H */

