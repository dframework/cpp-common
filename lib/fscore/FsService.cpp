#ifndef _WIN32
#include <dframework/fscore/FsService.h>
#include <syslog.h>

namespace dframework {

    FsService::FsService(){
        m_wl = new FsWorkerList();
        m_rq = new FsRequestQueue();
    }

    FsService::~FsService(){
    }

    sp<Retval> FsService::appendService(sp<FsContext>& context)
    {
        sp<Retval> retval;
        sp<FsService> worker;
        if( DFW_RET(retval, appendService(worker, context)) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> FsService::appendService(sp<FsService>& worker
                                    , sp<FsContext>& context)
    {
        sp<Retval> retval;
        if( DFW_RET(retval, createService(worker, context)) )
            return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, m_services.insert(worker)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> FsService::startService(){
        sp<Retval> retval;

        if(m_services.size()==0)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Not has service sites.");

        for(int k=0; k<m_services.size(); k++){
            sp<FsService> worker = m_services.get(k);
            if( DFW_RET(retval, worker->start(worker->getMountpoint())) )
                return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> FsService::stopService(){
        sp<Retval> retval;

        if(m_services.size()==0)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Not has service sites.");

        sp<FuseMount> mount = new FuseMount();
        for(int k=0; k<m_services.size(); k++){
            sp<FsService> worker = m_services.get(k);
            if( DFW_RET(retval, mount->umount( worker->getMountpoint() )) ){
                // FIXME:
            }
        }

        return NULL;
    }

    sp<Retval> FsService::createService(sp<FsService>& worker
                                      , sp<FsContext>& context)
    {
        sp<Retval> retval;

        if(!context->getMountpoint())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                , "mountpoint is null.");

        worker = newWorker();
        worker->setWorkerList(m_wl);
        worker->setRequestQueue(m_rq);
        worker->setContext(context);

        if( DFW_RET(retval, worker->readyService()) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void FsService::joinAll() {
        joinMainAll();
        joinWorkerAll();
    }

    void FsService::joinMainAll() {
        sp<Retval> retval;

        do{
            int jcount = 0;
            for(int k=0; k<m_services.size(); k++){
                sp<FsService> worker = m_services.get(k);
                if(worker.has()){
                    worker->join();
                    jcount++;
                }
            }

            if(m_services.size() <= jcount){
                break;
            }
        }while(true);
    }

    void FsService::joinWorkerAll() {
        sp<Retval> retval;
        do{
            for(int k=0; k<m_wl->workerSize(); k++){
                sp<FsService> worker = m_wl->getWorker(k);
                worker->setLanding(true);
                m_rq->broadcast();
            }

            if(m_wl->workerSize()==0){
                break;
            }
            usleep(1000*100);
        }while(true);
    }

}; // end namespace dframework
#endif

