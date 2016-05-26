#ifndef _WIN32
#include <dframework/fscore/FuseBaseWorker.h>
#include <dframework/log/Logger.h>

namespace dframework {

    FuseBaseWorker::FuseBaseWorker(){
        m_bExited = false;
    }

    FuseBaseWorker::~FuseBaseWorker(){
    }

    bool FuseBaseWorker::isExited() {
          return (m_bExited 
                   ? true 
                   : (m_mount.has() ? m_mount->isExited() : true));
    }

    sp<Retval> FuseBaseWorker::mount(const char* mountpoint){
        sp<Retval> retval;
        if( !m_mount.has() )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                , "Not find FuseMount. you call setMount(...) "
                  "function.");
        if( DFW_RET(retval, m_mount->mount(mountpoint)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> FuseBaseWorker::mount(const String& mountpoint){
        sp<Retval> retval;
        return DFW_RET_C(retval, mount(mountpoint.toChars()));
    }

    sp<Retval> FuseBaseWorker::start(const char* mountpoint){
        sp<Retval> retval;
        if( DFW_RET(retval, mount(mountpoint)) )
            return DFW_RETVAL_D(retval);
        return DFW_RET_C(retval, start());
    }

    sp<Retval> FuseBaseWorker::start(const String& mountpoint){
        sp<Retval> retval;
        return DFW_RET_C(retval, start(mountpoint.toChars()));
    }

    sp<Retval> FuseBaseWorker::start(){
        sp<Retval> retval;
        if( !m_mount.has() )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0
                , "Not find FuseMount. you call setMount(...) "
                  "function.");
        if( !isMounted() )
            return DFW_RETVAL_NEW_MSG(DFW_E_ISCONN, 0
                , "mount-point=%s, Not mounted.", mountpoint());

        if( !m_reqlist.has() )
            return DFW_RETVAL_NEW_MSG(DFW_E_ISCONN, 0
                , "mount-point=%s, Not have FuseRequestList."
                , mountpoint());

        m_bExited = false;

        return DFW_RET_C(retval, Thread::start());
    }

    void FuseBaseWorker::run(){
        sp<Retval> retval = runWorker();
        on_exit(retval);
    }

    sp<Retval> FuseBaseWorker::runWorker(){
        sp<Retval> retval;

        size_t outsize = 0;
        char buf[204800];
        size_t size = 102400+51200;

        while(true){
            if( isExited() )
                return DFW_RETVAL_NEW_MSG(DFW_E_EXIST, 0
                    , "mount-point=%s, Exited.", mountpoint());

            outsize = 0;
            if(DFW_RET(retval, m_mount->read(&outsize, buf, size))){
                return DFW_RETVAL_D(retval);
            }

            sp<FuseRequest> req = new FuseRequest(m_mount, m_safe_nl);
            req->m_context = m_context;
            if( DFW_RET(retval, req->set(buf, outsize)) )
                return DFW_RETVAL_D(retval);

            m_mount->setUnique(req->unique(), req->opcode());

            if( m_mount->isInit() && FUSE::OP_INIT == req->opcode() ){
                return DFW_RETVAL_NEW_MSG(DFW_E_IO, 0
                   , "mount-point=%s, opcode=%s(%d),"
                     " Already has OP_INIT."
                   , mountpoint()
                   , FUSE::getOpcodeName(req->opcode())
                   , req->opcode());
            }else if( !m_mount->isInit() ){
                if( FUSE::OP_INIT != req->opcode() ){
                    return DFW_RETVAL_NEW_MSG(DFW_E_IO, 0
                      , "mount-point=%s, opcode=%s(%d),"
                        " expected OP_INIT."
                      , mountpoint()
                      , FUSE::getOpcodeName(req->opcode())
                      , req->opcode());
                }
                if( DFW_RET(retval, process(req)) )
                    return DFW_RETVAL_D(retval);
                if( DFW_RET(retval, do_base_init(req)) )
                    return DFW_RETVAL_D(retval);
            }else if( DFW_RET(retval, process(req)) ){
                //FIXME:
                //return DFW_RETVAL_D(retval);
            }
        } // end while(true)
    }

    sp<Retval> FuseBaseWorker::process(sp<FuseRequest>& req)
    {
        sp<Retval> retval;

        DFWLOG(DFWLOG_DEBUG|DFWLOG_ID(DFWLOG_FSCORE_ID)
            , "fscore-process=%s, opcode=%u, unique=%lu"
              ", uid=%u, gid=%u, pid=%u"
            , FUSE::getOpcodeName(req->opcode())
            , req->opcode(), req->unique()
            , req->uid(), req->gid(), req->pid()
        );

        if( DFW_RET(retval, m_reqlist->insertRequest(req)) ){
            return DFW_RETVAL_D(retval);
        }

        retval = process_real(req);

        m_reqlist->removeRequest(req);

        if(retval.has())
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> FuseBaseWorker::process_real(sp<FuseRequest>& req)
    {
        sp<Retval> retval;
        int opcode = req->opcode();
        switch(opcode){
        case FUSE::OP_LOOKUP :
            return DFW_RET_C(retval, do_lookup(req));
        case FUSE::OP_FORGET :
            return DFW_RET_C(retval, do_forget(req));
        case FUSE::OP_GETATTR :
            return DFW_RET_C(retval, do_getattr(req));

        case FUSE::OP_SETATTR :
            return DFW_RET_C(retval, do_setattr(req));
        case FUSE::OP_READLINK :
            return DFW_RET_C(retval, do_readlink(req));
        case FUSE::OP_SYMLINK :
            return DFW_RET_C(retval, do_symlink(req));

        case FUSE::OP_MKNOD :
            return DFW_RET_C(retval, do_mknod(req));
        case FUSE::OP_MKDIR :
            return DFW_RET_C(retval, do_mkdir(req));
        case FUSE::OP_UNLINK :
            return DFW_RET_C(retval, do_unlink(req));
        case FUSE::OP_RMDIR :
            return DFW_RET_C(retval, do_rmdir(req));
        case FUSE::OP_RENAME :
            return DFW_RET_C(retval, do_rename(req));
        case FUSE::OP_LINK :
            return DFW_RET_C(retval, do_link(req));

        case FUSE::OP_OPEN :
            return DFW_RET_C(retval, do_open(req));
        case FUSE::OP_READ :
            return DFW_RET_C(retval, do_read(req));
        case FUSE::OP_WRITE :
            return DFW_RET_C(retval, do_write(req));
        case FUSE::OP_STATFS :
            return DFW_RET_C(retval, do_statfs(req));
        case FUSE::OP_RELEASE :
            return DFW_RET_C(retval, do_release(req));

        case FUSE::OP_FSYNC :
            return DFW_RET_C(retval, do_fsync(req));
        case FUSE::OP_SETXATTR :
            return DFW_RET_C(retval, do_setxattr(req));
        case FUSE::OP_GETXATTR :
            return DFW_RET_C(retval, do_getxattr(req));
        case FUSE::OP_LISTXATTR :
            return DFW_RET_C(retval, do_listxattr(req));
        case FUSE::OP_REMOVEXATTR :
            return DFW_RET_C(retval, do_removexattr(req));

        case FUSE::OP_FLUSH :
            return DFW_RET_C(retval, do_flush(req));
        case FUSE::OP_INIT :
            return DFW_RET_C(retval, do_init(req));
        case FUSE::OP_OPENDIR :
            return DFW_RET_C(retval, do_opendir(req));
        case FUSE::OP_READDIR :
            return DFW_RET_C(retval, do_readdir(req));
        case FUSE::OP_RELEASEDIR :
            return DFW_RET_C(retval, do_releasedir(req));

        case FUSE::OP_FSYNCDIR :
            return DFW_RET_C(retval, do_fsyncdir(req));
        case FUSE::OP_GETLK :
            return DFW_RET_C(retval, do_getlk(req));
        case FUSE::OP_SETLK :
            return DFW_RET_C(retval, do_setlk(req));
        case FUSE::OP_SETLKW :
            return DFW_RET_C(retval, do_setlkw(req));
        case FUSE::OP_ACCESS :
            return DFW_RET_C(retval, do_access(req));
        case FUSE::OP_CREATE :
            return DFW_RET_C(retval, do_create(req));

        case FUSE::OP_INTERRUPT :
            return DFW_RET_C(retval, do_interrupt(req));

        case FUSE::OP_BMAP :
            return DFW_RET_C(retval, do_bmap(req));
        case FUSE::OP_DESTROY :
            return DFW_RET_C(retval, do_destroy(req));
        case FUSE::OP_IOCTL :
            return DFW_RET_C(retval, do_ioctl(req));
        case FUSE::OP_POLL :
            return DFW_RET_C(retval, do_poll(req));
        case FUSE::OP_NOTIFY_REPLY : // FIXME:
            return DFW_RET_C(retval, do_notifyreply(req));

        case FUSE::OP_BATCH_FORGET : // FIXME:
            return DFW_RET_C(retval, do_batchforget(req));

        case FUSE::OP_FALLOCATE :
            return DFW_RET_C(retval, do_fallocate(req));

        case FUSE::OP_CUSE_INIT :
        default :
            req->send_error(-ENOSYS);
            return DFW_RETVAL_NEW_MSG(DFW_E_IO, EIO
                     , "mount-point=%s, opcode=%s(%d), "
                       "Not support opcode."
                     , req->mountpoint()
                     , FUSE::getOpcodeName(opcode)
                     , opcode);
        }

        return NULL;
    }

    sp<Retval> FuseBaseWorker::do_base_init(sp<FuseRequest>& req)
    {
        sp<Retval> retval;

        struct fuse_init_in* arg = (struct fuse_init_in*)req->arg();
        struct fuse_init_out outarg;
        //size_t bufsize = req->bufsize();
        struct conn_info* conn = &m_mount->m_conn;

#define MIN_BUFSIZE 0x21000
        size_t bufsize = getpagesize() + 0x1000;
        bufsize = bufsize < MIN_BUFSIZE ? MIN_BUFSIZE : bufsize;

        conn->proto_major = arg->major;
        conn->proto_minor = arg->minor;
        conn->capable = 0;
        conn->want = 0;

        ::memset(&outarg, 0, sizeof(outarg));
        outarg.major = FUSE_KERNEL_VERSION;
        outarg.minor = FUSE_KERNEL_MINOR_VERSION;

        if(arg->major < 7) {
            return DFW_RETVAL_NEW_MSG(DFW_E_PROTO, EPROTO
               , "fuse: unsupported protocol version: %u.%u"
               , arg->major, arg->minor);
        }

        if(arg->major > 7) {
            /* Wait for a second INIT request with a 7.X version */
            m_mount->setInit(true);
            return DFW_RET_C(retval,req->send_ok(&outarg,sizeof(outarg)));
        }

        if(arg->minor >= 6){
            if (conn->async_read)
                conn->async_read = arg->flags & FUSE_ASYNC_READ;
            if (arg->max_readahead < conn->max_readahead)
                conn->max_readahead = arg->max_readahead;
            if (arg->flags & FUSE_ASYNC_READ)     // defaults
                conn->capable |= FUSE_CAP_ASYNC_READ;
            if (arg->flags & FUSE_POSIX_LOCKS)    // defaults
                conn->capable |= FUSE_CAP_POSIX_LOCKS;
            if (arg->flags & FUSE_ATOMIC_O_TRUNC) // defaults
                conn->capable |= FUSE_CAP_ATOMIC_O_TRUNC;
            if (arg->flags & FUSE_EXPORT_SUPPORT) // defaults
                conn->capable |= FUSE_CAP_EXPORT_SUPPORT;
            if (arg->flags & FUSE_BIG_WRITES)     // defaults
                conn->capable |= FUSE_CAP_BIG_WRITES;
            if (arg->flags & FUSE_DONT_MASK)      // defaults
                conn->capable |= FUSE_CAP_DONT_MASK;
            if (arg->flags & FUSE_FLOCK_LOCKS)    // defaults
                conn->capable |= FUSE_CAP_FLOCK_LOCKS;
        } else {
            conn->async_read = 0;
            conn->max_readahead = 0;
        }

#if 1
#define HAVE_SPLICE
#define HAVE_VMSPLICE
        if (conn->proto_minor >= 14) {
#ifdef HAVE_SPLICE
#ifdef HAVE_VMSPLICE
            conn->capable |= FUSE_CAP_SPLICE_WRITE | FUSE_CAP_SPLICE_MOVE;
            // FIXME:
            //if (f->splice_write)
            //    conn->want |= FUSE_CAP_SPLICE_WRITE;
            //if (f->splice_move)
            //    conn->want |= FUSE_CAP_SPLICE_MOVE;
#endif
            conn->capable |= FUSE_CAP_SPLICE_READ;
            // FIXME:
            //if (f->splice_read)
            //    conn->want |= FUSE_CAP_SPLICE_READ;
#endif
        }
#endif

        if(conn->proto_minor >= 18)
            conn->capable |= FUSE_CAP_IOCTL_DIR;

#if 0
        // FIXME:
        if (f->atomic_o_trunc)
            conn->want |= FUSE_CAP_ATOMIC_O_TRUNC;
        if (f->op.getlk && f->op.setlk && !f->no_remote_posix_lock)
            conn->want |= FUSE_CAP_POSIX_LOCKS;
        if (f->op.flock && !f->no_remote_flock)
            conn->want |= FUSE_CAP_FLOCK_LOCKS;
        // FIXME:
        if (f->big_writes)
            conn->want |= FUSE_CAP_BIG_WRITES;
#endif

        if (bufsize < FUSE_MIN_READ_BUFFER) {
            bufsize = FUSE_MIN_READ_BUFFER;
        }

        bufsize -= 4096;
        if (bufsize < conn->max_write)
                conn->max_write = bufsize;

#if 0
        // FIXME:
        if (f->no_splice_read)
                f->conn.want &= ~FUSE_CAP_SPLICE_READ;
        if (f->no_splice_write)
                f->conn.want &= ~FUSE_CAP_SPLICE_WRITE;
        if (f->no_splice_move)
                f->conn.want &= ~FUSE_CAP_SPLICE_MOVE;
#endif
                conn->want |= FUSE_CAP_SPLICE_READ;
                conn->want |= FUSE_CAP_SPLICE_WRITE;
                conn->want |= FUSE_CAP_SPLICE_MOVE;

        //////////////////////////////////////////////////
        // This is fuse_lib_init at fuse.c in fuse-2.9.4
        conn->want |= FUSE_CAP_EXPORT_SUPPORT;
        //
        //////////////////////////////////////////////////

#if 0
        //////////////////////////////////////////////////
        // This is fuse_fs_init at fuse.c in fuse-2.9.4
        //if (!fs->op.write_buf)
                conn->want &= ~FUSE_CAP_SPLICE_READ;
        //if (!fs->op.lock)
                conn->want &= ~FUSE_CAP_POSIX_LOCKS;
        //if (!fs->op.flock)
                conn->want &= ~FUSE_CAP_FLOCK_LOCKS;
        //
        //////////////////////////////////////////////////
#endif

        if(conn->async_read || (conn->want & FUSE_CAP_ASYNC_READ)) // default
                outarg.flags |= FUSE_ASYNC_READ;
        if(conn->want & FUSE_CAP_POSIX_LOCKS)
                outarg.flags |= FUSE_POSIX_LOCKS;
        if(conn->want & FUSE_CAP_ATOMIC_O_TRUNC)
                outarg.flags |= FUSE_ATOMIC_O_TRUNC;
        if(conn->want & FUSE_CAP_EXPORT_SUPPORT) // default
                outarg.flags |= FUSE_EXPORT_SUPPORT;
        if(conn->want & FUSE_CAP_BIG_WRITES)
                outarg.flags |= FUSE_BIG_WRITES;
        if(conn->want & FUSE_CAP_DONT_MASK)
                outarg.flags |= FUSE_DONT_MASK;
        if(conn->want & FUSE_CAP_FLOCK_LOCKS)
                outarg.flags |= FUSE_FLOCK_LOCKS;

        outarg.max_readahead = conn->max_readahead;
        outarg.max_write = conn->max_write;

        if(conn->proto_minor >= 13) {
            if (conn->max_background >= (1 << 16))
                conn->max_background = (1 << 16) - 1;
            if (conn->congestion_threshold > conn->max_background)
                conn->congestion_threshold = conn->max_background;
            if (!conn->congestion_threshold)
                conn->congestion_threshold = conn->max_background * 3 / 4;

            outarg.max_background = conn->max_background;
            outarg.congestion_threshold = conn->congestion_threshold;
        }

        m_mount->setInit(true);
        return DFW_RET_C(retval, req->send_ok(
                     &outarg, (arg->minor < 5 ? 8 : sizeof(outarg))));
    }

};
#endif

