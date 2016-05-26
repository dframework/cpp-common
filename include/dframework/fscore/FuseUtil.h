#ifndef DFRAMEWORK_FSCORE_FUSEUTIL_H
#define DFRAMEWORK_FSCORE_FUSEUTIL_H

#include <dframework/base/Retval.h>
#include <dframework/fscore/Fuse_d.h>
#include <sys/stat.h>


#if defined( __ANDROID__ )
# define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atime_nsec)
# define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctime_nsec)
# define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtime_nsec)
# define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atime_nsec = (val)
# define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtime_nsec = (val)
#elif defined(__APPLE__)
# define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atimensec)
# define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctimensec)
# define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtimensec)
# define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atimensec = (val)
# define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtimensec = (val)
#else
# define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atim.tv_nsec)
# define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctim.tv_nsec)
# define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtim.tv_nsec)
# define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atim.tv_nsec = (val)
# define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtim.tv_nsec = (val)
#endif


#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseUtil
    {
    public:
        static void statToFuse(
            struct fuse_attr *attr, const struct stat *stbuf);
#if 0
        static void unsigned long calcTimeoutSec(double t);
        static unsigned int calcTimeoutNsec(double t);
#endif
    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEUTIL_H */

