#include <dframework/base/Object.h>
#include <dframework/base/sp.h>
#include <pthread.h>

namespace dframework {

    // -------------------------------------------------------------
    //
    //    S a f e _ K
    //
    // -------------------------------------------------------------

    Safe_K::Safe_K()
    {
        int error;
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        InitializeCriticalSection( &___m_handle );
#else
#  if defined(DFW_SAFE_RECURSIVE)
        if( (error = ::pthread_mutexattr_init(&___m_attr)) ){
            printf("error: error=%d,  %s:%d\n", error, __FILE__,__LINE__);
        }
        if( (error = ::pthread_mutexattr_settype(&___m_attr, PTHREAD_MUTEX_RECURSIVE)) ){
            printf("error: error=%d,  %s:%d\n", error, __FILE__,__LINE__);
        }
#  endif
        ___m_status = ___m_init();
#endif
    }

    Safe_K::~Safe_K()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        DeleteCriticalSection(&___m_handle);
#else
#  if defined(DFW_SAFE_RECURSIVE)
        ::pthread_mutexattr_destroy(&___m_attr);
#  endif
        if(!___m_status) ::pthread_mutex_destroy(&___m_handle);
#endif
    }

    int Safe_K::___m_init(){
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
#elif defined(DFW_SAFE_RECURSIVE)
        int status;
        if((status=::pthread_mutex_init(&___m_handle, &___m_attr)))
            return status;
#else
        int status;
        if((status=::pthread_mutex_init(&___m_handle, NULL)))
            return status;
#endif
        return 0;
    }

    int Safe_K::lock()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        EnterCriticalSection(&___m_handle);
        return 0;
#elif defined(DFW_SAFE_RECURSIVE)
        return ::pthread_mutex_lock(&___m_handle);
#else
        pthread_t tempid = ::pthread_self();
        if(tempid==___m_thread_id){
            ___m_count++;
        }else{
            int eno = ::pthread_mutex_lock(&___m_handle);
            if(eno) return eno;
            ___m_thread_id = tempid;
            ___m_count = 1;
        }
        return 0;
#endif
    }
    
    int Safe_K::unlock()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        LeaveCriticalSection(&___m_handle);
        return 0;
#elif defined(DFW_SAFE_RECURSIVE)
        return ::pthread_mutex_unlock(&___m_handle);
#else
        pthread_t tempid = ::pthread_self();
        if(tempid!=___m_thread_id)
            return 0;
        
        ___m_count--;
        if(___m_count<=0){
            ___m_count = 0;
            ___m_thread_id = 0;
            int eno = ::pthread_mutex_unlock(&___m_handle);
            if(eno) return eno;
        }
        return 0;
#endif
    }

    // -------------------------------------------------------------
    //
    //    S a f e
    //
    // -------------------------------------------------------------

    Safe_K Safe::___static_safe_k;

    Safe::Safe(){
        ___safe_k = NULL;
    }

    Safe::~Safe(){
        DFW_DELETE(___safe_k);
    }

    int Safe::lock(){
        ___static_safe_k.lock();
        if( !___safe_k )
            ___safe_k = new Safe_K();
        ___static_safe_k.unlock();
        return ___safe_k->lock();
    }

    int Safe::unlock(){
        return (___safe_k ? ___safe_k->unlock() : 0);
    }

    // -------------------------------------------------------------
    //
    //    O b j e c t
    //
    // -------------------------------------------------------------

    DFW_DECLARATION_SAFE_COUNT(Object);

    Object::Object() :
          ___m_ref_count(0)
    {
        DFW_SAFE_ADD(Object, l);
    }

    Object::~Object()
    {
        DFW_SAFE_REMOVE(Object, l);
    }

    int Object::incRefCount()
    {
        AutoLock _l(this);
        ___m_ref_count++;
        DFW_ABORT(___m_ref_count<0);
        return ___m_ref_count;
    }

    int Object::decRefCount()
    {
        AutoLock _l(this);
        ___m_ref_count--;
        DFW_ABORT(___m_ref_count<0);
        return ___m_ref_count;
    }

}; // end namespace dframework

