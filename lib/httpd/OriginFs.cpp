#include <dframework/httpd/OriginFs.h>
#include <dframework/httpd/HttpdHost.h>

namespace dframework {

    OriginFs::OriginFs(){
        m_bInit = false;
    }

    OriginFs::~OriginFs(){
    }

    sp<Retval> OriginFs::ready(sp<Object>& host){
        AutoLock _l(this);
        sp<Retval> retval;
        m_bInit = false;
        if( !host.has() )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not has host.");
        m_host = host;
        m_bInit = true;
        return NULL;
    }

    sp<Retval> OriginFs::getattr(const char* path, struct stat* st){
        sp<Retval> retval;
        if( DFW_RET(retval, getattr_l(path, st)) )
            return DFW_RETVAL_D(retval);
        return NULL;
/*
        int round = 0;
        do{
            round++;
            if( DFW_RET(retval, getattr_l(path, st)) ){
                if(round>10)
                    return DFW_RETVAL_D(retval);
                usleep(1000*100);
                continue;
            }
            return NULL;
        }while(true);
*/
    }

    sp<Retval> OriginFs::getattr_l(const char* path, struct stat* st){
        sp<Retval> retval;

        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");

        sp<HttpdHost> host = m_host;
        sp<HttpdHost::AliasUri> uris = host->getAliasUri(path);
        if(!uris.has()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "No enable orgin uris. path=%s", path);
        }

        int pathoff = uris->getAliasName().length();
        if(pathoff!=0) pathoff++;
        int start = uris->getCurrent();
        int size = uris->size();
        int cur = start + 1;
        while(true){
            if( cur >= size ) cur = 0;

            sp<URI> uri = uris->getUri(cur);
            if(uri.has()){
                sp<UriFs> fs = new UriFs();
                if( DFW_RET(retval, fs->ready(uri)) ){
                }else if( DFW_RET(retval, fs->getattr(path+pathoff, st)) ){
                }else{
                    uris->setCurrent(cur);
                    return NULL;
                }
            }

            cur++;
            if( cur == (start + 1)){
                if( retval.has() )
                    return DFW_RETVAL_D(retval);
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "No enable orgin uris. path=%s, uri-size=%d"
                           , path, size);
            }
        }

        return NULL;
    }

    sp<Retval> OriginFs::readdir(const char* path, sp<DirBox>& db){
        sp<Retval> retval;

        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");

        sp<HttpdHost> host = m_host;
        sp<HttpdHost::AliasUri> uris = host->getAliasUri(path);
        if(!uris.has()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "No enable orgin uris. path=%s", path);
        }

        int pathoff = uris->getAliasName().length();
        if(pathoff!=0) pathoff++;
        int start = uris->getCurrent();
        int size = uris->size();
        int cur = start + 1;
        while(true){
            if( cur >= size ) cur = 0;

            sp<URI> uri = uris->getUri(cur);
            if(uri.has()){
                sp<UriFs> fs = new UriFs();
                if( DFW_RET(retval, fs->ready(uri)) ){
                }else if( DFW_RET(retval, fs->readdir(path+pathoff, db)) ){
                }else{
                    uris->setCurrent(cur);
                    return NULL;
                }
            }

            cur++;
            if( cur == (start + 1)){
                if( retval.has() )
                    return DFW_RETVAL_D(retval);
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "No enable orgin uris. path=%s, uri-size=%d"
                           , path, size);
            }
        }

        return NULL;
    }

    sp<Retval> OriginFs::open(const char* path, int flag, int mode){
        sp<Retval> retval;

        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");

        sp<HttpdHost> host = m_host;
        sp<HttpdHost::AliasUri> uris = host->getAliasUri(path);
        if(!uris.has()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "No enable orgin uris. path=%s", path);
        }

        int poff = uris->getAliasName().length();
        if(poff!=0) poff++;
        int start = uris->getCurrent();
        int size = uris->size();
        int cur = start + 1;
        while(true){
            if( cur >= size ) cur = 0;

            sp<URI> uri = uris->getUri(cur);
            if(uri.has()){
                sp<UriFs> fs = new UriFs();
                if( DFW_RET(retval, fs->ready(uri)) ){
                }else if( DFW_RET(retval, fs->open(path+poff, flag, mode)) ){
                }else{
                    uris->setCurrent(cur);
                    m_fs = fs;
                    m_poff = poff;
                    return NULL;
                }
            }

            cur++;
            if( cur == (start + 1)){
                if( retval.has() )
                    return DFW_RETVAL_D(retval);
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "No enable orgin uris. path=%s, uri-size=%d"
                           , path, size);
            }
        }

        return NULL;
    }

    sp<Retval> OriginFs::read(const char* path, unsigned *outsize
                              , char* buf, uint32_t size, uint64_t offset)
    {
        sp<Retval> retval;

        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");

        if(m_fs.has()){
            if( DFW_RET(retval, m_fs->read(path+m_poff, outsize
                                         , buf, size, offset)) ){
                return DFW_RETVAL_D(retval);
            }
            return NULL;
        }
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Has not UriFs for read. path=%s", path);
    }

    sp<Retval> OriginFs::close(const char* path){
        sp<Retval> retval;

        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");

        if(m_fs.has()){
            if( DFW_RET(retval, m_fs->close(path+m_poff)) )
                return DFW_RETVAL_D(retval);
            return NULL;
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Has not UriFs for close. path=%s", path);
    }

    sp<Retval> OriginFs::getContentType(String& contentType){
        sp<Retval> retval;
        if(!m_bInit)
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not ready.");
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not fs.");
        if( DFW_RET(retval, m_fs->getContentType(contentType)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

};

