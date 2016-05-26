#include <dframework/base/System.h>
#include <dframework/io/LocalFs.h>
#include <dframework/io/File.h>
#include <dframework/http/HttpContentType.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(LocalFs);

    LocalFs::LocalFs(){
        DFW_SAFE_ADD(LocalFs, l);
        //DFW_FD_INIT(m_fd);
    }

    LocalFs::~LocalFs(){
        DFW_SAFE_REMOVE(LocalFs, l);
        //DFW_CLOSE(m_fd);
    }

    sp<Retval> LocalFs::ready(sp<URI>& uri){
        m_sUri = uri->toString();
        m_oUri = uri;
        return NULL;
    }

    sp<Retval> LocalFs::getattr(const char* path, struct stat* st){
        sp<Retval> retval;
        m_sPath = path;
        String rpath = String::format("%s%s"
                                    , m_oUri->getPath().toChars()
                                    , path);
        Stat ost;
        if( DFW_RET(retval, ost.stat(rpath.toChars())) )
            return DFW_RETVAL_D(retval);
        ::memcpy((void*)st, (void*)&ost.m_stat, sizeof(struct stat));
        return NULL;
    }

    sp<Retval> LocalFs::readdir(const char* path, sp<DirBox>& db){
        sp<Retval> retval;
        m_sPath = path;
        String rpath = String::format("%s%s"
                                    , m_oUri->getPath().toChars()
                                    , path);
        const char* prpath = rpath.toChars();
        DIR *dp;
        struct dirent *de;
        dp = ::opendir(prpath);
        if (dp == NULL)
            return DFW_RETVAL_NEW(DFW_ERROR, errno);
        while ((de = ::readdir(dp)) != NULL) {
            sp<Stat> st = new Stat();
            String subpath = String::format("%s/%s", prpath, de->d_name);
            if( DFW_RET(retval, st->stat(subpath)) ){
#ifdef _WIN32
                continue;
#else
                st->m_stat.st_ino = de->d_ino;
                st->m_stat.st_mode = de->d_type << 12;
#endif
            }
            st->m_name = de->d_name;
            if( DFW_RET(retval, db->insert(st)) ){
                ::closedir(dp);
                return DFW_RETVAL_D(retval);
            }
        }
        ::closedir(dp);
        return NULL;
    }

    sp<Retval> LocalFs::open(const char* path, int flag, int mode){
        sp<Retval> retval;
        m_sPath = path;
        String rpath = String::format("%s%s"
                                    , m_oUri->getPath().toChars()
                                    , path);
        m_file = new File();
        if( DFW_RET(retval, m_file->open(rpath.toChars(), flag, mode)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> LocalFs::read(const char* path
                          , unsigned *outsize
                          , char* buf, uint32_t size
                          , uint64_t offset)
    {
        sp<Retval> retval;
        if(!m_file.has())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not file object.");
        m_sPath = path;
        String rpath = String::format("%s%s"
                                    , m_oUri->getPath().toChars()
                                    , path);
        if( DFW_RET(retval, m_file->read(outsize, buf, size, offset)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> LocalFs::close(const char* path){
        m_sPath = path;
        m_file = NULL;
        return NULL;
    }

    sp<Retval> LocalFs::getContentType(String& sContentType){
        URI::FileInfo fi;
        fi.parse(m_sPath.toChars());
        const char* ext = fi.m_sExtension.toChars();
        String sType = HttpContentType::getContentType(ext);
        bool bUseEncode = false;
        if( sType.indexOf("text/")==0 ){
            bUseEncode = true;
        }else if( sType.indexOf("xml/")==0 ){
            bUseEncode = true;
        }else if( sType.indexOf("/xml")!=((dfw_size_t)-1) ){
            bUseEncode = true;
        }

        if(bUseEncode){
            const char* enc = System::encoding();
            if(enc){
                sContentType = String::format("%s; charset=%s"
                                            , sType.toChars(), enc);
            }else{
                sContentType = sType;
            }
        }else{
            sContentType = sType;
        }

        return NULL;
    }

    sp<Retval> LocalFs::setAttribute(int type, int value){
        DFW_UNUSED(type);
        DFW_UNUSED(value);
        return NULL;
    }

    sp<Retval> LocalFs::setAttribute(int type, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(value);
        return NULL;
    }

    sp<Retval> LocalFs::setAttribute(int type
                         , const char* name, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(name);
        DFW_UNUSED(value);
        return NULL;
    }


};

