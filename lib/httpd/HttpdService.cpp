#include <dframework/httpd/HttpdService.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpdService::HttpdService(){
        m_serverType = 0;
        m_defaultServerType = 0;
        m_bReady = false;

        m_configure = new HttpdConfigure();
        m_accept = new HttpdAcceptor();
        m_accept->m_configure = m_configure;
    }

    HttpdService::~HttpdService(){
    }

    sp<Retval> HttpdService::setServerType(int serverType){
        AutoLock _l(this);
        switch(serverType){
        case SERVER_TYPE_POLL:
        case SERVER_TYPE_THREAD :
            m_defaultServerType = serverType;
            break;
        default:
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                                    , "Not find server type. type=%d"
                                    , serverType);
        }
        return NULL;
    }

    int HttpdService::getServerType(){
        AutoLock _l(this);
        if( m_serverType == 0 ){
            if( m_defaultServerType != 0 )
                return m_defaultServerType;
            return 0;
        }
        return m_serverType;
    }

    sp<Retval> HttpdService::ready(){
        sp<Retval> retval;
        AutoLock _l(this);

        // check server type.
        int type = getServerType();
        if( type == 0 ){
            type = m_serverType = SERVER_TYPE_POLL;
        }

        switch( type ){
        case SERVER_TYPE_POLL :
            m_worker = new HttpdWorker();
            m_stream = new HttpdSendStream();
            m_localfile = new HttpdSendLocalFile();

            m_accept->m_worker = m_worker;
            m_stream->m_worker = m_worker;
            m_localfile->m_worker = m_worker;

            m_worker->m_configure = m_configure;
            m_stream->m_configure = m_configure;
            m_localfile->m_configure = m_configure;

            m_worker->m_stream = m_stream;
            m_worker->m_localfile = m_localfile;
            break;

        case SERVER_TYPE_THREAD :
            break;

        default :
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                                    , "Not find server type. type=%d"
                                    , type);
        }

        //
        m_configure->setServerType(type);

        // modules
        for(int k=0; k<m_configure->m_aMods.size(); k++){
            sp<HttpdMod> mod = m_configure->m_aMods.get(k);
            if(DFW_RET(retval,mod->loadModule(m_configure->m_sModPath)))
                return DFW_RETVAL_D(retval);
        }

        // hosts and ports
        for(int k=0; k<m_configure->m_aHosts.size(); k++){
            sp<HttpdHost> host = m_configure->m_aHosts.get(k);
            if(!host.has()){
                //FIXME:
                continue;
            }
            if( DFW_RET(retval, m_accept->appendServerSocket(
                                              host->m_serverSocket)) ){
                return DFW_RETVAL_D(retval);
            }
        }

        m_bReady = true;
        return NULL;
    }

    sp<Retval> HttpdService::start(){
        sp<Retval> retval;

        if( !m_bReady ){
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                                    , "Not server ready");
        }

        int type = getServerType();
        switch( type ){
        case SERVER_TYPE_POLL :
            if( DFW_RET(retval, m_localfile->start()) )
                return DFW_RETVAL_D(retval);
            if( DFW_RET(retval, m_stream->start()) )
                return DFW_RETVAL_D(retval);
            if( DFW_RET(retval, m_worker->start()) )
                return DFW_RETVAL_D(retval);
            break;

        case SERVER_TYPE_THREAD :
            break;

        default :
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                                    , "Not find server type. type=%d"
                                    , type);
        }

        if( DFW_RET(retval, m_accept->start()) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    void HttpdService::join(){
        if( m_localfile.has() ) m_localfile->join();
        if( m_stream.has() )    m_stream->join();
        if( m_accept.has() )    m_accept->join();
        if( m_worker.has() )    m_worker->join();
    }

    sp<Retval> HttpdService::stop(){
        m_configure->stop();
        if( m_localfile.has() ) m_localfile->stop();
        if( m_stream.has() )    m_stream->stop();
        if( m_accept.has() )    m_accept->stop();
        if( m_worker.has() )    m_worker->stop();
        join();
        return NULL;
    }

};

