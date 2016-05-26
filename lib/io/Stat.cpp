#include <dframework/io/Stat.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace dframework {

    Stat::Stat(){
        clear();
    }

    Stat::Stat(const char* uri)
            : Object()
    {
        clear();
        stat(uri);
    }

    Stat::Stat(const String& uri)
            : Object()
    {
        clear();
        stat(uri);
    }

    Stat::Stat(const URI& uri)
            : Object()
    {
        clear();
        stat(uri);
    }

    Stat::~Stat(){
    }

    void Stat::clear(){
        m_Uri.clear();
        ::memset(&m_stat, 0 , sizeof(m_stat));
#if 0
        m_dev = 0;
        m_ino = 0;
        m_mode = 0;
        m_nlink = 0;
        m_uid = 0;
        m_gid = 0;
        m_rdev = 0;
        m_size = 0;
        m_blksize = 0;
        m_blocks = 0;
        m_atime = 0l;
        m_mtime = 0l;
        m_ctime = 0l;
#endif
    }

    sp<Retval> Stat::stat(const char* uri){
        m_Uri = uri;
        return stat(m_Uri);
    }

    sp<Retval> Stat::stat(const String& uri){
        m_Uri = uri;
        return stat(m_Uri);
    }

    sp<Retval> Stat::stat(const URI& uri){
        m_Uri = uri;
        return (m_LastRetval = stat_real());
    }

    sp<Retval> Stat::stat_real(){
        ::memset(&m_stat, 0 , sizeof(m_stat));
        if( m_Uri.getScheme().equals("file") )
            return stat_file_real();
        return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
    }

    sp<Retval> Stat::stat_file_real(){
        if(m_Uri.getPath().empty())
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        const char *path = m_Uri.getPath().toChars();
#ifdef _WIN32
        if( -1 == ::stat(path, &m_stat) ){
#else
        if( -1 == ::lstat(path, &m_stat) ){
#endif
            int eno = errno;
            switch(eno){
            case EACCES:
                return DFW_RETVAL_NEW(DFW_E_ACCES, eno);
            case EBADF:
                return DFW_RETVAL_NEW(DFW_E_BADF, eno);
            case EFAULT:
                return DFW_RETVAL_NEW(DFW_E_FAULT, eno);
            case ELOOP:
                return DFW_RETVAL_NEW(DFW_E_LOOP, eno);
            case ENAMETOOLONG:
                return DFW_RETVAL_NEW(DFW_E_NAMETOOLONG, eno);
            case ENOENT:
                return DFW_RETVAL_NEW(DFW_E_NOENT, eno);
            case ENOTDIR:
                return DFW_RETVAL_NEW(DFW_E_NOTDIR, eno);
            case EOVERFLOW:
                return DFW_RETVAL_NEW(DFW_E_OVERFLOW, eno);
            case EISCONN:
                return DFW_RETVAL_NEW(DFW_E_ISCONN, eno);
            case ENOTCONN:
                return DFW_RETVAL_NEW(DFW_E_NOTCONN, eno);
            }
            return DFW_RETVAL_NEW(DFW_E_STAT, eno);
        }

#if 0
        m_dev = st.st_dev;
        m_ino = st.st_ino;
        m_mode = st.st_mode;
        m_nlink = st.st_nlink;
        m_uid = st.st_uid;
        m_gid = st.st_gid;
        m_rdev = st.st_rdev;
        m_size = st.st_size;
        m_blksize = st.st_blksize;
        m_blocks = st.st_blocks;
        m_atime = st.st_atime;
        m_mtime = st.st_mtime;
        m_ctime = st.st_ctime;
#endif
        return NULL;
    }

    bool Stat::operator == (const Stat & from){
        if( isDir() && !from.isDir() )
            return false;
        else if( !isDir() && from.isDir() )
            return false;
        return (m_Uri == from.m_Uri);
    }

    bool Stat::operator != (const Stat & from){
        if( isDir() && !from.isDir() )
            return true;
        else if( !isDir() && from.isDir() )
            return true;
        return (m_Uri != from.m_Uri);
    }

    bool Stat::operator >  (const Stat & from){
        if( isDir() && !from.isDir() )
            return false;
        else if( !isDir() && from.isDir() )
            return true;
        return (m_Uri > from.m_Uri);
    }

    bool Stat::operator <  (const Stat & from){
        if( isDir() && !from.isDir() )
            return true;
        else if( !isDir() && from.isDir() )
            return false;
        return (m_Uri < from.m_Uri);
    }

    bool Stat::operator >= (const Stat & from){
        if( isDir() && !from.isDir() )
            return false;
        else if( !isDir() && from.isDir() )
            return true;
        return (m_Uri >= from.m_Uri);
    }

    bool Stat::operator <= (const Stat & from){
        if( isDir() && !from.isDir() )
            return true;
        else if( !isDir() && from.isDir() )
            return false;
        return (m_Uri <= from.m_Uri);
    }


};

