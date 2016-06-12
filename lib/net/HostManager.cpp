#include <dframework/net/HostManager.h>

namespace dframework {

    HostManager::HostLookup::HostLookup(){
        m_refresh_seconds = 2;
    }

    HostManager::HostLookup::~HostLookup(){
    }

    sp<Retval> HostManager::HostLookup::start(){
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not enable this function. "
                     "you are call start(unsigned refresh_seconds);");
    }

    sp<Retval> HostManager::HostLookup::start(unsigned refresh){
        sp<Retval> retval;
        m_refresh_seconds = refresh;
        if( DFW_RET(retval, Thread::start()) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void HostManager::HostLookup::run(){
        sp<Retval> retval;
        while(m_refresh_seconds){
            sleep(m_refresh_seconds);
            for(int k=0; k<Hostname::m_Cache.size(); k++){
                if( !m_refresh_seconds ) return;
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
    Object HostManager::m_lock;

    /* static */
    sp<HostManager::HostLookup> HostManager::m_hostLookup;

    /* static */
    sp<Retval> HostManager::start(unsigned refresh, unsigned timesec){
        AutoLock _l(&m_lock);
        sp<Retval> retval;

        Hostname::CACHE_TIME_SEC = timesec;

        if(!m_hostLookup.has()){
            m_hostLookup = new HostLookup();
            if( DFW_RET(retval, m_hostLookup->start(refresh)) )
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    /* static */
    sp<Retval> HostManager::stop(){
        AutoLock _l(&m_lock);
        sp<Retval> retval;
        if( m_hostLookup.has() ){
            m_hostLookup->m_refresh_seconds = 0;
            if( DFW_RET(retval, m_hostLookup->join()) )
                return DFW_RETVAL_D(retval);
            m_hostLookup = NULL;
        }
        return NULL;
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

