#ifndef DFRAMEWORK_FSCORE_FUSEBASEWORKER_H
#define DFRAMEWORK_FSCORE_FUSEBASEWORKER_H

#include <dframework/base/Retval.h>
#include <dframework/base/Thread.h>
#include <dframework/fscore/FsNode.h>
#include <dframework/fscore/FuseMount.h>
#include <dframework/fscore/FuseRequest.h>
#include <dframework/fscore/FsContext.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseBaseWorker : public Thread
    {
    protected:
      sp<FuseMount>       m_mount;
      sp<FuseRequestList> m_reqlist;
      sp<FsNodeList>      m_safe_nl;
      sp<FsContext>       m_context;
      bool                m_bExited;

      virtual sp<Retval> runWorker();
      virtual sp<Retval> process(sp<FuseRequest>& req);
      virtual sp<Retval> process_real(sp<FuseRequest>& req);

    public:
      FuseBaseWorker();
      virtual ~FuseBaseWorker();

      virtual void run();

      virtual bool isExited();

      virtual sp<Retval> mount(const char* mountpoint);
      virtual sp<Retval> mount(const String& mountpoint);

      virtual sp<Retval> start(const char* mountpoint);
      virtual sp<Retval> start(const String& mountpoint);
      virtual sp<Retval> start();

      virtual sp<Retval> do_base_init(sp<FuseRequest>& req);

      inline void setRequestList(sp<FuseRequestList>& l){ m_reqlist = l; }
      inline void setMount(sp<FuseMount>& mount){ m_mount = mount; }
      inline void setNodeList(sp<FsNodeList>& l){ m_safe_nl = l; }
      inline void setContext(sp<FsContext>& c){ m_context = c; }

      inline bool isMounted() const {
          return m_mount.has() ? m_mount->isMounted() : false;
      }
      inline const char* mountpoint() const {
          return (m_mount.has() 
                 ? (m_mount->getMountPoint().empty() 
                   ? "" : m_mount->getMountPoint().toChars())
                 : "");
      }

      inline virtual void on_exit(sp<Retval>& retval){ DFW_UNUSED(retval); }

      // ------------------------------------------------------------

      inline virtual sp<Retval> do_forget(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return NULL;
      }
      inline virtual sp<Retval> do_interrupt(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return NULL;
      }
      inline virtual sp<Retval> do_batchforget(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return NULL;
      }

      inline virtual sp<Retval> do_init(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_getattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_opendir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_readdir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_releasedir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_lookup(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_open(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_read(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_write(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_release(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_setattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_readlink(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_symlink(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_mknod(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_mkdir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_unlink(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_rmdir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_rename(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_link(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_statfs(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_fsync(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_setxattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_getxattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_listxattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_removexattr(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_flush(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_fsyncdir(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_getlk(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_setlk(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_setlkw(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_access(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_create(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_bmap(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_destroy(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_ioctl(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_poll(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }
      inline virtual sp<Retval> do_notifyreply(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

      inline virtual sp<Retval> do_fallocate(sp<FuseRequest>& req){
          DFW_UNUSED(req);
          return req->send_error(-ENOSYS); }

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEBASEWORKER_H */

