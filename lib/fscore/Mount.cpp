#ifndef _WIN32

#include <dframework/fscore/Mount.h>
#include <dframework/io/Dir.h>

#include <fcntl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>

namespace dframework {

    Mount::Mount(){
        init();
    }

    Mount::~Mount(){
        umount();
        DFW_FILE_CLOSE(m_iHandle);
    }

    sp<Retval> Mount::umount(){
        sp<Retval> retval;
        if(m_bMounted){
            if( DFW_RET(retval, Mount::umount(m_sMountPoint.toChars())) )
                return DFW_RETVAL_D(retval);
            m_bMounted = false;
        }
        return NULL;
    }

    sp<Retval> Mount::umount(const char* mountpoint){
#ifdef __APPLE__
        if( -1 == ::unmount(mountpoint, MNT_FORCE)){
#else
        if( -1 == ::umount2(mountpoint, MNT_FORCE)){
#endif
            int eno = errno;
            switch(eno){
            case EAGAIN: return DFW_RETVAL_NEW(DFW_E_AGAIN, eno);
            case EFAULT: return DFW_RETVAL_NEW(DFW_E_FAULT, eno);
            case EINVAL: return DFW_RETVAL_NEW(DFW_E_INVAL, eno);
            case ENAMETOOLONG: 
                return DFW_RETVAL_NEW(DFW_E_NAMETOOLONG, eno);
            case ENOENT: return DFW_RETVAL_NEW(DFW_E_NOENT, eno);
            case ENOMEM: return DFW_RETVAL_NEW(DFW_E_NOMEM, eno);
            case EPERM: return DFW_RETVAL_NEW(DFW_E_PERM, eno);
            }
            return DFW_RETVAL_NEW(DFW_E_UMOUNT2, eno);
        }
        return NULL;
    }

    sp<Retval> Mount::umount(const String& mountpoint){
        sp<Retval> retval;
        return DFW_RET_C(retval, Mount::umount(mountpoint.toChars()));
    }

    sp<Retval> Mount::read(size_t* outsize, char *buf, size_t size){
        //AutoLock _l(m_read_lock);
        sp<Retval> retval;
        int eno;

        if(!outsize) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
        do{
          *outsize = ::read(m_iHandle, buf, size);
          eno = errno;
          if(isExited())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0 
                      , "mount-point=%s, This is exited."
                      , m_sMountPoint.toChars());

          if( ((size_t)-1) == *outsize ) {
            /* ENOENT means the operation was interrupted, it's safe
               to restart */
            if(ENOENT==eno) {
                printf("enoent ....\n");
                continue;
            }
            /* Errors occurring during normal operation: EINTR (read
               interrupted), EAGAIN (nonblocking I/O), ENODEV (filesystem
               umounted) */
            dfw_retno_t retno;
            switch(eno){
            case EINTR : retno = DFW_E_INTR; break;
            case EAGAIN: retno = DFW_E_AGAIN; break;
            case ENODEV: 
                setExited();
                retno = DFW_E_NODEV; break;
            case EBADF:  retno = DFW_E_BADF; break;
            case EFAULT: retno = DFW_E_FAULT; break;
            case EINVAL: retno = DFW_E_INVAL; break;
            case EIO:    retno = DFW_E_IO; break;
            case EISDIR: retno = DFW_E_ISDIR; break;
            default:     retno = DFW_E_READ; break;
            }
            return DFW_RETVAL_NEW_MSG(retno, eno
                      , "Not reading device(%s)"
                      , m_sDeviceName.toChars());
          }
          return NULL;
        }while(true);
    }

    void Mount::init(){
        DFW_FILE_INIT(m_iHandle);
        m_bMounted = false;
        m_bExited = false;

        m_bAllowOther = false;
        m_bAllowRoot = false;
        m_bNonEmpty = false;
        m_bAutoUnmount = false;
        m_bBlkDev = false;
        m_iFlags = 0;
    }

    void Mount::addDefaultFlags(){
#ifndef __APPLE__
        addFlag(MS_NOSUID | MS_NODEV);
#endif
    }

    sp<Retval> Mount::mount(const char* device, const char* mountpoint){
        sp<Retval> retval;
        m_sDeviceName = device;
        m_sMountPoint = mountpoint;
        return DFW_RET_C(retval, mount());
    }

    sp<Retval> Mount::mount(const String& device, const String& mountpoint){
        sp<Retval> retval;
        m_sDeviceName = device;
        m_sMountPoint = mountpoint;
        return DFW_RET_C(retval, Mount::mount());
    }

    sp<Retval> Mount::mount(){
        sp<Retval> retval;
        if(m_sDeviceName.empty())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                   , "device name is empty.");
        if(m_sMountPoint.empty())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                   , "mount point is empty.");
        if( m_bAllowOther && m_bAllowRoot )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                   , "'allow_other' and 'allow_root' options are mutually"
                     " exclusive");

        Stat st;
        if(DFW_RET(retval, st.stat(m_sMountPoint))){
            if(DFW_E_ISCONN==DFW_RETVAL_V(retval)){
                return DFW_RETVAL_NEW_MSG(DFW_E_ISCONN, retval->error()
                           , "mount-point=%s, "
                             "Transport endpoint is already connected"
                           , m_sMountPoint.toChars());
            }else if(DFW_E_NOTCONN==DFW_RETVAL_V(retval)){
                // FIXME: Transport endpoint is not connected
                return DFW_RETVAL_NEW_MSG(DFW_E_ISCONN, retval->error()
                           , "mount-point=%s, "
                             "Transport endpoint is not connected"
                           , m_sMountPoint.toChars());
            }
            return DFW_RETVAL_D(retval);
        }

        if( !m_bNonEmpty && DFW_RET(retval, checkEmptyMountPoint(st)) )
            return DFW_RETVAL_D(retval);

        addDefaultFlags();

        return DFW_RET_C(retval, mount_real(st));
    }

    sp<Retval> Mount::mount_real(const Stat& st){
        sp<Retval> retval;

        if(DFW_RET(retval, mount_open()))
            return DFW_RETVAL_D(retval);

        m_sKernelOpts = String::format(
                 "fd=%i,rootmode=%o,user_id=%u,group_id=%u,allow_other"
                , m_iHandle
                , st.m_stat.st_mode & Stat::D_IFMT
                , getuid(), getgid());

        //String sType = (m_bBlkDev ? "fuseblk" : "fuse");
        String sType = String::format("%s.%s"
                             , (m_bBlkDev ? "fuseblk" : "fuse")
                             , "zonevd"
                );
        if(!m_sSubtype.empty())
            sType.appendFmt(".%s", m_sSubtype.toChars());

        String sSource = (!m_sFsname.empty() 
                         ? m_sFsname : (!m_sSubtype.empty() 
                         ? m_sSubtype : m_sDeviceName));

#if 0
printf("mount source: %s\nmp: %s\ntype: %s\nflags: %d\nkernel-opts: %s\n"
, sSource.toChars()
, m_sMountPoint.toChars()
, sType.toChars()
, m_iFlags
, m_sKernelOpts.toChars());
#endif
        
#ifdef __APPLE__
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not supported apple.");
#else
        int res;
        int eno = 0;
        if( -1 == (res = ::mount(
                      sSource.toChars()
                    , m_sMountPoint.toChars()
                    , sType.toChars()
                    , m_iFlags
                    , m_sKernelOpts.toChars())))
        {
            eno = errno;
            if( eno==ENODEV && !m_sSubtype.empty() ){
                /* Probably missing subtype support */
                sType = (m_bBlkDev ? "fuseblk" : "fuse");
                if(!m_sFsname.empty()){
                    if(!m_bBlkDev){
                        sSource = String::format("%s#%s"
                                     , m_sSubtype.toChars()
                                     , m_sFsname.toChars()
                        );
                    }
                }else{
                    sSource = sType;
                }
                if( -1 == (res = ::mount(
                      sSource.toChars()
                    , m_sMountPoint.toChars()
                    , sType.toChars()
                    , m_iFlags
                    , m_sKernelOpts.toChars()))){
                    eno = errno;
                }
            }
            if(-1 == res ){
            dfw_retno_t retno;
            switch(eno){
            case EPERM: 
                return DFW_RETVAL_NEW_MSG(DFW_E_PERM, eno
                   , "mount-point=%s, Permission deny."
                   , m_sMountPoint.toChars());
            case ENODEV: 
                if(m_bBlkDev && !check_fuseblk())
                    return DFW_RETVAL_NEW_MSG(DFW_E_NODEV, eno
                       , "mount-point=%s, 'fuseblk' support missing"
                       , m_sMountPoint.toChars());
                retno = DFW_E_NODEV;
                break;
            case EACCES: retno = DFW_E_ACCES; break;
            case EBUSY: retno = DFW_E_BUSY; break;
            case EFAULT: retno = DFW_E_FAULT; break;
            case EINVAL: retno = DFW_E_INVAL; break;
            case ELOOP: retno = DFW_E_LOOP; break;
            case EMFILE: retno = DFW_E_MFILE; break;
            case ENAMETOOLONG: retno = DFW_E_NAMETOOLONG; break;
            case ENOENT: retno = DFW_E_NOENT; break;
            case ENOMEM: retno = DFW_E_NOMEM; break;
            case ENOTBLK: retno = DFW_E_NOTBLK; break;
            case ENOTDIR: retno = DFW_E_NOTDIR; break;
            case ENXIO: retno = DFW_E_NXIO; break;
            default: retno = DFW_E_MOUNT; break;
            }
            return DFW_RETVAL_NEW_MSG(retno, eno
                   , "mount-point=%s, Don't mount."
                   , m_sMountPoint.toChars());
            }
        }
#endif
        m_bMounted = true;

        return NULL;
    }

    sp<Retval> Mount::mount_open(){
        do {
          if( -1 == (m_iHandle = ::open(m_sDeviceName.toChars(), O_RDWR)) ){
            int eno = errno;
            dfw_retno_t retno = DFW_E_OPEN;
            switch(eno){
            case EINTR: retno = DFW_E_INTR; break;
            case ENODEV : 
                return DFW_RETVAL_NEW_MSG(DFW_E_NODEV, eno
                   , "mount-point=%s, Device not found."
                     " try 'modprobe fuse' first."
                   , m_sMountPoint.toChars());
            case ENOENT : 
                return DFW_RETVAL_NEW_MSG(DFW_E_NOENT, eno
                   , "mount-point=%s, Device not found."
                     " try 'modprobe fuse' first."
                   , m_sMountPoint.toChars());
            case EACCES: retno = DFW_E_ACCES; break;
            case EEXIST: retno = DFW_E_EXIST; break;
            case EFAULT: retno = DFW_E_FAULT; break;
            case EFBIG : retno = DFW_E_FBIG; break;
            case EISDIR : retno = DFW_E_ISDIR; break;
            case ELOOP : retno = DFW_E_LOOP; break;
            case EMFILE : retno = DFW_E_MFILE; break;
            case ENAMETOOLONG : retno = DFW_E_NAMETOOLONG; break;
            case ENFILE : retno = DFW_E_NFILE; break;
            case ENOMEM : retno = DFW_E_NOMEM; break;
            case ENOSPC : retno = DFW_E_NOSPC; break;
            case ENOTDIR : retno = DFW_E_NOTDIR; break;
            case ENXIO : retno = DFW_E_NXIO; break;
            case EOVERFLOW : retno = DFW_E_OVERFLOW; break;
            case EPERM : retno = DFW_E_PERM; break;
            case EROFS : retno = DFW_E_ROFS; break;
            case ETXTBSY : retno = DFW_E_TXTBSY; break;
            case EWOULDBLOCK : retno = DFW_E_AGAIN; break;
            }
            return DFW_RETVAL_NEW_MSG(retno, eno
                   , "mount-point=%s, Not open mount point."
                   , m_sMountPoint.toChars());
          }
          return NULL;
        }while(true);
    }

    sp<Retval> Mount::checkEmptyMountPoint(const Stat& st){
        sp<Retval> retval;

        if(!st.isDir())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                   , "mount-point=%s, mount point is not empty."
                   , m_sMountPoint.toChars());

        String name;
        Dir dir;
        if(DFW_RET(retval, dir.open(m_sMountPoint.toChars())))
            return DFW_RETVAL_D(retval);
        while(true){
            if(DFW_RET(retval, dir.read(name))){
                if(DFW_T_COMPLETE==DFW_RETVAL_V(retval))
                    break;
                return DFW_RETVAL_D(retval);
            }
            if( name.equals(".") || name.equals("..") ) continue;
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                   , "mount-point=%s, mount point is not empty."
                   , m_sMountPoint.toChars());
        }
 
        return NULL;
    }

    int Mount::check_fuseblk(void)
    {
        char buf[256];
        FILE *f = ::fopen("/proc/filesystems", "r");
        if(!f) return 1;
        while(fgets(buf, sizeof(buf), f)){
            if(strstr(buf, "fuseblk\n")) {
                fclose(f);
                return 1;
            }
        }
        fclose(f);
        return 0;
    }


};
#endif

