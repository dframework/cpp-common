#ifndef _WIN32
#include <dframework/fscore/FuseOpenCtx.h>

namespace dframework {

    FuseOpenCtx::FuseOpenCtx(){
        m_flags = 0;
        m_direct_io = true;
        m_keep_cache = true;
        m_nonseekable = true;
    }

    FuseOpenCtx::~FuseOpenCtx(){
    }

};
#endif

