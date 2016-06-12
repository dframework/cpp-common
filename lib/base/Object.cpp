#include <dframework/base/Object.h>
#include <dframework/base/sp.h>
#include <pthread.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(Object);

    Object::Object() :
          ___m_ref_count(0)
    {
        DFW_SAFE_ADD(Object, l);

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

    Object::~Object()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        DeleteCriticalSection(&___m_handle);
#else
#  if defined(DFW_SAFE_RECURSIVE)
        ::pthread_mutexattr_destroy(&___m_attr);
#  endif
        if(!___m_status) ::pthread_mutex_destroy(&___m_handle);
#endif
        DFW_SAFE_REMOVE(Object, l);
    }

    int Object::___m_init(){
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
#else
        int status;
#  if defined(DFW_SAFE_RECURSIVE)
        if((status=::pthread_mutex_init(&___m_handle, &___m_attr)))
            return status;
#  else
        if((status=::pthread_mutex_init(&___m_handle, NULL)))
            return status;
#  endif
#endif
        return 0;
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

    int Object::lock()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        EnterCriticalSection(&___m_handle);
#else
        if(___m_status){
printf("lock status : %d\n", ___m_status); exit(1);
            //if( (___m_status = ___m_init()) ) return ___m_status;
            return ___m_status;
        }
#  if defined(DFW_SAFE_RECURSIVE)
        int eno = ::pthread_mutex_lock(&___m_handle);
        if(eno) {
//printf("lock eno : %d\n", eno);// exit(1);
		 return eno;
	}
#  else
        pthread_t tempid = ::pthread_self();
        if(tempid==___m_thread_id){
            ___m_count++;
        }else{
            int eno = ::pthread_mutex_lock(&___m_handle);
            if(eno) return eno;
            ___m_thread_id = tempid;
            ___m_count = 1;
        }
#  endif
#endif
        return 0;
    }
    
    int Object::unlock()
    {
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        LeaveCriticalSection(&___m_handle);
#else
        if(___m_status){
            //if( (___m_status = ___m_init()) ) return ___m_status;
            return ___m_status;
        }
#  if defined(DFW_SAFE_RECURSIVE)
        int eno = ::pthread_mutex_unlock(&___m_handle);
        if(eno) return eno;
#  else
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
#  endif
#endif
        return 0;
    }

}; // end namespace dframework

