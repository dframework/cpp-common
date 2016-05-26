#ifndef DFRAMEWORK_BASE_THREADMANAGER_H
#define DFRAMEWORK_BASE_THREADMANAGER_H

#include <dframework/base/Condition.h>
#include <dframework/base/Object.h>
#include <dframework/base/Thread.h>
#include <dframework/util/ArraySorted.h>

#define DFW_TRACE(var,enable,fmt,args...) \
    AutoTrace var(enable,__FILE__,__func__,__LINE__,fmt,##args)

#ifdef __cplusplus
namespace dframework {

    class ThreadManager : public Object
    {
    private:
        class Node : public Object {
        public:
            sp<Thread> m_thread;
            pthread_t  m_handle;

            Node(sp<Thread>& thread);
            Node(pthread_t handle);
            inline virtual ~Node() {}

            DFW_OPERATOR_EX_DECLARATION(Node, m_handle);
        };

        static Object            m_safe;
        static sp<ThreadManager> m_manager;
        ArraySorted<Node> m_aList;
        int  m_refcount;
        bool m_iscancel;

        ThreadManager();

    public:
        virtual ~ThreadManager();
        static sp<ThreadManager>& instance();

        int refcount();

        int size();
        sp<Thread> get(int position);
        sp<Thread> getThread(pthread_t id);

        sp<Retval> joinAll();
        sp<Retval> cancelAll();

    private:
        sp<Retval> insert(sp<Thread>& running_thread);
        sp<Thread> remove(pthread_t stoping_thread_id);

        friend class BaseThread;
    };

    class AutoTrace {
    private:
        sp<Thread::Trace> m_trace;
        sp<Thread>        m_thread;
    public:
        AutoTrace(bool enable, const char* fl
                , const char* fn, int l, const char* f, ...);
        virtual ~AutoTrace();
    };

}; // end namespace dframework
#endif // end if cplusplus

#endif /* DFRAMEWORK_BASE_THREADMANAGER_H */

