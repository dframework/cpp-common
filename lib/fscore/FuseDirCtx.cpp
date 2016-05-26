#ifndef _WIN32
#include <dframework/fscore/FuseUtil.h>
#include <dframework/fscore/FuseRequest.h>
#include <dframework/fscore/FuseDirCtx.h>
#include <stddef.h>

namespace dframework {

    FuseDirCtx::FuseDirCtx(){
        m_bFilled = false;
        m_pContents = NULL;
        m_iLength = 0;
        m_iSize = 0;
    }

    FuseDirCtx::~FuseDirCtx(){
        DFW_FREE(m_pContents);
    }

    sp<Retval> FuseDirCtx::extend(size_t minsize){
        if(minsize > m_iSize) {
            char *newptr;
            size_t newsize = m_iSize;
            if(!newsize)
                newsize = 1024;
            while(newsize < minsize) {
                if (newsize >= 0x80000000)
                    newsize = 0xffffffff;
                else
                    newsize *= 2;
            }

            if( !(newptr = (char *) ::realloc(m_pContents, newsize)) )
                return DFW_RETVAL_NEW(DFW_E_NOMEM, -ENOMEM);
            m_pContents = newptr;
            m_iSize = newsize;
        }
        return NULL;
    }

    sp<Retval> FuseDirCtx::fill(
            const sp<FuseRequest>& req
          , const char* in_name, const struct stat* in_st)
    {
        DFW_UNUSED(req);

        sp<Retval> retval;
        sp<String> s_name = new String(in_name);

        if(m_nameList.get(s_name).has())
            return NULL;
        if( DFW_RET(retval, m_nameList.insert(s_name)) )
            return DFW_RETVAL_D(retval);

        unsigned namelen = strlen(in_name);
        unsigned entlen  = FUSE_NAME_OFFSET + namelen;
        unsigned entsize = FUSE_DIRENT_ALIGN(FUSE_NAME_OFFSET+namelen);
        unsigned padlen  = entsize - entlen;
        size_t   newlen  = m_iLength + entsize;

        if( DFW_RET(retval, extend(newlen)) )
            return DFW_RETVAL_D(retval);

        char *buf = m_pContents + m_iLength;
        struct fuse_dirent *dirent = (struct fuse_dirent *)buf;
        dirent->ino = FUSE_UNKNOWN_INO; //FIXME: in_st->st_ino;
        dirent->off = newlen;
        dirent->namelen = namelen;
        dirent->type = (in_st->st_mode & 0170000) >> 12;
        strncpy(dirent->name, in_name, namelen);
        if (padlen)
                memset(buf + entlen, 0, padlen);

        m_iLength = newlen;

        return NULL;
    }

};
#endif

