#include <dframework/base/System.h>
#include <dframework/io/SSH2Fs.h>
#include <dframework/io/File.h>
#include <dframework/http/HttpContentType.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(SSH2Fs);

    SSH2Fs::SSH2Fs(){
        DFW_SAFE_ADD(SSH2Fs, l);
        m_session = new SSH2Session();
        m_uTimeout = 0;
    }

    SSH2Fs::~SSH2Fs(){
        DFW_SAFE_REMOVE(SSH2Fs, l);
    }

    sp<Retval> SSH2Fs::ready(sp<URI>& uri){
        sp<Retval> retval;
        m_sUri = uri->toString();
        m_oUri = uri;
        URI test = uri;

        if( DFW_RET(retval, m_session->ready(test)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    void SSH2Fs::setTimeout(unsigned long value){
        m_uTimeout = value;
    }

    sp<Retval> SSH2Fs::getattr(const char* path, struct stat* st){
        sp<Retval> retval;
        m_sPath = path;
        String sPath;
        if( m_oUri->getPath().empty() )
            sPath = path;
        else
            sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);
#ifndef _WIN32
        if( DFW_RET(retval, m_session->ftp_stat(sPath.toChars(), st)) )
            return DFW_RETVAL_D(retval);
#endif
        return NULL;
    }

    sp<Retval> SSH2Fs::readdir(const char* path, sp<DirBox>& db){
        sp<Retval> retval;
        m_sPath = path;
        String sPath;
        if( m_oUri->getPath().empty() )
            sPath = path;
        else
            sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);
        if( DFW_RET(retval, m_session->ftp_readdir(sPath, db)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Fs::open(const char* path, int flag, int mode){
        sp<Retval> retval;
        m_sPath = path;
        String sPath;
        if( m_oUri->getPath().empty() )
            sPath = path;
        else
            sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);
        if( DFW_RET(retval, m_session->ftp_open(sPath, flag, mode)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Fs::read(const char* path
                          , unsigned *outsize
                          , char* buf, uint32_t size
                          , uint64_t offset)
    {
        sp<Retval> retval;
        m_sPath = path;
        if( DFW_RET(retval, m_session->ftp_read(outsize, buf, size, offset)))
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Fs::close(const char* path){
        sp<Retval> retval;
        m_sPath = path;
        if( DFW_RET(retval, m_session->ftp_close()) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Fs::getContentType(String& sContentType){
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

    sp<Retval> SSH2Fs::setAttribute(int type, int value){
        DFW_UNUSED(type);
        DFW_UNUSED(value);
        return NULL;
    }

    sp<Retval> SSH2Fs::setAttribute(int type, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(value);
        return NULL;
    }

    sp<Retval> SSH2Fs::setAttribute(int type
                         , const char* name, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(name);
        DFW_UNUSED(value);
        return NULL;
    }


};

