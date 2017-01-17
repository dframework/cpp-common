#ifndef DFRAMEWORK_BASE_THREAD_H
#define DFRAMEWORK_BASE_THREAD_H

#include <dframework/base/Condition.h>
#include <dframework/base/Object.h>
#include <dframework/base/Sigaction.h>
#include <dframework/util/Time.h>
#include <signal.h>

enum dfw_thread_status_t
{
    DFW_THREAD_STATUS_INIT    = 0,
    DFW_THREAD_STATUS_READY   = 1,
    DFW_THREAD_STATUS_RUN     = 2,
    DFW_THREAD_STATUS_STOP    = 3,
    DFW_THREAD_STATUS_EXIT    = 4,
    DFW_THREAD_STATUS_DELETE  = 5,
};

#ifdef __cplusplus
namespace dframework {

    class Thread;

    class BaseThread : public Object, public Condition
    {
    private:
        friend class Thread;

        Thread*             m_thread;
        pthread_t           m_handle; /* thread_id */
        dfw_thread_status_t m_status;
        bool                m_bRunnable;
        bool                m_bCancle;
        bool                m_bJoin;
        bool                m_bDetach;
        dfw_timeval         m_startTime;
        dfw_timeval         m_lastTime;

        struct sigaction    m_act;
        //sigset_t            m_newmask; // FIXME
        int                 m_lastSigno;

    protected:
        bool                m_bStopped;

    public:
        pthread_attr_t      m_attr;
        //sigset_t            m_oldset; // FIXME

    private:
        BaseThread(Thread* thread);
    public:
        virtual ~BaseThread();

    private:
        sp<Retval> start_check();
        sp<Retval> start_real();
        sp<Retval> start();
        sp<Retval> join();

        static void* ___run(void *obj);
        void run();

        static void ___sig_handler(int signo);
        sp<Retval> setSigno(int signo);

        void       stop();
        bool       isstop();
        sp<Retval> detach();
        bool       isdetach();
        sp<Retval> cancel();
        bool       iscancel();
        bool       isjoin();
        dfw_thread_status_t status();
        pthread_t  handle();
        sp<Retval> kill(int signum);

        inline void setLastSigno(int signo){ m_lastSigno = signo; }
        inline int  getLastSigno(){ return m_lastSigno; }

        void       getStartTime(dfw_timeval *out);
        void       setLastTime();
        void       getLastTime(dfw_timeval *out);
    };

    class Thread : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(Thread);

        class  Trace : public Object
        {
        public:
            char*       m_pFile;
            char*       m_pFunc;
            int         m_iLine;
            char*       m_pMessage;
            dfw_timeval m_enter;
            dfw_timeval m_leave;
            sp<Trace>   m_pNext;
            sp<Trace>   m_pPrev;

            Trace();
            Trace(const char* file, const char* fn, int l, const char* m);
            void leave();
            virtual ~Trace();

        private:
            void alloc(char** d, const char* s);
        };

    private:
        BaseThread ___m_base;
        sp<Trace>  ___m_pTrace;
        sp<Trace>  ___m_pLastTrace;
        sp<Trace>  ___m_pLeaveTrace;

    protected:
        bool m_bStoped;         

    public:
        Thread();
        virtual ~Thread();

        void addTrace(sp<Trace>& stack);
        void removeTrace(sp<Trace>& stack);
        String getTrace();


        inline virtual void run()         {}
        inline virtual void cleanup()     {}

        inline virtual sp<Retval> onPrepare()   { return NULL; }
        inline virtual sp<Retval> onStart()     { return NULL; }
        inline virtual void onStoped()    {}
        inline virtual void onSignal(int signum) { DFW_UNUSED(signum); }

        inline virtual sp<Retval> start()    { return ___m_base.start(); }
        inline virtual void stop()           { ___m_base.stop(); }
        inline virtual bool isstop()         { ___m_base.isstop(); }
        inline virtual sp<Retval> join()     { return ___m_base.join(); }
        inline virtual bool       isjoin()   { return ___m_base.isjoin(); }
        inline virtual sp<Retval> detach()   { return ___m_base.detach(); }
        inline virtual bool       isdetach() { return ___m_base.isdetach(); }
        inline virtual sp<Retval> cancel()   { return ___m_base.cancel(); }
        inline virtual bool       iscancel() { return ___m_base.iscancel(); }
        inline virtual dfw_thread_status_t status(){
            return ___m_base.status(); 
        }
        inline virtual bool       iswait()   { return ___m_base.iswait(); }
        inline virtual sp<Retval> wait()     { return ___m_base.wait(); }
        inline virtual sp<Retval> timedwait(long msec){
            return ___m_base.timedwait(msec);
        }
        inline virtual sp<Retval> wakeup()   { return ___m_base.wakeup(); }
        inline virtual sp<Retval> broadcast(){ return ___m_base.broadcast(); }
        inline virtual pthread_t  handle()   { return ___m_base.handle(); }

        inline void setLastSigno(int signo){ 
            return ___m_base.setLastSigno(signo); 
        }
        inline int getLastSigno(){ return ___m_base.getLastSigno(); }
        inline sp<Retval> setSigno(int signo){
            return ___m_base.setSigno(signo);
        }
        inline virtual sp<Retval> kill(int signo) {
            return ___m_base.kill(signo);
        }
        //inline SigSet* getSigset(){ return &___m_base.m_Sigset; }

        inline void getStartTime(dfw_timeval *out){ 
            return ___m_base.getStartTime(out); 
        }
        inline void setLastTime() { ___m_base.setLastTime(); }
        inline void getLastTime(dfw_timeval *out) {
            return ___m_base.getLastTime(out); 
        }

    };

}; // end namespace dframework
#endif

#endif /* DFRAMEWORK_BASE_THREAD_H */

