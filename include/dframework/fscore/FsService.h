#ifndef DFRAMEWORK_FSCORE_FSSERVICE_H
#define DFRAMEWORK_FSCORE_FSSERVICE_H

#include <dframework/fscore/FsWorker.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FsService : public FsWorker
    {
    private:
        ArraySorted<FsService> m_services;

    public:
        FsService();
        virtual ~FsService();

        sp<Retval> appendService(sp<FsContext>& c);
        sp<Retval> stopService();
        sp<Retval> startService();

        void joinAll();

        inline virtual sp<FsWorker> newWorker(){return new FsService();}
        inline int serviceSize() { return m_services.size(); }
        inline sp<FsService> getService(int position){
            return m_services.get(position);
        }

    protected:
        inline virtual sp<Retval> readyService(){ return NULL; }

    private:
        sp<Retval> appendService(sp<FsService>& out_worker
                               , sp<FsContext>& context);
        sp<Retval> createService(sp<FsService>& worker
                               , sp<FsContext>& context);
        void joinMainAll();
        void joinWorkerAll();

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FSSERVICE_H */

