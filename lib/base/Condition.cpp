#include <dframework/base/Condition.h>
#include <dframework/util/Time.h>
#ifdef DFW_OS_LINUX
#include <syslog.h>
#endif

namespace dframework {

    
    Condition::Condition()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        ___m_cond_handle = ::CreateEvent(NULL, false, false, NULL);
#else
        ___m_cond_mutex_errno = ___init_mutex();
        ___m_cond_errno = ___init_cond();
#endif
    }
     
    Condition::~Condition()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        if(___m_cond_handle)
            ::CloseHandle(___m_cond_handle);
#else
        if(!DFW_RETVAL_HASX(___m_cond_errno))
            ::pthread_cond_destroy(&___m_cond_handle);
        if(!DFW_RETVAL_HASX(___m_cond_mutex_errno))
            ::pthread_mutex_destroy(&___m_cond_mutex_handle);
#endif
    }

    dfw_retval_t Condition::___init_mutex(){
        dfw_errno_t eno;
        int count = 1;
        do{
            if( !(eno = ::pthread_mutex_init(&___m_cond_mutex_handle, NULL)) )
                return DFW_OK_LOCAL;
            if(eno==EAGAIN){
                if(count>10)
                    return {DFW_E_AGAIN, eno};
                usleep(1000);
                count++;
                continue;
            }
            switch(eno){
            case EINTR:
                return {DFW_E_INTR, eno};
            case EINVAL:
                return {DFW_E_INVAL, eno};
            case EDEADLK:
                return {DFW_E_DEADLK, eno};
            case EBUSY:
                return {DFW_E_BUSY, eno};
            case ETIMEDOUT:
                return {DFW_E_TIMEOUT, eno};
            case EPERM:
                return {DFW_E_PERM, eno};
            }
            return {Retval::retno(eno), eno};
        }while (true);
    }
    
    dfw_retval_t Condition::___init_cond(){
        dfw_errno_t eno;
        int count = 1;
        do{
            if( !(eno = ::pthread_cond_init(&___m_cond_handle, NULL)) )
                return DFW_OK_LOCAL;
            if(eno==EAGAIN){
                if(count>10)
                    return {DFW_E_AGAIN, eno};
                usleep(1000);
                count++;
                continue;
            }
            switch(eno){
            case EINTR:
                return {DFW_E_INTR, eno};
            case EINVAL:
                return {DFW_E_INVAL, eno};
            case EBUSY:
                return {DFW_E_BUSY, eno};
            case ENOMEM:
                return {DFW_E_NOMEM, eno};
            }
            return {Retval::retno(eno), eno};
        }while (true);
    }
    
    sp<Retval> Condition::___check_init(){
        if(DFW_RETVAL_HASX(___m_cond_mutex_errno)){
            ___m_cond_mutex_errno=___init_mutex();
            if(DFW_RETVAL_HASX(___m_cond_mutex_errno))
                return DFW_RETVAL_NEWX(___m_cond_mutex_errno);
        }

        if(DFW_RETVAL_HASX(___m_cond_errno)){
            ___m_cond_errno=___init_cond();
            if(DFW_RETVAL_HASX(___m_cond_errno))
                return DFW_RETVAL_NEWX(___m_cond_errno);
        }

        return NULL;
    }

    sp<Retval> Condition::___wait_real()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        WaitForSingleObject(___m_cond_handle, 0);
        return DFW_OK;
#else
        sp<Retval> retval = ___check_init();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        int eno;
        pthread_mutex_lock(&___m_cond_mutex_handle);
        if( !(eno = ::pthread_cond_wait(
                        &___m_cond_handle, &___m_cond_mutex_handle)) ){
            pthread_mutex_unlock(&___m_cond_mutex_handle);
            return NULL;
        }
        pthread_mutex_unlock(&___m_cond_mutex_handle);

        switch(eno){
        case ETIMEDOUT:
            return DFW_RETVAL_NEW(DFW_E_TIMEOUT,eno);
        case EINVAL:
            return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
        case EPERM:
            return DFW_RETVAL_NEW(DFW_E_PERM,eno);
        }
        return DFW_RETVAL_NEW(Retval::retno(eno), eno);
#endif
    }

    sp<Retval> Condition::___timedwait_real(long milliSeconds)
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        WaitForSingleObject(___m_cond_handle, milliSeconds);
        return NULL;
#else
        sp<Retval> retval = ___check_init();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        int eno;
        struct timespec ts;

#define DFW_NSEC 1000000000
        dfw_timeval tv;
        Time::currentTimeMillis(&tv);
        ts.tv_sec = tv.sec + (milliSeconds/1000);
        ts.tv_nsec = (tv.usec + ((milliSeconds%1000)*1000))*1000;

        if(ts.tv_nsec>DFW_NSEC){
            ts.tv_sec += (ts.tv_nsec/DFW_NSEC);
            ts.tv_nsec = (ts.tv_nsec%DFW_NSEC);
        }

        pthread_mutex_lock(&___m_cond_mutex_handle);
        if( !(eno = ::pthread_cond_timedwait(
                &___m_cond_handle, &___m_cond_mutex_handle, &ts)) ){
            pthread_mutex_unlock(&___m_cond_mutex_handle);
            return NULL;
        }
        pthread_mutex_unlock(&___m_cond_mutex_handle);

        switch(eno){
        case ETIMEDOUT:
            return DFW_RETVAL_NEW(DFW_E_TIMEOUT,eno);
        case EINVAL:
            return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
        case EPERM:
            return DFW_RETVAL_NEW(DFW_E_PERM,eno);
        }
        return DFW_RETVAL_NEW(Retval::retno(eno), eno);
#endif
    }

    sp<Retval> Condition::wait()
    {
        sp<Retval> retval;
        {
            AutoLock _l(&___m_safe);
            ___m_b_wait = true;
        }
        if( DFW_RET(retval, ___wait_real()) ){
            {
                AutoLock _l(&___m_safe);
                ___m_b_wait = false;
            }
            return DFW_RETVAL_D(retval);
        }
        {
            AutoLock _l(&___m_safe);
            ___m_b_wait = false;
        }
        return NULL;
    }

    sp<Retval> Condition::timedwait(long milliSeconds)
    {
        sp<Retval> retval;
        {
            AutoLock _l(&___m_safe);
            ___m_b_wait = true;
        }
        if( DFW_RET(retval, ___timedwait_real(milliSeconds)) ){
            {
                AutoLock _l(&___m_safe);
                ___m_b_wait = false;
            }
            return DFW_RETVAL_D(retval);
        }
        {
            AutoLock _l(&___m_safe);
            ___m_b_wait = false;
        }
        return NULL;
    }

    bool Condition::iswait()
    {
        AutoLock _l(&___m_safe);
        return ___m_b_wait;
    }

    sp<Retval> Condition::wakeup()
    {
        sp<Retval> retval = ___check_init();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        int eno;
        pthread_mutex_lock(&___m_cond_mutex_handle);
        if( !(eno = ::pthread_cond_signal(&___m_cond_handle)) ){
            pthread_mutex_unlock(&___m_cond_mutex_handle);
            return NULL;
        }
        pthread_mutex_unlock(&___m_cond_mutex_handle);

        switch(eno){
        case EINVAL:
            return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
        }
        return DFW_RETVAL_NEW(Retval::retno(eno), eno);
    }

    sp<Retval> Condition::broadcast()
    {
        sp<Retval> retval = ___check_init();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        int eno;
        pthread_mutex_lock(&___m_cond_mutex_handle);
        if( !(eno = ::pthread_cond_broadcast(&___m_cond_handle)) ){
            pthread_mutex_unlock(&___m_cond_mutex_handle);
            return NULL;
        }
        pthread_mutex_unlock(&___m_cond_mutex_handle);

        switch(eno){
        case EINVAL:
            return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
        }
        return DFW_RETVAL_NEW(Retval::retno(eno), eno);
    }

}; // end namespace dframework

