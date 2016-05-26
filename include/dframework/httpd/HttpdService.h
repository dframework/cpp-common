#ifndef DFRAMEWORK_HTTPD_HTTPDSERVICE_H
#define DFRAMEWORK_HTTPD_HTTPDSERVICE_H

#include <dframework/base/Thread.h>
#include <dframework/httpd/HttpdAcceptor.h>
#include <dframework/httpd/HttpdWorker.h>
#include <dframework/httpd/HttpdSendStream.h>
#include <dframework/httpd/HttpdSendLocalFile.h>
#include <dframework/httpd/HttpdConfigure.h>

namespace dframework {

    class HttpdService : public Object
    {
    public:
        static const int SERVER_TYPE_POLL   = 1;
        static const int SERVER_TYPE_THREAD = 2;

    private:
        int               m_defaultServerType;
        int               m_serverType;
        bool              m_bReady;

        sp<HttpdAcceptor> m_accept;
        sp<HttpdWorker>   m_worker;
        sp<HttpdSendStream>    m_stream;
        sp<HttpdSendLocalFile> m_localfile;
        sp<HttpdConfigure>     m_configure;

    public:
        HttpdService();
        virtual ~HttpdService();

        virtual sp<Retval> ready();
        virtual sp<Retval> start();
        virtual sp<Retval> stop();
        virtual void join();

        sp<HttpdConfigure> getConfigure(){
            return m_configure;
        }

        sp<Retval> setServerType(int serverType);
        inline int getDefaultServerType(){ return m_defaultServerType; }
        int getServerType();

        inline void setUserAgent(const char* agent){
            m_configure->setUserAgent(agent);
        }
        inline void setUserAgent(const String& agent){
            m_configure->setUserAgent(agent);
        }
        inline const char* getUserAgent(){
            return m_configure->getUserAgent();
        }

        inline void setPrintRequestHeader(bool bUse){
            m_configure->setPrintRequestHeader(bUse);
        }
        inline void setPrintResponseHeader(bool bUse){
            m_configure->setPrintResponseHeader(bUse);
        }

        inline bool isPrintRequestHeader(){ return m_configure->isPrintRequestHeader(); }
        inline bool isPrintResponseHeader(){ return m_configure->isPrintResponseHeader(); }


        /////////////////////////////////////////////////////////////
        inline sp<Retval> addHost(sp<HttpdHost>& host){
            return m_configure->addHost(host);
        }

        /////////////////////////////////////////////////////////////
        inline void setModPath(const char* path) { 
            m_configure->setModPath(path); 
        }
        inline void setModPath(String& sPath) { 
            setModPath(sPath.toChars());
        }
        inline sp<Retval> addModule(sp<HttpdMod>& mod){
            return m_configure->addModule(mod);
        }

    };

};

#endif /* DFRAMEWORK_HTTPD_HTTPDSERVICE_H */

