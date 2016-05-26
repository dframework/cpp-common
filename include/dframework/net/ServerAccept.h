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

    public:
        ServerSocket();
        virtual ~ServerSocket();

        inline int getPort() { return m_iPort; }
        inline int getHandle() { return m_iHandle; }

        sp<Retval> ready(int port);
        sp<Retval> ready(int sport, int eport);
        sp<Retval> create(int iport);

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
        bool m_bLive;
        bool m_bStarted;
        ArraySorted<ServerSocket> m_aSockList;

        Poll m_poll;
        sp<Retval> accept_poll();

        // FIXME: sp<Retval> accept_no_poll(bool* bAccept);

    public:
        ServerAccept();
        virtual ~ServerAccept();

        sp<Retval> appendPort(int port);
        sp<Retval> appendServerSocket(sp<ServerSocket>& sock);

        virtual sp<Retval> start();
        virtual void run();
        virtual void stop();

        sp<Retval> accept();

        virtual sp<Retval> onAccept(sp<ClientSocket>& sock);

    private:
        sp<Retval> repairServer(sp<ServerSocket>& sock);
    };

};
#endif

#endif /* DFRAMEWORK_NET_SERVERACCEPT_H */

