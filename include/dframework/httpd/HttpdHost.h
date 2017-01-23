#ifndef DFRAMEWORK_HTTPD_HTTPDHOST_H
#define DFRAMEWORK_HTTPD_HTTPDHOST_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/net/URI.h>
#include <dframework/net/ServerAccept.h>
#include <dframework/httpd/HttpdModules.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdService;
    class HttpdConfigure;
    class HttpdHost;

    class HttpdHost : public Object
    {
    public:
        class AliasUri : public Object
        {
        private:
            int        m_current;
            String     m_sAliasName;
            Array<URI> m_aUris;

        public:
            AliasUri();
            virtual ~AliasUri();

            inline String& getAliasName() { return m_sAliasName; }
            inline Array<URI>& getUris()  { return m_aUris; }

            inline int getCurrent() { return m_current; }
            inline void setCurrent(int cur) { m_current = cur; }
            inline int size() { return m_aUris.size(); }
            inline sp<URI> getUri(int k) { return m_aUris.get(k); }

            friend class HttpdHost;
        };

    private:
        sp<ServerSocket> m_serverSocket;
        String  m_sHost;
        String  m_sUserAgent;
        bool    m_bReuseAddr;

        sp<AliasUri>           m_aBaseDocumentRoot;
        Array<AliasUri>        m_aAliasDocumentRoot;

        ArraySorted<HttpdMod>  m_aMods;
        sp<Object>             m_configure;

    public:
        HttpdHost(sp<Object>& configure);
        virtual ~HttpdHost();

        int getPort();
        String& getHost();

        inline setReuseAddr(bool bReuseAddr){
            m_bReuseAddr = bReuseAddr;
        }

        sp<Retval> ready(int port);
        sp<Retval> ready(const char* sHost, int port);
        inline sp<Retval> ready(String& sHost, int port){
            return ready(sHost.toChars(), port);
        }
        sp<Retval> ready(int sport, int eport);
        sp<Retval> ready(const char* sHost, int sport, int eport);
        inline sp<Retval> ready(String& sHost, int sport, int eport){
            return ready(sHost.toChars(), sport, eport);
        }

        inline void setUserAgent(const char* sAgent){
            m_sUserAgent = sAgent;
        }
        inline void setUserAgent(const String& sAgent){
            m_sUserAgent = sAgent;
        }

        /////////////////////////////////////////////////////////////
        sp<AliasUri> getAliasUri(const char* path);
        sp<Retval> addDocumentRoot(const char* document
                                 , const char* alias=NULL);
        inline sp<Retval> addDocumentRoot(String& sDocument){
            return addDocumentRoot(sDocument.toChars());
        }
        inline sp<Retval> addDocumentRoot(String& sDocument, String& alias){
            return addDocumentRoot(sDocument.toChars(), alias.toChars());
        }

        /////////////////////////////////////////////////////////////
        sp<Retval> addModule(sp<HttpdMod>& mod);
        int getModSize();
        sp<HttpdMod> getMod(int k);

        friend class HttpdConfigure;
        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDHOST_H */

