#include <dframework/net/HostManager.h>

namespace dframework {

    HostManager::HostLookup::HostLookup(){
    }

    HostManager::HostLookup::~HostLookup(){
    }

    void HostManager::HostLookup::run(){
        sp<Retval> retval;
        while(HostManager::REFRESH_SECONDS!=-1){
            sleep(HostManager::REFRESH_SECONDS);
            for(int k=0; k<Hostname::m_Cache.size(); k++){
                if(HostManager::REFRESH_SECONDS==-1)
                    return;
                sp<Hostname::CacheData> cd = Hostname::m_Cache.get(k);
                if(cd.has()){
                    sp<Hostname> host = new Hostname();
                    String s = String::format("http://%s/"
                                            , cd->m_sHost.toChars());
                    if( DFW_RET(retval, host->get(s.toChars(), false)) ){
                        // FIXME:
                    }
                }
            }
        } // end while()
    }

    // --------------------------------------------------------------

    /* static */
    int HostManager::REFRESH_SECONDS = 2;

    /* static */
    Object HostManager::m_lock;

    /* static */
    sp<HostManager::HostLookup> HostManager::m_hostLookup;

    /* static */
    sp<Retval> HostManager::start(){
        AutoLock _l(&m_lock);
        sp<Retval> retval;
        if(!m_hostLookup.has()){
            m_hostLookup = new HostLookup();
            if( DFW_RET(retval, m_hostLookup->start()) )
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    /* static */
    void HostManager::stop(){
        AutoLock _l(&m_lock);
        HostManager::REFRESH_SECONDS = -1;
    }

    /* static */
    sp<Retval> HostManager::join(){
        AutoLock _l(&m_lock);
        sp<Retval> retval;
        if(m_hostLookup.has()){
            if( DFW_RET(retval, m_hostLookup->join()) )
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

};

