#ifndef DFRAMEWORK_LIB_HOSTMANAGER_H
#define DFRAMEWORK_LIB_HOSTMANAGER_H


#ifdef __cplusplus
#include <dframework/base/Retval.h>
#include <dframework/base/Thread.h>
#include <dframework/net/Hostname.h>

namespace dframework {

    class HostManager: public Object
    {
    public:
        class HostLookup : public Thread
        {
        private:
            unsigned m_refresh_seconds;

        private:
            HostLookup();
            
        public:
            virtual ~HostLookup();

            virtual sp<Retval> start();
            virtual sp<Retval> start(unsigned refresh);
            virtual void run();

            friend class HostManager;
        };

    public:
        static sp<Retval> start(unsigned refresh, unsigned timesec);
        static sp<Retval> stop();
        static sp<Retval> join();

    private:
        static Object         m_lock;
        static sp<HostLookup> m_hostLookup;

    };

};
#endif

#endif /* DFRAMEWORK_LIB_HOSTMANAGER_H */

