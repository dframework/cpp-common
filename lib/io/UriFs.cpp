#include <dframework/io/UriFs.h>
#include <dframework/io/HttpFs.h>
#include <dframework/io/LocalFs.h>
#include <dframework/io/SSH2Fs.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(UriFs);

    UriFs::UriFs() {
        DFW_SAFE_ADD(UriFs, l);
    }

    UriFs::~UriFs(){
        close("");
        DFW_SAFE_REMOVE(UriFs, l);
    }

    sp<Retval> UriFs::ready(const char* uri)
    {
        sp<Retval> retval;
        if(uri)
            m_sUri = uri;
        if(!m_sUri.length())
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
        if( DFW_RET(retval, m_oUri->parse(uri)) )
            return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, onFindBaseFs(m_oUri, m_fs)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::ready(sp<URI>& uri){
        sp<Retval> retval;
        m_sUri = uri->toString();
        m_oUri = uri;
        if( DFW_RET(retval, onFindBaseFs(uri, m_fs)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::onFindBaseFs(sp<URI>& uri, sp<BaseFs>& fs)
    {
        sp<Retval> retval;
        String scheme = uri->getScheme();
        fs = NULL;

        if(scheme.equals("file"))
            fs = new LocalFs();
        else if(scheme.equals("http"))
            fs = new HttpFs();
        else if(scheme.equals("https"))
            fs = new HttpFs();
        else if(scheme.equals("webdav"))
            fs = new HttpFs();
        else if(scheme.equals("webdavs"))
            fs = new HttpFs();
        else if(scheme.equals("sftp"))
            fs = new SSH2Fs();

        if( !fs.has() )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "scheme=%s, Not supported scheme.", scheme.toChars());

        if( DFW_RET(retval, fs->ready(uri) ) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    void UriFs::setTimeout(unsigned long value){
        if(m_fs.has()){
            m_fs->setTimeout(value);
        }
    }

    sp<Retval> UriFs::getattr(const char* path, struct stat* st)
    {
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "uri=%s, path=%s, Unknown BaseFs. "
                         "Check onFindBaseFs(..) function."
                       , m_sUri.toChars(), path);
        if( DFW_RET(retval, m_fs->getattr(path, st)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::readdir(const char* path, sp<DirBox>& db)
    {
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "uri=%s, path=%s, Unknown BaseFs. "
                         "Check onFindBaseFs(..) function."
                       , m_sUri.toChars(), path);
        if( DFW_RET(retval, m_fs->readdir(path, db)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::open(const char* path, int flag, int mode)
    {
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "uri=%s, path=%s, Unknown BaseFs. "
                         "Check onFindBaseFs(..) function."
                       , m_sUri.toChars(), path);
        if( DFW_RET(retval, m_fs->open(path, flag, mode)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::read(const char* path
                         , unsigned *outsize
                         , char* buf, uint32_t size, uint64_t offset)
    {
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "uri=%s, path=%s, Unknown BaseFs. "
                         "Check onFindBaseFs(..) function."
                       , m_sUri.toChars(), path);
        if( DFW_RET(retval, m_fs->read(path, outsize, buf, size, offset)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::close(const char* path)
    {
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "uri=%s, path=%s, Unknown BaseFs. "
                         "Check onFindBaseFs(..) function."
                       , m_sUri.toChars(), path);
        if( DFW_RET(retval, m_fs->close(path)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::getContentType(String& sContentType){
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown BaseFs.");
        if( DFW_RET(retval, m_fs->getContentType(sContentType)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::setAttribute(int type, int value){
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown BaseFs.");
        if( DFW_RET(retval, m_fs->setAttribute(type, value)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::setAttribute(int type, const char* value){
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown BaseFs.");
        if( DFW_RET(retval, m_fs->setAttribute(type, value)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> UriFs::setAttribute(int type
                         , const char* name, const char* value){
        sp<Retval> retval;
        if(!m_fs.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown BaseFs.");
        if( DFW_RET(retval, m_fs->setAttribute(type, name, value)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }


};

