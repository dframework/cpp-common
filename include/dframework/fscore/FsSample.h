#ifndef DFRAMEWORK_FSCORE_FSSAMPLE_H
#define DFRAMEWORK_FSCORE_FSSAMPLE_H

#include <dframework/fscore/FuseWorker.h>
#include <dframework/util/ArraySorted.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FsSample;

    class FsSampleList : public Object
    {
    private:
        int                   m_max_spare;
        ArraySorted<FsSample> m_aWorkers;
        ArraySorted<FsSample> m_aWakeWorkers;

    public:
        FsSampleList();
        virtual ~FsSampleList();

        int workerSize();
        sp<Retval> insertWorker(sp<FsSample>& worker);
        sp<FsSample> removeWorker(sp<FsSample>& worker);

        int wakeSize();
        sp<Retval> insertWake(sp<FsSample>& worker);
        sp<FsSample> removeWake(sp<FsSample>& worker);

        void setMaxSpare(int max);
        int getMaxSpare();

        int queueSize();
        bool hasQueue();
        bool hasMaxQueue();

        void joinAll();
    };

    // --------------------------------------------------------------

    class FsSample : public FuseWorker
    {
    protected:
      sp<FsSampleList>   m_wl;

      virtual sp<Retval> process(sp<FuseRequest>& req);

    public:
      FsSample();
      virtual ~FsSample();

      virtual void setWorkerList(sp<FsSampleList>& l) { m_wl = l; }

      virtual sp<Retval> start();
      virtual sp<Retval> start(const char* mountpoint);

      virtual void onStoped();

      inline virtual sp<FsSample> newWorker(){ return new FsSample(); }
      inline void joinAll() { m_wl->joinAll(); }

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FSSAMPLE_H */

