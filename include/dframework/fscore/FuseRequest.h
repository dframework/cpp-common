#ifndef DFRAMEWORK_FSCORE_FUSEREQUEST_H
#define DFRAMEWORK_FSCORE_FUSEREQUEST_H

#include <dframework/base/Retval.h>
#include <dframework/fscore/FsNode.h>
#include <dframework/fscore/Fuse_d.h>
#include <dframework/fscore/FuseMount.h>
#include <dframework/fscore/FsContext.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {


    class FuseRequest : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(FuseRequest);

    private:
      char*                  m_buf;
      size_t                 m_bufsize;
      uint64_t               m_unique;
      bool                   m_bInterrupt;
      bool                   m_bInterrupted;

    public:
      sp<FuseMount>          m_mount;
      sp<FsNodeList>         m_nl;
      sp<FsContext>          m_context;
      struct fuse_in_header* m_head;
      void*                  m_arg;
      size_t                 m_argsize;
        
    public:
      FuseRequest(uint64_t unique);
      FuseRequest(sp<FuseMount>& mount, sp<FsNodeList>& nl);
      virtual ~FuseRequest();

      sp<Retval> set(const char* buf, size_t size);

      void setInterrupt();
      void setInterrupted();
      bool getInterrupt();
      bool getInterrupted();

      inline uint32_t len()     { return m_head->len; }
      inline uint32_t opcode()  { return m_head->opcode; }
      inline uint64_t unique()  { return m_head->unique; }
      inline uint64_t nodeid()  { return m_head->nodeid; }
      inline uint32_t uid()     { return m_head->uid; }
      inline uint32_t gid()     { return m_head->gid; }
      inline uint32_t pid()     { return m_head->pid; }
      inline uint32_t padding() { return m_head->padding; }
      inline size_t   bufsize() { return m_bufsize; }
      inline size_t   argsize() { return m_argsize; }
      inline void*    arg()     { return m_arg; }

      inline unsigned proto_major() { return m_mount->proto_major(); }
      inline unsigned proto_minor() { return m_mount->proto_minor(); }
      inline unsigned async_read() { return m_mount->async_read(); }
      inline unsigned max_write() { return m_mount->max_write(); }
      inline unsigned max_readahead() { return m_mount->max_readahead(); }
      inline unsigned capable() { return m_mount->capable(); }
      inline unsigned want() { return m_mount->want(); }
      inline unsigned max_background() { return m_mount->max_background(); }
      inline unsigned congestion_threshold() { 
          return m_mount->congestion_threshold(); 
      }
      inline const char* mountpoint() { 
          return (m_mount.has() 
                  ? (!m_mount->getMountPoint().empty() 
                    ? m_mount->getMountPoint().toChars() : "")
                  : "");
      }

      virtual sp<Retval> send(uint64_t unique, int error
                            , const dfw_point_t arg, size_t argsize);
      inline virtual sp<Retval> send_error(
            int error, void* buf, size_t size){
        return send(unique(), error, buf, size);
      }
      inline virtual sp<Retval> send_error(int error){
        return send(unique(), error, NULL, 0);
      }
      inline virtual sp<Retval> send_ok(void* buf, size_t size){
        return send(unique(), 0, buf, size);
      }
      inline virtual sp<Retval> send_ok(){
        return send(unique(), 0, NULL, 0);
      }
      virtual sp<Retval> send_entry(struct fuse_entry_param* entry);

      // ------------------------------------------------------------
      // F s C o n t e x t
      // ------------------------------------------------------------

      int contextSize();

      sp<Retval> insertContext(sp<NamedObject>& ctx);

      sp<NamedObject> removeContext(sp<NamedObject>& ctx);

      sp<NamedObject> getContext(sp<NamedObject>& ctx);

      sp<NamedObject> getContext(int position);

      sp<Object> getContext(const String& named);

      sp<Object> getContext(const char* named);

      // ------------------------------------------------------------

      DFW_OPERATOR_EX_DECLARATION(FuseRequest, m_unique);

    };

    class FuseRequestList : public Object
    {
    private:
        ArraySorted<FuseRequest> m_aList;

    public:
        FuseRequestList();
        virtual ~FuseRequestList();

        int size();

        sp<Retval> insertRequest(sp<FuseRequest>& req);
        sp<FuseRequest> removeRequest(sp<FuseRequest>& req);
        sp<FuseRequest> removeRequest(uint64_t unique);

        sp<FuseRequest> getRequest(sp<FuseRequest>& req);
        sp<FuseRequest> getRequest(uint64_t unique);

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEREQUEST_H */

