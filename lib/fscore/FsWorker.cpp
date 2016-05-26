#ifndef _WIN32
#include <dframework/fscore/FsWorker.h>
#ifndef _WIN32
# include <syslog.h>
#endif

#include <dframework/util/NamedObject.h>
#include <dframework/util/NamedValue.h>

namespace dframework {

    FsRequestQueue::FsRequestQueue() {
    }

    FsRequestQueue::~FsRequestQueue() {
    }

    sp<Retval> FsRequestQueue::push(sp<FuseRequest>& req) {
        sp<Retval> retval;
        {
            AutoLock _l(this);
            if( DFW_RET(retval, m_queue.push(req)) )
                return DFW_RETVAL_D(retval);
        }
        broadcast();
        return NULL;
    }

    sp<FuseRequest> FsRequestQueue::pop(sp<FsWorker>& fly){
        sp<FuseRequest> req;
        while(!fly->isLanding()){
            {
                AutoLock _l(this);
                req = m_queue.pop();
            }
            if(req.has())
                return req;
            wait();
        }
        return NULL;
    }

    int FsRequestQueue::size(){
        AutoLock _l(this);
        return m_queue.size();
    }

    // --------------------------------------------------------------

    FsWorkerList::FsWorkerList(){
        m_max_spare = 5;
    }

    FsWorkerList::~FsWorkerList(){
    }

    int FsWorkerList::workerSize(){
        AutoLock _l(this);
        return m_aWorkers.size();
    }

    sp<Retval> FsWorkerList::insertWorker(sp<FsWorker>& worker){
        AutoLock _l(this);
        return m_aWorkers.insert(worker);
    }

    sp<FsWorker> FsWorkerList::removeWorker(sp<FsWorker>& worker){
        AutoLock _l(this);
        return m_aWorkers.remove(worker);
    }

    int FsWorkerList::wakeSize(){
        AutoLock _l(this);
        return m_aWakeWorkers.size();
    }

    sp<FsWorker> FsWorkerList::getWorker(int k){
        AutoLock _l(this);
        return m_aWorkers.get(k);
    }

    sp<Retval> FsWorkerList::insertWake(sp<FsWorker>& worker){
        AutoLock _l(this);
        return m_aWakeWorkers.insert(worker);
    }

    sp<FsWorker> FsWorkerList::removeWake(sp<FsWorker>& worker){
        AutoLock _l(this);
        return m_aWakeWorkers.remove(worker);
    }

    void FsWorkerList::setMaxSpare(int max){
        AutoLock _l(this);
        m_max_spare = max;
    }

    int FsWorkerList::getMaxSpare(){
        AutoLock _l(this);
        return m_max_spare;
    }

    int FsWorkerList::queueSize(){
        AutoLock _l(this);
        return (m_aWorkers.size() - m_aWakeWorkers.size());
    }

    bool FsWorkerList::hasQueue(){
        AutoLock _l(this);
        int qsize = m_aWorkers.size() - m_aWakeWorkers.size();
        if(qsize > 0)
            return true;
        return false;
    }

    bool FsWorkerList::hasMaxQueue(){
        AutoLock _l(this);
        int qsize = m_aWorkers.size() - m_aWakeWorkers.size();
        if(qsize > getMaxSpare())
            return true;
        return false;
    }


    // --------------------------------------------------------------

    FsWorker::FsWorker(){
        m_mode = DFW_FS_MODE_INIT;
    }

    FsWorker::~FsWorker(){
    }

    bool FsWorker::isLanding(){
        return m_bLanding;
    }

    sp<Retval> FsWorker::start_worker(){
        sp<Retval> retval;

        if( !m_wl->hasQueue() ){
            sp<FsWorker> fly = newWorker();

            //fly->setMount(); // unused
            fly->setRequestList(m_reqlist);
            //fly->setNodeList(); // unused
            fly->setRequestQueue(m_rq);
            fly->setWorkerList(m_wl);

            fly->m_mode = DFW_FS_MODE_WORKER;

            if( DFW_RET(retval, m_wl->insertWorker(fly)) )
                return DFW_RETVAL_D(retval);

            if(DFW_RET(retval, fly->start()) ){
                m_wl->removeWorker(fly);
                return DFW_RETVAL_D(retval);
            }
        }

        return NULL;
    }

    void FsWorker::onStoped(){
        sp<FsWorker> thiz = this;
        if(DFW_FS_MODE_WORKER == m_mode)
            m_wl->removeWorker(thiz);
    }

    sp<Retval> FsWorker::start(){
        sp<Retval> retval;
        sp<FsWorker> thiz = this;

        if( DFW_FS_MODE_MAIN == m_mode )
            return DFW_RET_C(retval, FuseWorker::start());

        m_bLanding = false;

        if( DFW_RET(retval, Thread::start()) ){
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> FsWorker::start(const char* mountpoint){
        sp<Retval> retval;

        m_bLanding = false;
        m_mode = DFW_FS_MODE_MAIN;

        if( !m_mount.has() )   m_mount = new FuseMount();
        if( !m_reqlist.has() ) m_reqlist = new FuseRequestList();
        if( !m_safe_nl.has() ) m_safe_nl = new FsNodeList();
        if( !m_rq.has() )      m_rq = new FsRequestQueue();
        if( !m_wl.has() )      m_wl = new FsWorkerList();

        if( DFW_RET(retval, FuseWorker::start(mountpoint)) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    void FsWorker::run(){
        sp<Retval> retval;

        switch(m_mode){
        case DFW_FS_MODE_MAIN :
            FuseWorker::run();
            return;
        case DFW_FS_MODE_WORKER :
            while(!isLanding()){
                sp<FsWorker> thiz = this;
                sp<FuseRequest> req = m_rq->pop(thiz);
                if(req.has()){
                    if( DFW_RET(retval, process_worker(req)) )
                        DFW_RETVAL_D(retval);
                    on_logging(req, retval, FuseWorker::LOG_LEVEL_V);
                }
            }
            return;
        }
    }

    sp<Retval> FsWorker::process(sp<FuseRequest>& req){
        sp<Retval> retval;

        if( DFW_FS_MODE_MAIN != m_mode ){
            retval = DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                , "This is not main FsWorker.");
            on_logging(req, retval, FuseWorker::LOG_LEVEL_E);
            return retval;
        }

        int retry = 0;
        do{
            retry++;
            if( DFW_RET(retval, m_rq->push(req)) ){
                if(retry>50){
                    retval = DFW_RETVAL_NEW_MSG(DFW_ERROR,0
                        , "retry=%d, Don't push request to queue.");
                    on_logging(req, retval, FuseWorker::LOG_LEVEL_E);
                    return retval;
                }
                usleep(1000*20);
                continue;
            }

            if( DFW_RET(retval, start_worker()) )
                on_logging(req, retval, FuseWorker::LOG_LEVEL_E);

            m_rq->broadcast();

            return NULL;
        }while(true);
    }

    sp<Retval> FsWorker::process_worker(sp<FuseRequest>& req){
        sp<Retval> retval;
        sp<FsWorker> thiz = this;
        sp<FuseMount> null_mount;

        m_wl->insertWake(thiz);

        if( DFW_RET(retval, start_worker()) ){
            DFW_RETVAL_D(retval);
            on_logging(req, retval, FuseWorker::LOG_LEVEL_E);
        }

        if( DFW_RET(retval, FuseWorker::process(req)) )
            DFW_RETVAL_D(retval);

        m_wl->removeWake(thiz);

        if( m_wl->hasMaxQueue() ){
            m_bLanding = true;
            m_wl->removeWorker(thiz);
        }

        if(retval.has())
            return DFW_RETVAL_D(retval);
        return NULL;
    }

}; // end namespace dframework
#endif

