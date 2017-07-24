#include <dframework/base/Thread.h>
#include <dframework/base/ThreadManager.h>
#include <dframework/util/Time.h>
#include <signal.h>

namespace dframework {

#if 0
class testabcd{
public:
	testabcd() { printf("c abcd\n"); }
	virtual ~testabcd() { printf("d abcd\n"); }
};
#endif

    BaseThread::BaseThread(Thread* thread)
            : Object()
    {
        m_thread = thread;
        m_status = DFW_THREAD_STATUS_INIT;
        m_bRunnable = false;
        m_bDetach = false;
        m_bCancle = false;
        m_lastSigno = 0;
        m_bStopped = false;

        ::pthread_attr_init(&m_attr);

        //::sigemptyset(&m_newmask); // FIXME
        ::memset(&m_act, 0, sizeof(m_act));
        m_act.sa_handler = BaseThread::___sig_handler;

        Time::currentTimeMillis(&m_startTime);
        Time::currentTimeMillis(&m_lastTime);
    }

    BaseThread::~BaseThread(){
    }

    sp<Retval> BaseThread::start_check(){
        sp<Retval> retval;
        if( DFW_THREAD_STATUS_INIT != m_status
            && DFW_THREAD_STATUS_EXIT != m_status){
            return DFW_RETVAL_NEW_MSG(DFW_E_THREAD_STATUS, 0
                      ,"this=%p, status=%d, Wrong status."
                      , m_thread, m_status);
        }
        return NULL;
    }

    sp<Retval> BaseThread::start(){
        sp<Retval> retval;

        if( DFW_RET(retval, start_check()) )
            return DFW_RETVAL_D(retval);

        if( DFW_RET(retval, start_real()) )
            return DFW_RETVAL_D(retval);
        return retval;
    }

    sp<Retval> BaseThread::start_real(){
        sp<Retval> retval;
        m_bRunnable = false;
        m_bCancle = false;
        m_lastSigno = 0;
        m_status = DFW_THREAD_STATUS_READY;
        int count = 0;
        int eno;
        bool bRunnable = false;
        int rcount = 0;

        Time::currentTimeMillis(&m_startTime);
        Time::currentTimeMillis(&m_lastTime);

        if( DFW_RET(retval, m_thread->onPrepare()) )
            return DFW_RETVAL_D(retval);

        do{
            if( (eno = ::pthread_create(&m_handle, &m_attr
                              , BaseThread::___run, (void*)this) ) < 0 )
            {
                m_status = DFW_THREAD_STATUS_INIT;
                if(eno==EAGAIN){
                    if(count>10){
                        return DFW_RETVAL_NEW_MSG(DFW_E_AGAIN, eno
                                  , "pthread_create result is EAGAIN.");
                    }
                    count++;
                    usleep(1000);
                    continue;
                }
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, eno
                          , "pthread_create result is error(%d).",eno);
            }

//FIXME
#if 1
            do{
                {
                    AutoLock _l(this);
                    bRunnable = m_bRunnable;
                }

                if(bRunnable) return NULL;

                rcount++;
                if(rcount>900){
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                       , "thread is not start. maybe max running thread");
                }
                usleep(1000);
            } while(true);
#endif
            return NULL;
        }while(true);
    }

    void* BaseThread::___run(void *obj)
    {
        BaseThread *p = (BaseThread*)obj;
        sp<Thread> i_thread = p->m_thread;

        {
            AutoLock _l(p);
            p->m_bRunnable = true;
        }

        sp<ThreadManager> thdmana = ThreadManager::instance();

        {
            AutoLock _l(p);
            thdmana->insert(i_thread);
        }

        p->run();

        {
            AutoLock _l(p);
            thdmana->remove(p->handle());
            p->m_status = DFW_THREAD_STATUS_EXIT;
        }

        bool candetach = false;
        {
            AutoLock _l(p);
            if(!p->m_bDetach && !p->m_bJoin)
                candetach = true;
        }

        if(candetach){
            sp<Retval> retval = p->detach();
            if(retval.has()){
            }
        }

        {
            AutoLock _l(p);
            if(p->m_bJoin){
                pthread_exit(NULL);
            }
        }

        return NULL;
    }

    void BaseThread::run()
    {
        sp<Retval> retval;
        Time::currentTimeMillis(&m_startTime);
        Time::currentTimeMillis(&m_lastTime);

        if( DFW_RET(retval, m_thread->onStart()) )
            return;

        m_status = DFW_THREAD_STATUS_RUN;

        m_thread->run();

        m_status = DFW_THREAD_STATUS_STOP;

        m_thread->onStoped();
    }

    void BaseThread::___sig_handler(int signo){
        sp<Thread> thread;
        sp<ThreadManager> mana = ThreadManager::instance();
        int retry = 0;
        do{
            thread = mana->getThread(::pthread_self());
            if(!thread.has()){
                usleep(1000);
                retry++;
                if(retry>900){
                    return;
                }
                continue;
            }
            break;
        }while(true);
        thread->setLastSigno(signo);
        thread->onSignal(signo);
    }

    sp<Retval> BaseThread::setSigno(int signo){
        sp<Retval> retval;
        if( -1 == ::sigaction(signo, &m_act, NULL) ){
            int eno = errno;
            switch(eno){
            case EFAULT: return DFW_RETVAL_NEW(DFW_E_FAULT, eno);
            case EINVAL: return DFW_RETVAL_NEW(DFW_E_INVAL, eno);
            }
            return DFW_RETVAL_NEW(DFW_ERROR, eno);
        }
        return NULL;
    }

    void BaseThread::stop(){
        AutoLock _l(this);
        m_bStopped = true;
    }

    sp<Retval> BaseThread::join(){
        sp<Retval> retval;
        int eno;
        void* out_join;
        {
            AutoLock _l(this);
            m_bJoin = true;
        }

        if( (eno = ::pthread_join(m_handle, &out_join)) ){
            {
                AutoLock _l(this);
                m_bJoin = false;
            }
            return DFW_RETVAL_NEW(DFW_ERROR, eno);
        }
        return NULL;
    }

    sp<Retval> BaseThread::detach(){
        AutoLock _l(this);
        int eno;
        sp<Retval> retval;
        m_bDetach = true;
        if( (eno = ::pthread_detach(m_handle)) ){
            dfw_retno_t retno;
            switch(eno){
            case EINVAL: retno = DFW_E_INVAL; break;
            case ESRCH: retno = DFW_E_SRCH; break;
            default : retno = DFW_ERROR; break;
            }
            return DFW_RETVAL_NEW(retno, eno);
        }
        return NULL;
    }

    sp<Retval> BaseThread::cancel(){
        AutoLock _l(this);
#ifdef __ANDROID__
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not supported thread cancel on android");
#else
        sp<Retval> retval;
        int eno;
        m_bCancle = true;
        if( (eno = ::pthread_cancel(m_handle)) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, eno
               , "handle=%d, errno=%d, Not cancel thread", m_handle, eno);
        }
        return NULL;
#endif
    }

    bool BaseThread::isstop(){
        AutoLock _l(this);
        return m_bStopped;
    }

    bool BaseThread::isjoin() {
        AutoLock _l(this);
        return m_bJoin;
    }

    bool BaseThread::isdetach(){
        AutoLock _l(this);
        return m_bDetach;
    }

    bool BaseThread::iscancel() {
        AutoLock _l(this);
        return m_bCancle;
    }
   
    dfw_thread_status_t BaseThread::status(){
        AutoLock _l(this);
        return m_status;
    } 

    pthread_t BaseThread::handle(){
        AutoLock _l(this);
        return m_handle;
    }

    sp<Retval> BaseThread::kill(int signum){
        int eno = ::pthread_kill(m_handle, signum);
        if(eno){
            switch(eno){
            case EINVAL : return DFW_RETVAL_NEW(DFW_E_INVAL, eno);
            case ESRCH  : return DFW_RETVAL_NEW(DFW_E_SRCH, eno);
            }
            return DFW_RETVAL_NEW(DFW_ERROR, eno);
        }
        return NULL;
    }
 
    void BaseThread::getStartTime(dfw_timeval *out){
        AutoLock _l(this);
        if(out)
            ::memcpy(out, &m_startTime, sizeof(dfw_timeval));
    }

    void BaseThread::setLastTime(){
        AutoLock _l(this);
        Time::currentTimeMillis(&m_lastTime);
    } 
 
    void BaseThread::getLastTime(dfw_timeval *out){
        AutoLock _l(this);
        if(out)
            ::memcpy(out, &m_lastTime, sizeof(dfw_timeval));
    } 

    // --------------------------------------------------------------

    Thread::Trace::Trace()
    {
        m_pFile = NULL;
        m_pFunc = NULL;
        m_iLine = 0;
        m_pMessage = NULL;
        m_pPrev = NULL;
        m_pNext = NULL;

        ::memset(&m_enter, 0, sizeof(dfw_timeval));
        ::memset(&m_leave, 0, sizeof(dfw_timeval));
    }

    Thread::Trace::Trace(const char* file, const char* func
                       , int line, const char* msg) :
            Object()
    {
        m_iLine = line;
        m_pPrev = NULL;
        m_pNext = NULL;

        alloc(&m_pFile, file);
        alloc(&m_pFunc, func);
        alloc(&m_pMessage, msg);

        ::memset(&m_enter, 0, sizeof(dfw_timeval));
        ::memset(&m_leave, 0, sizeof(dfw_timeval));
        Time::currentTimeMillis(&m_enter);
    }

    Thread::Trace::~Trace(){
        DFW_FREE(m_pFile);
        DFW_FREE(m_pFunc);
        DFW_FREE(m_pMessage);
    }

    void Thread::Trace::leave(){
        Time::currentTimeMillis(&m_leave);
    }

    void Thread::Trace::alloc(char** d, const char* s){
        if(!s){ *d = NULL; return; }
        int size = strlen(s);
        *d = (char*) ::malloc( size + 1 );
        if( *d ){
            char* t = *d;
            memcpy( t, s, size );
            t[size] = '\0';
        }
    }

    // --------------------------------------------------------------

    DFW_DECLARATION_SAFE_COUNT(Thread);

    Thread::Thread() :
             ___m_base(this)
    {
        DFW_SAFE_ADD(Thread, l);
        m_bStoped = false;
    }

    Thread::~Thread() {
        DFW_SAFE_REMOVE(Thread, l);
    }

    String Thread::getTrace(){
        String v;
        sp<Thread::Trace> next = ___m_pTrace;
        while(next.has()){
            v.appendFmt("  `-- %s (%ld:%ld~%ld:%ld) in %s(..) at %s:%d\n"
                , next->m_pMessage
                , next->m_enter.sec, next->m_enter.usec
                , next->m_leave.sec, next->m_leave.usec
                , next->m_pFunc, next->m_pFile, next->m_iLine);
            next = next->m_pNext;
        }
        return v;
    }

    void Thread::addTrace(sp<Thread::Trace>& trace){
        AutoLock _l(this);
        setLastTime();
        if(trace.has()){
            if(___m_pLeaveTrace.has()){
                if( !___m_pLeaveTrace->m_pPrev.has() ){
                    ___m_pTrace = ___m_pLastTrace = NULL;
                }else{
                    ___m_pLeaveTrace->m_pPrev->m_pNext = NULL;
                    ___m_pLastTrace = ___m_pLeaveTrace->m_pPrev;
                }
            }
            ___m_pLeaveTrace = NULL;

            if(!___m_pLastTrace.has()){
                ___m_pTrace = ___m_pLastTrace = trace;
            }else{
                ___m_pLastTrace->m_pNext = trace;
                trace->m_pPrev = ___m_pLastTrace;
                ___m_pLastTrace = trace;
            }
        }
    }

    void Thread::removeTrace(sp<Thread::Trace>& trace){
        AutoLock _l(this);
        setLastTime();
        if(!trace.has() || !___m_pTrace.has()) {
            printf("error remove trace\n");
            return;
        }
        trace->leave();
        ___m_pLeaveTrace = trace;
    }

}; // end namespace dframework

