#include <dframework/io/File.h>
#include <dframework/io/DirBox.h>
#include <dframework/net/URI.h>
#include <dframework/net/Poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#ifdef _WIN32

ssize_t pread (int fd, void *buf, size_t count, off_t offset)
{
  if (::lseek(fd, offset, SEEK_SET) != offset)
    return -1;
  return ::read(fd, buf, count);
}

#endif

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(File);

    File::File(){
        DFW_SAFE_ADD(File, l);

        DFW_FILE_INIT(m_fd);
        m_offset = 0;
        m_uTimeout = 0;
    }

    File::~File(){
        close_l();
        DFW_SAFE_REMOVE(File, l);
    }

    void File::close(){
        close_l();
    }

    void File::setTimeout(unsigned long value){
        if( m_uTimeout && !value && m_fd ){
            m_uTimeout = 10000;
        }else{
            m_uTimeout = value;
        }
    }

    sp<Retval> File::open(const char* path, int flag){
        sp<Retval> retval;
        close_l();
        m_sPath = path;
#ifdef _WIN32
        m_sPath.replace('/','\\');
#endif
        if( DFW_RET(retval, File::open(&m_fd, m_sPath.toChars(), flag)) ){
            return DFW_RETVAL_D(retval);
        }
        if( m_uTimeout ){
            if( DFW_RET(retval, File::setNonBlockSocket(m_fd, true)) ){
                return DFW_RETVAL_D(retval);
            }
        }
        return NULL;
    }

    sp<Retval> File::open(const char* path, int flag, int mode){
        sp<Retval> retval;
        close_l();
        m_sPath = path;
#ifdef _WIN32
        m_sPath.replace('/','\\');
#endif
        if( DFW_RET(retval, File::open(&m_fd, m_sPath.toChars(), flag, mode)) ){
            return DFW_RETVAL_D(retval);
        }
        if( m_uTimeout ){
            if( DFW_RET(retval, File::setNonBlockSocket(m_fd, true)) ){
                return DFW_RETVAL_D(retval);
            }
        }
        return NULL;
    }

    void File::close_l(){
        DFW_FILE_CLOSE(m_fd);
        m_offset = 0;
    }

    sp<Retval> File::read(unsigned *out_size, char* buf, unsigned size
                        , uint64_t offset)
    {
        sp<Retval> retval;
        if( (offset!=m_offset) && DFW_RET(retval, seek(offset)) ){
            return DFW_RETVAL_D(retval);
        }
        if( m_uTimeout ){
            if( DFW_RET(retval, File::isReadable(m_fd, m_uTimeout)) ){
                return DFW_RETVAL_D(retval);
            }
        }
        if( DFW_RET(retval, File::read(m_fd, out_size, buf, size)) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> File::write(const char* buf, unsigned size, uint64_t offset){
        sp<Retval> retval;
        if( (offset!=m_offset) && DFW_RET(retval, seek(offset)) )
            return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, File::write(m_fd, buf, size)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> File::seek(uint64_t offset){
        sp<Retval> retval;
        if( offset!=m_offset ){
            if( DFW_RET(retval, File::seek(m_fd, offset)) )
                return DFW_RETVAL_D(retval);
            m_offset = offset;
        }
        return NULL;
    }

    sp<Retval> File::lastSeek(uint64_t offset){
        sp<Retval> retval;
        if( offset!=m_offset ){
            if( DFW_RET(retval, File::lastSeek(m_fd, offset)) )
                return DFW_RETVAL_D(retval);
            m_offset = offset;
        }
        return NULL;
    }

    // --------------------------------------------------------------

    DFW_STATIC
    bool File::isAccess(const char* path){
        if( ::access(path, R_OK)==0 )
            return true;
        return false;
    }

    DFW_STATIC
    bool File::isFile(const char* path){
        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));
        if( -1 == ::stat(path, &st) )
            return false;
        return S_ISREG(st.st_mode);
    }

    DFW_STATIC
    bool File::isDirectory(const char* path){
        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));
        if( -1 == ::stat(path, &st) )
            return false;
        return S_ISDIR(st.st_mode);
    }

    DFW_STATIC
    sp<Retval> File::makeDirectory(const char* path, int mode){
        if( isDirectory(path) )
            return NULL;
#ifdef _WIN32
        DFW_UNUSED(mode);
        if( -1 == ::mkdir(path) ){
#else
        mode_t imode = (mode_t)mode;
        if( -1 == ::mkdir(path, imode) ){
#endif
            int eno = errno;
            dfw_retno_t rno = DFW_ERROR;
            const char* msg = Retval::errno_short(&rno, eno, "Not mkdir");
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "Not make directory. path=%s, (%s)", path, msg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::makeDir(const char* basedir
                           , const char* path, int mode)
    {
        sp<Retval> retval;

        String sMakePath = basedir;
        sp<URI::PathInfo> pi = new URI::PathInfo(path);

        for(int k=1; k<pi->size(); k++){
            String sPath = pi->path(k);
            sMakePath.appendFmt("/%s", sPath.toChars());
            if( DFW_RET(retval, File::makeDirectory(
                                         sMakePath.toChars(), mode)))
                return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> File::open(int* out_fd, const char* path, int flag){
    DFW_STATIC
        sp<Retval> retval;
        return DFW_RET_C(retval, open(out_fd, path, flag, 0));
    }

    DFW_STATIC
    sp<Retval> File::open(int* out_fd, const char* path, int flag, int mode){
        if(!out_fd)
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                 , "out_fd parameter is null.");

        int fd;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;
        const char* ppath;

#ifdef _WIN32
        String sPath = path;
        sPath.replace('/','\\');
        ppath = sPath.toChars();
        if( -1 == (fd = ::_open(ppath, flag, mode)) ){
#else
        ppath = path;
        if( -1 == (fd =  ::open(ppath, flag, mode)) ){
#endif
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not open file");
            return DFW_RETVAL_NEW_MSG(rno, eno, "path=%s, %s", ppath, msg);
        }

        *out_fd = fd;

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::truncate(const char* path, uint64_t size){
        sp<Retval> retval;
        int res;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

#ifdef _WIN32
        String sPath = path;
        sPath.replace('/','\\');
        if( -1 == (res = ::truncate(sPath.toChars(), size)) ){
#else
        if( -1 == (res = ::truncate(path, size)) ){
#endif
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not truncate file");
            return DFW_RETVAL_NEW_MSG(rno, eno, "path=%s, size=%ld, %s"
                                              , path, size, msg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::truncate(int fd, uint64_t size, const char* path){
        sp<Retval> retval;
        int res;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        if( -1 == (res = ::ftruncate(fd, size)) ){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not truncate file");
            if(path)
                return DFW_RETVAL_NEW_MSG(rno, eno
                           , "path=NULL, fd=%d, size=%ld, %s"
                           , fd, size, msg);
            else
                return DFW_RETVAL_NEW_MSG(rno, eno
                           , "path=%s, fd=%d, size=%ld, %s"
                           , path, fd, size, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::contents(String& contents, const char* path){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        int fd;
        DFW_FILE_INIT(fd);
#ifdef _WIN32
        int flags = O_RDONLY | O_BINARY;
#else
        int flags = O_RDONLY;
#endif
        if( DFW_RET(retval, open(&fd, path, flags)) )
            return DFW_RETVAL_D(retval);

        char buf[4096];
        int size = 4096;
        int recv = 0;
        while(true){
            recv = ::read(fd, buf, size);
            if(-1 == recv){
                eno = errno;
                ::close(fd);
                msg = Retval::errno_short(&rno, eno, "Not read file");
                return DFW_RETVAL_NEW_MSG(rno, eno, "path=%s, %s", path, msg);
            }
            if( 0 == recv){
                ::close(fd);
                return NULL; 
            }
            contents.append(buf, recv);
        }
    }

    DFW_STATIC
    sp<Retval> File::read(int fd, const char* path
                        , unsigned *out_size
                        , char* buf, uint32_t size, uint64_t offset)
    {
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        if( ((unsigned)-1)==(*out_size = ::pread(fd, buf, size, offset)) ){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not read file");
            if(path)
                return DFW_RETVAL_NEW_MSG(rno, eno
                           , "fd=%d, offset=%lu, size=%u, path=%s, %s"
                           , fd, offset, size, path, msg);
            else
                return DFW_RETVAL_NEW_MSG(rno, eno
                           , "fd=%d, offset=%lu, size=%u, path=NULL, %s"
                           , fd, offset, size, msg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::read(int fd, unsigned *out_size
                        , char* buf, uint32_t size)
    {
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        if( ((unsigned)-1)==(*out_size = ::read(fd, buf, size)) ){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not read file");
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "fd=%d, size=%u, %s"
                       , fd, size, msg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::read(int fd, unsigned *out_size
                        , char* buf, uint32_t size, uint64_t offset)
    {
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        if( ((unsigned)-1)==(*out_size = ::pread(fd, buf, size, offset)) ){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not read file");
            return DFW_RETVAL_NEW_MSG(rno, eno
                       , "fd=%d, offset=%lu, size=%u, %s"
                       , fd, offset, size, msg);
        }
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::save(const char* buf, unsigned size, const char* path){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;
        unsigned sended = 0;
        int fd;
        DFW_FILE_INIT(fd);

#ifdef _WIN32
        int flags = O_WRONLY|O_CREAT|O_BINARY;
#else
        int flags = O_WRONLY|O_CREAT;
#endif
        if( DFW_RET(retval, open(&fd, path, flags, 0644)) )
            return DFW_RETVAL_D(retval);
        
        do{
            unsigned send = ::write(fd, buf+sended, size);
            if((unsigned)-1 == send){
                eno = errno;
                ::close(fd);
                msg = Retval::errno_short(&rno, eno, "Not write file");
                return DFW_RETVAL_NEW_MSG(rno, eno, "path=%s, %s", path, msg);
            }
            if( 0 == send){
                ::close(fd);
                return NULL; 
            }
            sended += send;
            size -= send;
            if( 0==size ){
                ::close(fd);
                return NULL; 
            }
        }while(true);
    }

    DFW_STATIC
    sp<Retval> File::write(int fd, const char* buf, unsigned size){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        unsigned sended = 0;
        unsigned send = 0;
        unsigned left = size;
        while(true){
            send = ::write(fd, buf+sended, left);
            if(((unsigned)-1) == send){
                eno = errno;
                msg = Retval::errno_short(&rno, eno, "Not write file");
                return DFW_RETVAL_NEW_MSG(rno, eno
                        , "fd=%d, size=%u, sended=%u, left=%u, %s"
                        , fd, size, sended, left, msg);
            }
            sended += send;
            left -= send;
            if(!left){
                return NULL;
            }
        }
    }

    DFW_STATIC
    sp<Retval> File::write(int fd, const char* contents, unsigned size
                              , uint64_t offset)
    {
        sp<Retval> retval;
        if( DFW_RET(retval, seek(fd, offset)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, write(fd, contents, size));
    }

    DFW_STATIC
    sp<Retval> File::seek(int fd, uint64_t offset){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        dfw_uint_t res = (dfw_uint_t)::lseek(fd, offset, SEEK_SET);
        if(((dfw_uint_t)-1) == res){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not seek.");
            return DFW_RETVAL_NEW_MSG(rno, eno
                    , "fd=%d, offset=%ld, %s"
                    , fd, offset, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::lastSeek(int fd, uint64_t offset){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        dfw_uint_t res = (dfw_uint_t)::lseek(fd, offset, SEEK_END);
        if(((dfw_uint_t)-1) == res){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not lastSeek.");
            return DFW_RETVAL_NEW_MSG(rno, eno
                    , "fd=%d, offset=%ld, %s"
                    , fd, offset, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::remove(const char* path){
        sp<Retval> retval;
        int res;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;
        const char* ppath;
#ifdef _WIN32
        String sPath = path;
        sPath.replace('/', '\\');
        ppath = sPath.toChars();
        if( -1 == (res = ::_unlink(ppath)) ){
#else
        ppath = path;
        if( -1 == (res = ::remove(ppath)) ){
#endif

            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not remove.");
            return DFW_RETVAL_NEW_MSG(rno, eno
                    , "path=%s, %s", ppath, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::removePath(const char* base, const char* path){
        sp<Retval> retval;
        sp<URI::PathInfo> pi = new URI::PathInfo(path);

        if( pi->size() <= 1 )
            return NULL;

        for(int k=(pi->size()-1); k>=1; k--){
            String test = pi->fullpath(k);
            if(test.empty()) return NULL;

            String testpath = String::format("%s%s", base, test.toChars());
#ifdef _WIN32
            testpath.replace('/', '\\');
#endif
            if( File::isDirectory(testpath)){
                sp<DirBox> db = new DirBox(testpath);
                if(db->size()==0){
                    if(DFW_RET(retval, File::remove(testpath))){
                        return DFW_RETVAL_D(retval);
                    }
                    continue;
                }
                return NULL;
            }else if( File::isFile(testpath) ){
                if(DFW_RET(retval, File::remove(testpath))){
                    return DFW_RETVAL_D(retval);
                }
            }
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::removeAll(const char* ipath){
        sp<Retval> retval;
        const char* ppath;

#ifdef _WIN32
        String sPath = ipath;
        sPath.replace('/', '\\');
        ppath = sPath.toChars();
#else
        ppath = ipath;
#endif

        if( !File::isDirectory(ppath) ){
            if( DFW_RET(retval, remove(ppath)) )
                return DFW_RETVAL_D(retval);
            return NULL;
        }

        sp<DirBox> db = new DirBox(ppath);
        for(int k=0; k<db->size(); k++){
            sp<Stat> st = db->get(k);
            if(st.has()){
                if(st->m_name.length()==0 
                       || st->m_name.equals(".") || st->m_name.equals(".."))
                    continue;
#ifdef _WIN32
                String sSubPath = String::format("%s\\%s"
                                          , ppath, st->m_name.toChars());
#else
                String sSubPath = String::format("%s/%s"
                                          , ppath, st->m_name.toChars());
#endif
                if(st->isDir()){
                    if( DFW_RET(retval, removeAll(sSubPath)) )
                        return DFW_RETVAL_D(retval);
                }else if( DFW_RET(retval, remove(sSubPath)) ){
                    return DFW_RETVAL_D(retval);
                }
            }
        }

        if( DFW_RET(retval, remove(ppath)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::rename(const char* oldname, const char* newname){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        int res = ::rename(oldname, newname);
        if(-1 == res){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not rename.");
            return DFW_RETVAL_NEW_MSG(rno, eno
                    , "oldname=%s, newname=%s, %s"
                    , oldname, newname, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::mtime(const char* path, uint64_t mtime){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        struct utimbuf buf;
        ::memset(&buf, 0, sizeof(struct utimbuf));
        buf.actime = (time_t)mtime;
        buf.modtime = (time_t)mtime;

        int res = ::utime(path, &buf);
        if(-1 == res){
            eno = errno;
            msg = Retval::errno_short(&rno, eno, "Not utime.");
            return DFW_RETVAL_NEW_MSG(rno, eno
                    , "path=%s, mtime=%lu, %s"
                    , path, mtime, msg);
        }

        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::setNonBlockSocket(int fd, bool is){
#ifdef _WIN32
        DFW_UNUSED(fd);
        DFW_UNUSED(is);
#else
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	if(-1 == flags){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, errno
                     , "Not set non blocking fd by GETFL. fd=%d", fd);
        }
	if(-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK)){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, errno
                     , "Not set non blocking fd. fd=%d", fd);
        }
#endif
        return NULL;
    }

    DFW_STATIC
    sp<Retval> File::isReadable(int fd, int timeout){
        int ret;
        int eno;

        struct pollfd fds;
        fds.fd = fd;
        fds.events = POLLIN|POLLERR|POLLHUP|POLLNVAL;
        fds.revents = 0;
#ifdef _WIN32
        if( -1 == (ret = win32_poll(&fds, 1, timeout)) )
#else
        if( -1 == (ret = ::poll(&fds, 1, timeout)) )
#endif
        {
                eno = errno;
                switch(eno){
                case EINTR:
                    return DFW_RETVAL_NEW(DFW_E_INTR,eno);
                case EFAULT :
                    return DFW_RETVAL_NEW(DFW_E_FAULT,eno);
                case EINVAL:
                    return DFW_RETVAL_NEW(DFW_E_INVAL,eno);
                case ENOMEM:
                    return DFW_RETVAL_NEW(DFW_E_NOMEM,eno);
                }
                return DFW_RETVAL_NEW(DFW_E_POLL,eno);
        }else if( 0 == ret ){
            return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT,0
                       , "handle=%d, timeout=%lu, TIMEOUT", fd, timeout);
        }

        if( (fds.revents & POLLIN)==POLLIN ){
            return NULL;
        }

        if( (fds.revents & POLLERR) == POLLERR ){
            return DFW_RETVAL_NEW_MSG(DFW_E_POLLERR,0
                       , "handle=%d, timeout=%lu, POLLERR", fd, timeout);
        }else if( (fds.revents & POLLHUP) == POLLHUP ){
            return DFW_RETVAL_NEW_MSG(DFW_E_POLLHUP,0
                       , "handle=%d, timeout=%lu, POLLHUP", fd, timeout);
        }else if( (fds.revents & POLLNVAL) == POLLNVAL ){
            return DFW_RETVAL_NEW_MSG(DFW_E_POLLNVAL,0
                       , "handle=%d, timeout=%lu, POLLNVAL", fd, timeout);
        }
        return DFW_RETVAL_NEW_MSG(DFW_E_POLL,0
                   , "handle=%d, timeout=%lu", fd, timeout);
    }

};

