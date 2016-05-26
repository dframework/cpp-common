#ifndef _WIN32
#include <dframework/fscore/FuseMount.h>

namespace dframework {

    const char*  FuseMount::DEVICE_NAME = "/dev/fuse";
    const size_t FuseMount::S_PAGE_SIZE = getpagesize();
    const size_t FuseMount::BUFFER_SIZE
            = (((FuseMount::S_PAGE_SIZE+0x1000) < FuseMount::MIN_BUFFER_SIZE)
            ? FuseMount::MIN_BUFFER_SIZE : (FuseMount::S_PAGE_SIZE+0x1000));

    Object FuseMount::SAFE_LOCK;

    FuseMount::FuseMount(){
        init();
    }

    FuseMount::FuseMount(const char* mountpoint)
            : Mount()
    {
        init();
        setDeviceName(DEVICE_NAME);
        setMountPoint(mountpoint);
    }

    FuseMount::~FuseMount(){
    }

    void FuseMount::init(){
        ::memset( &m_conn, 0, sizeof(struct conn_info) );
        m_conn.async_read = 1;
        m_conn.max_write = (unsigned)-1;
        m_conn.max_readahead = (unsigned)-1;

        m_bGetInit = false;
        m_currentUnique = 0;
    }

    void FuseMount::setUnique(uint64_t unique, int opcode){
        AutoLock _l(this);
        m_currentUnique = unique;
        m_lastOpcode = opcode;
    }

    uint64_t FuseMount::getUnique(){
        AutoLock _l(this);
        return m_currentUnique;
    }

    int FuseMount::getLastOpcode(){
        AutoLock _l(this);
        return m_lastOpcode;
    }

    sp<Retval> FuseMount::mount(const char* mountpoint){
        sp<Retval> retval;
        setDeviceName(DEVICE_NAME);
        setMountPoint(mountpoint);
        return DFW_RET_C(retval, Mount::mount());
    }

    sp<Retval> FuseMount::mount(const String& mountpoint){
        sp<Retval> retval;
        setDeviceName(DEVICE_NAME);
        setMountPoint(mountpoint);
        return DFW_RET_C(retval, Mount::mount());
    }

    size_t FuseMount::iov_length(const struct iovec *iov, size_t count)
    {
        size_t ret = 0;
        for (size_t seg = 0; seg < count; seg++)
                ret += iov[seg].iov_len;
        return ret;
    }

    sp<Retval> FuseMount::send(const dfw_point_t piov, size_t count)
    {
        //AutoLock _l(m_send_lock);
        sp<Retval> retval;
        const struct iovec* iov = (struct iovec*)piov;
        if (!iov)
            return NULL;

        dfw_retno_t retno;
        ssize_t res;
        int eno;
        do{
          {
              AutoLock _l(&FuseMount::SAFE_LOCK);
              res = ::writev(getHandle(), iov, count);
              eno = errno;
          }
          if (res == -1) {
            switch(eno){
            case ENOENT: retno = DFW_E_NOENT; break;
                /* ENOENT means the operation was interrupted */
            case EINTR: retno = DFW_E_INTR; break;
            case EAGAIN: retno = DFW_E_AGAIN; break;
            case EINVAL: retno = DFW_E_INVAL; break;
            case EBADF: retno = DFW_E_BADF; break;
            case EDESTADDRREQ: retno = DFW_E_DESTADDRREQ; break;
            case EFAULT: retno = DFW_E_FAULT; break;
            case EFBIG: retno = DFW_E_FBIG; break;
            case EIO: retno = DFW_E_IO; break;
            case ENOSPC: retno = DFW_E_NOSPC; break;
            case EPIPE: retno = DFW_E_PIPE; break;
            default:
                retno = DFW_E_WRITEV; break;
            }
printf("wrtei error eno=%d\n", eno);
            return DFW_RETVAL_NEW_MSG(retno, eno
                  , "handle=%d, "
                    "Not send to device"
                  , getHandle());
          }
          return NULL;
        }while(true);
    }

    sp<Retval> FuseMount::send(uint64_t unique, int error
                             , const dfw_point_t arg, size_t size)
    {
        sp<Retval> retval;
        struct fuse_out_header out;
        struct iovec iov[2];
        int count = 1;

        if (error <= -1000 || error > 0)
            error = -ERANGE;

        out.unique = unique;
        out.error = error;

        iov[0].iov_base = &out;
        iov[0].iov_len = sizeof(struct fuse_out_header);

        if(size){
            iov[1].iov_base = (void*) arg;
            iov[1].iov_len = size;
            count++;
        }

        out.len = iov_length(iov, count);

        {
            AutoLock _l(&FuseMount::SAFE_LOCK);
            return DFW_RET_C(retval, send(iov, count));
        }
    }

    sp<Retval> FuseMount::read(size_t *out, char* buf, size_t size){
        sp<Retval> retval;

        if(DFW_RET(retval, Mount::read(out, buf, size)))
            return DFW_RETVAL_D(retval);

        size_t realsize = sizeof(struct fuse_in_header);
        if((size_t) *out < realsize)
            return DFW_RETVAL_NEW_MSG(DFW_E_IO, EIO
                        , "fd=%d, read-size:%ld, real-size:%ld"
                          " short read on fuse device"
                        , getHandle(), *out, realsize);
        return NULL;
    }

};
#endif

