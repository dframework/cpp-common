#include <dframework/httpd/HttpdConfigure.h>

namespace dframework {

    HttpdConfigure::HttpdConfigure(){
        m_bStoped = false;
    }

    HttpdConfigure::~HttpdConfigure(){
    }

    sp<Retval> HttpdConfigure::addHost(sp<HttpdHost>& host){
        AutoLock _l(this);
        sp<Retval> retval;
        if( DFW_RET(retval, m_aHosts.insert(host)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<HttpdHost> HttpdConfigure::getHost(String& sAlias){
        AutoLock _l(this);
        String sHost;
        String sPort;
        int port;
        int index = sAlias.indexOf(':');
        if( index==-1 ){
            sHost = sAlias;
            port = 80;
        }else{
            sHost = sAlias.substring(0, index);
            sPort = sAlias.substring(index+1);
            port = Integer::parseInt(sPort);
        }

        return getHost(sHost, port);
    }

    sp<HttpdHost> HttpdConfigure::getHost(String& sHost, int port){
        AutoLock _l(this);
        sp<Retval> retval;

        for(int k=0; k<m_aHosts.size(); k++){
            sp<HttpdHost> host = m_aHosts.get(k);
            if( host.has() ){
                if( port == host->getPort()
                        && sHost.equals(host->getHost())){
                    return host;
                }
            }
        }

        for(int k=0; k<m_aHosts.size(); k++){
            sp<HttpdHost> host = m_aHosts.get(k);
            if( host.has() ){
                if( sHost.equals(host->getHost()) )
                    return host;
            }
        }

        for(int k=0; k<m_aHosts.size(); k++){
            sp<HttpdHost> host = m_aHosts.get(k);
            if( host.has() ){
                if( port == host->getPort() )
                    return host;
            }
        }

        return NULL;
    }

    // --------------------------------------------------------------

    sp<Retval> HttpdConfigure::addModule(sp<HttpdMod>& mod){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdMod> find = m_aMods.get(mod);
        if(find.has())
            return NULL;
        if( DFW_RET(retval, m_aMods.insert(mod)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }
    
};

