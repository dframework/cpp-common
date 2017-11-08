#ifndef DFRAMEWORK_NET_SERVERACCEPT_H
#define DFRAMEWORK_NET_SERVERACCEPT_H

#include <dframework/base/Thread.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/net/Socket.h>
#include <dframework/net/Poll.h>

#ifdef __cplusplus
namespace dframework {

    class ServerAccept;

    class ServerSocket : public Object
    {
    private:
        int m_iPort;
        int m_iStartPort; // safe port for client.
        int m_iEndPort;   // safe port for client.
        int m_iHandle;
        int m_iPollNum;
        bool m_bReuseAddr;

    public:
        ServerSocket();
        virtual ~ServerSocket();

        inline int getPort() { return m_iPort; }
        inline int getHandle() { return m_iHandle; }

        void setReuseAddr(bool bReuseAddr);
        sp<Retval> ready(int port);
        sp<Retval> ready(int sport, int eport);
        sp<Retval> create(int iport);
        void close();

        DFW_OPERATOR_EX_DECLARATION(ServerSocket, m_iHandle);
    
        friend class ServerAccept;
    };

    class ClientSocket : public Socket
    {
    private:
        int m_iServerPort;

    public:
        ClientSocket(int sockfd, String& sIp, int port);
        virtual ~ClientSocket();

        inline int getServerPort() { return m_iServerPort; }
    };

    // --------------------------------------------------------------

    class ServerAccept : public Thread
    {
    private:
        //bool m_bStarted;
        int  m_iServerStatus;
        bool m_bReuseAddr;
        ArraySorted<ServerSocket> m_aSockList;

        Poll m_poll;
        sp<Retval> accept_poll();

        // FIXME: sp<Retval> accept_no_poll(bool* bAccept);

    public:
        static const int STATUS_STOP = -1;
        static const int STATUS_NONE = 0;
        static const int STATUS_RUNNING = 2;
        
    public:
        ServerAccept();
        virtual ~ServerAccept();

        virtual void onCleanup();
        
        void setReuseAddr(bool bReuseAddr);
        sp<Retval> appendPort(int port);
        sp<Retval> appendServerSocket(sp<ServerSocket>& sock);
        
        virtual sp<Retval> start();
        virtual void run();
        virtual void stop();
        void run_l();

        sp<Retval> accept();
        //virtual bool isStart() { return m_bStarted; }
        virtual int  serverStatus() { return m_iServerStatus; }

        virtual sp<Retval> onAccept(sp<ClientSocket>& sock);
        
        sp<Retval> clearServerSockets();
    private:
        sp<Retval> repairServerSocket(sp<ServerSocket>& sock);
    };

};
#endif

#endif /* DFRAMEWORK_NET_SERVERACCEPT_H */

