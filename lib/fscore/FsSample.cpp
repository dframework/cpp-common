#ifndef _WIN32
#include <dframework/fscore/FsSample.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

namespace dframework {

    FsSampleList::FsSampleList(){
        m_max_spare = 5;
    }

    FsSampleList::~FsSampleList(){
    }

    int FsSampleList::workerSize(){
        AutoLock _l(this);
        return m_aWorkers.size();
    }

    sp<Retval> FsSampleList::insertWorker(sp<FsSample>& worker){
        AutoLock _l(this);
        return m_aWorkers.insert(worker);
    }

    sp<FsSample> FsSampleList::removeWorker(sp<FsSample>& worker){
        AutoLock _l(this);
        return m_aWorkers.remove(worker);
    }

    int FsSampleList::wakeSize(){
        AutoLock _l(this);
        return m_aWakeWorkers.size();
    }

    sp<Retval> FsSampleList::insertWake(sp<FsSample>& worker){
        AutoLock _l(this);
        return m_aWakeWorkers.insert(worker);
    }

    sp<FsSample> FsSampleList::removeWake(sp<FsSample>& worker){
        AutoLock _l(this);
        return m_aWakeWorkers.remove(worker);
    }

    void FsSampleList::setMaxSpare(int max){
        AutoLock _l(this);
        m_max_spare = max;
    }

    int FsSampleList::getMaxSpare(){
        AutoLock _l(this);
        return m_max_spare;
    }

    int FsSampleList::queueSize(){
        AutoLock _l(this);
        return (m_aWorkers.size() - m_aWakeWorkers.size());
    }

    bool FsSampleList::hasQueue(){
        AutoLock _l(this);
        int qsize = m_aWorkers.size() - m_aWakeWorkers.size();
        if(qsize > 0)
            return true;
        return false;
    }

    bool FsSampleList::hasMaxQueue(){
        AutoLock _l(this);
        int qsize = m_aWorkers.size() - m_aWakeWorkers.size();
        if(qsize >= getMaxSpare())
            return true;
        return false;
    }

    void FsSampleList::joinAll(){
        while(true){
            if( workerSize() == 0 ){
                return;
            }
            sleep(1);
        }
    }

    // --------------------------------------------------------------

    FsSample::FsSample(){
    }

    FsSample::~FsSample(){
    }

    void FsSample::onStoped(){
        sp<FsSample> thiz = this;
        m_wl->removeWorker(thiz);
    }

    sp<Retval> FsSample::start(){
        sp<Retval> retval;
        return DFW_RET_C(retval, FuseWorker::start());
    }

    sp<Retval> FsSample::start(const char* mountpoint){
        sp<Retval> retval;
        sp<FsSample> thiz = this;

        if( !m_mount.has() )   m_mount = new FuseMount();
        if( !m_reqlist.has() ) m_reqlist = new FuseRequestList();
        if( !m_safe_nl.has() ) m_safe_nl = new FsNodeList();
        if( !m_wl.has() )      m_wl = new FsSampleList();

        if( DFW_RET(retval, m_wl->insertWorker(thiz)) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, FuseWorker::start(mountpoint)) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    sp<Retval> FsSample::process(sp<FuseRequest>& req){
        sp<Retval> retval;
        sp<FsSample> thiz = this;

        m_wl->insertWake(thiz);

        bool isnewworker = false;
        {
            AutoLock _l(m_wl.get());
            if( !m_wl->hasQueue() ){
                isnewworker = true;
            }
        }

        if( isnewworker ){
            sp<FsSample> nworker = newWorker();
            nworker->setMount(m_mount);
            nworker->setNodeList(m_safe_nl);
            nworker->setWorkerList(m_wl);
            nworker->setRequestList(m_reqlist);

            if( DFW_RET(retval, m_wl->insertWorker(nworker)) ){
                // FIXME
                printf("%s\n", retval->dump().toChars());
            }else if( DFW_RET(retval, nworker->start()) ){
                // FIXME
                printf("%s\n", retval->dump().toChars());
                m_wl->removeWorker(nworker);
            }
        }

        retval = FuseWorker::process(req);

        m_wl->removeWake(thiz);

        {
            AutoLock _l(m_wl.get());
            if( m_wl->hasMaxQueue() ){
                m_bExited = true;
                m_wl->removeWorker(thiz);
            }
        }

        return retval;
    }

}; // end namespace dframework
#endif

