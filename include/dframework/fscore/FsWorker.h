#ifndef DFRAMEWORK_FSCORE_FSWORKER_H
#define DFRAMEWORK_FSCORE_FSWORKER_H

#include <dframework/fscore/FuseWorker.h>
#include <dframework/base/Condition.h>
#include <dframework/util/Queue.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FsWorker;

    class FsRequestQueue : public Object, public Condition
    {
    private:
        Queue<FuseRequest> m_queue;

    public:
        FsRequestQueue();
        virtual ~FsRequestQueue();

        sp<Retval> push(sp<FuseRequest>& req);
        sp<FuseRequest> pop(sp<FsWorker>& fly);
        int size();
    };

    // --------------------------------------------------------------

    class FsWorkerList : public Object
    {
    private:
        int                  m_max_spare;
        ArraySorted<FsWorker> m_aWorkers;
        ArraySorted<FsWorker> m_aWakeWorkers;

    public:
        FsWorkerList();
        virtual ~FsWorkerList();

        int workerSize();
        sp<Retval> insertWorker(sp<FsWorker>& worker);
        sp<FsWorker> removeWorker(sp<FsWorker>& worker);
        sp<FsWorker> getWorker(int position);

        int wakeSize();
        sp<Retval> insertWake(sp<FsWorker>& worker);
        sp<FsWorker> removeWake(sp<FsWorker>& worker);

        void setMaxSpare(int max);
        int getMaxSpare();

        int queueSize();
        bool hasQueue();
        bool hasMaxQueue();

        void joinAll();
    };

    // --------------------------------------------------------------

    class FsWorker : public FuseWorker
    {
    public:
        static const int DFW_FS_MODE_INIT   = 0;
        static const int DFW_FS_MODE_MAIN   = 1;
        static const int DFW_FS_MODE_WORKER = 2;

    protected:
        int                m_mode;
        sp<FsRequestQueue> m_rq;
        sp<FsWorkerList>   m_wl;
        bool               m_bLanding;

    public:
        FsWorker();
        virtual ~FsWorker();

        inline void setRequestQueue(sp<FsRequestQueue>& q) { m_rq = q; }
        inline void setWorkerList(sp<FsWorkerList>& w) { m_wl = w; }
        inline const char* getMountpoint() const {
            return m_context->getMountpoint(); 
        }
        inline virtual sp<FsWorker> newWorker(){ return new FsWorker(); }

        bool isLanding();
        inline void setLanding(bool b) { m_bLanding = b; }

        virtual void onStoped();
        virtual sp<Retval> start();
        virtual sp<Retval> start(const char* mountpoint);
        virtual void run();

        sp<Retval> start_worker();

        sp<Retval> process(sp<FuseRequest>& req);
        sp<Retval> process_main(sp<FuseRequest>& req);
        sp<Retval> process_worker(sp<FuseRequest>& req);

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FSWORKER_H */

