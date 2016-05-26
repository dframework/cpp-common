#ifndef DFRAMEWORK_HTTPD_HTTPDCONFIGURE_H
#define DFRAMEWORK_HTTPD_HTTPDCONFIGURE_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/lang/Integer.h>
#include <dframework/httpd/HttpdHost.h>
#include <dframework/httpd/HttpdModules.h>

#ifdef __cplusplus
namespace dframework {

    class HttpdService;

    class HttpdConfigure : public Object
    {
    private:
        int                    m_serverType;

        Array<HttpdHost>       m_aHosts;
        String                 m_sModPath;
        ArraySorted<HttpdMod>  m_aMods;
        bool                   m_bStoped;

        String                 m_sUserAgent;
        bool                   m_bPrintReqHeader;
        bool                   m_bPrintRespHeader;

    public:
        HttpdConfigure();
        virtual ~HttpdConfigure();

        inline void setServerType(int type){
            m_serverType = type;
        }
        inline int getServerType(){ return m_serverType; }
        inline void stop() { m_bStoped = true; }
        inline bool isstop() { return m_bStoped; }

        inline void setUserAgent(const char* agent){
            m_sUserAgent = agent;
        }
        inline void setUserAgent(const String& agent){
            m_sUserAgent = agent;
        }
        inline const char* getUserAgent(){
            return m_sUserAgent.toChars();
        }

        inline void setPrintRequestHeader(bool bUse){
            m_bPrintReqHeader = bUse;
        }
        inline void setPrintResponseHeader(bool bUse){
            m_bPrintRespHeader = bUse;
        }

        inline bool isPrintRequestHeader(){ return m_bPrintReqHeader; }
        inline bool isPrintResponseHeader(){ return m_bPrintRespHeader; }

        // ----------------------------------------------------------
        sp<Retval> addHost(sp<HttpdHost>& host);
        sp<HttpdHost> getHost(String& sAlias);
        sp<HttpdHost> getHost(String& sHost, int port=80);

        // ----------------------------------------------------------
        void setModPath(const char* path) { m_sModPath = path; }
        void setModPath(String& sPath) { setModPath(sPath.toChars()); }
        sp<Retval> addModule(sp<HttpdMod>& mod);

        friend class HttpdService;
    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDCONFIGURE_H */

