#ifndef DFRAMEWORK_UTIL_MICROTIMEDEPTH_H
#define DFRAMEWORK_UTIL_MICROTIMEDEPTH_H


#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <sys/time.h>

#ifdef __cplusplus
namespace dframework {


    class MicroTimeDepth
    {
    private:
        struct timeval m_Dst;
        struct timeval m_Src;
    public:
        MicroTimeDepth();
        virtual ~MicroTimeDepth();

        virtual void clear();

        sp<Retval> start();
        sp<Retval> stop();
        sp<Retval> stop(struct timeval *result);

        long sec();
        long usec();

        String format(const char *fmt);
    };


};
#endif

#endif /* DFRAMEWORK_UTIL_MICROTIMEDEPTH_H */

