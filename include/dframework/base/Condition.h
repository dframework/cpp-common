#ifndef DFRAMEWORK_BASE_CONDITION_H
#define DFRAMEWORK_BASE_CONDITION_H
#define DFRAMEWORK_BASE_CONDITION_VERSION 1

#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
#else
#include <pthread.h>
#endif

#include <dframework/base/Retval.h>


#ifdef __cplusplus
namespace dframework {
    
    class Condition
    {
    private:
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        HANDLE ___m_cond_handle;
#else
        pthread_cond_t  ___m_cond_handle;
        pthread_mutex_t ___m_cond_mutex_handle;
        dfw_retval_t    ___m_cond_errno;
        dfw_retval_t    ___m_cond_mutex_errno;
#endif
        Object          ___m_safe;
        bool            ___m_b_wait;
        dfw_retval_t    ___init_mutex();
        dfw_retval_t    ___init_cond();
        sp<Retval>      ___check_init();
        sp<Retval>      ___wait_real();
        sp<Retval>      ___timedwait_real(long milliSeconds);

    public:
        Condition();
        virtual ~Condition();

    public:
        bool       iswait();
        sp<Retval> wait();
        sp<Retval> timedwait(long milliSeconds);
        sp<Retval> wakeup();
        sp<Retval> broadcast();
    };

    
}; // end namespace dframework
#endif /* ifdef __cplusplus */


#endif /* DFRAMEWORK_BASE_CONDITION_H */

