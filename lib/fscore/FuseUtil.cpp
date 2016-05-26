#ifndef _WIN32
#include <dframework/fscore/FuseUtil.h>

namespace dframework {

    void FuseUtil::statToFuse(
            struct fuse_attr *attr, const struct stat *stbuf)
    {
        attr->ino       = stbuf->st_ino;
        attr->mode      = stbuf->st_mode;
        attr->nlink     = stbuf->st_nlink;
        attr->uid       = stbuf->st_uid;
        attr->gid       = stbuf->st_gid;
        attr->rdev      = stbuf->st_rdev;
        attr->size      = stbuf->st_size;
        attr->blksize   = stbuf->st_blksize;
        attr->blocks    = stbuf->st_blocks;
        attr->atime     = stbuf->st_atime;
        attr->mtime     = stbuf->st_mtime;
        attr->ctime     = stbuf->st_ctime;
#ifdef __APPLE__
        attr->atimensec = 0;
        attr->mtimensec = 0;
        attr->ctimensec = 0;
#else
        attr->atimensec = ST_ATIM_NSEC(stbuf);
        attr->mtimensec = ST_MTIM_NSEC(stbuf);
        attr->ctimensec = ST_CTIM_NSEC(stbuf);
#endif
    }

#if 0
    void unsigned long FuseUtil::calcTimeoutSec(double t)
    {
        if (t > (double) ULONG_MAX)
            return ULONG_MAX;
        else if (t < 0.0)
            return 0;
        else
            return (unsigned long) t;
    }

    unsigned int FuseUtil::calcTimeoutNsec(double t)
    {
        double f = t - (double) calcTimeoutSec(t);
        if (f < 0.0)
            return 0;
        else if (f >= 0.999999999)
            return 999999999;
        else
            return (unsigned int) (f * 1.0e9);
    }
#endif

};
#endif

