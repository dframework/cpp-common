#ifndef DFRAMEWORK_FSCORE_FUSEOPENCTX_H
#define DFRAMEWORK_FSCORE_FUSEOPENCTX_H

#include <dframework/base/Retval.h>
#include <dframework/fscore/FuseRequest.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseOpenCtx : public FsContext
    {
    public:
        int  m_flags;
        bool m_direct_io;
        bool m_keep_cache;
        bool m_nonseekable;

        FuseOpenCtx();
        virtual ~FuseOpenCtx();

    };

};
#endif
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEOPENCTX_H */

