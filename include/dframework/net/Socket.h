#ifndef DFRAMEWORK_NET_SOCKET_H
#define DFRAMEWORK_NET_SOCKET_H
#define DFRAMEWORK_NET_SOCKET_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/net/URI.h>

#if defined(_WIN32)
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

#ifdef __ANDROID__
# include <sys/socket.h>
#endif

#ifdef _WIN32
#ifndef DFW_D_SOCKLEN_T
#define DFW_D_SOCKLEN_T
typedef int socklen_t;
#endif
#endif

#ifdef __cplusplus
namespace dframework {

    class ServerSocket;
    class ClientSocket;
    class ServerAccept;
    
    class Socket : public Object
    {
    public:
        Socket();
        virtual ~Socket();
        
    private:
        int     m_iHandle;
        String  m_sIp;
        int m_iPort;
        int m_iAddrType;    // default : AF_INET.
        int m_iSockType;    // defualt : SOCK_STREAM.

        int m_ConnTimeMSec; // default : 5000. If this is zero, block mode.
        int m_TimeoutMSec;  // default : 5000. If this is zero, block mode.
        int m_RecvSoSize;   // os socket buffer size
        int m_SendSoSize;   // os socket buffer size

        time_t m_useTime;

        sp<Retval> wait(int rdwr, int msec/*1/1000sec*/);
        sp<Retval> connect_raw();
        sp<Retval> connect_real();
             
    public:

        sp<Retval> fcntl_k(int *out, int cmd, int arg);
        sp<Retval> setFcntl(int value);
        sp<Retval> removeFcntl(int value);

        sp<Retval> getSockOpt(int level, int name, void* value, socklen_t* len);
        sp<Retval> getSockOptError(int *eno);

        sp<Retval> setSockOpt(int level, int name, const void* value, socklen_t len);

        sp<Retval> setLinger(int onoff, int linger);
        sp<Retval> setKeepAlive(int alive);
        sp<Retval> setNonBlock(bool nonblock);
        sp<Retval> setTcpNoDelay(bool val);

        bool isConnect() { return DFW_SOCK_ENABLE(m_iHandle); }

        sp<Retval> open(int addrtype, int socktype);
        virtual sp<Retval> close();

        sp<Retval> send_raw(size_t *o_size, const char *data
                          , size_t size, bool iswait);
        inline sp<Retval> send(size_t *o_size
                             , const char *buffer
                             , size_t size){
            return send_raw(o_size, buffer, size, false);
        }
        inline sp<Retval> send_wait(size_t *o_size
                                  , const char *buffer
                                  , size_t size){
            return send_raw(o_size, buffer, size, true);
        }

        sp<Retval> recv_raw(char* o_data, size_t *o_size
                          , size_t size, bool iswait);
        inline sp<Retval> recv(char* o_data, size_t *o_size, size_t size){
            return recv_raw(o_data, o_size, size, false);
        }
        inline sp<Retval> recv_wait(char* o_data, size_t *o_size, size_t size){
            return recv_raw(o_data, o_size, size, true);
        }

        sp<Retval> setNonBlockSocket();
        
        sp<Retval> connect(const char *uri);
        sp<Retval> connect(URI& uri);
        sp<Retval> connectbyip(int addrType
                             , const char *ip, int port, bool isopen);

        inline sp<Retval> wait_connect(){ return wait(1, m_ConnTimeMSec); }
        inline sp<Retval> wait_send(){ return wait(1, m_TimeoutMSec); }
        inline sp<Retval> wait_recv(){ return wait(0, m_TimeoutMSec); }

        inline void setConnectTimeout(int msec){ m_ConnTimeMSec = msec; }
        inline void setTimeout(int msec){ m_TimeoutMSec = msec; }

        inline const char *getIp() { return m_sIp.toChars(); }
        inline int getHandle() { return m_iHandle; }

        sp<Retval> setRecvBufferSize(int size);
        sp<Retval> setSendBufferSize(int size);
        sp<Retval> getRecvBufferSize(int* size);
        sp<Retval> getSendBufferSize(int* size);

        sp<Retval> getSendBufferLeftSize(int *size);

        friend class ServerSocket;
        friend class ClientSocket;
        friend class ServerAccept;
    };
    
};
#endif

#endif /* DFRAMEWORK_NET_SOCKET_H */
