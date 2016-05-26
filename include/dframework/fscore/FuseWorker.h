#ifndef DFRAMEWORK_FSCORE_FUSEWORKER_H
#define DFRAMEWORK_FSCORE_FUSEWORKER_H

#include <dframework/fscore/FsNode.h>
#include <dframework/fscore/FuseBaseWorker.h>
#include <dframework/fscore/FuseDirCtx.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseWorker : public FuseBaseWorker
    {
    public:
        static const int LOG_LEVEL_V = 1;
        static const int LOG_LEVEL_E = 2;

    protected:
      char                 m_opcodes[FUSE::OP_MAX];

    public:
      FuseWorker();
      virtual ~FuseWorker();

      inline bool set_enable(uint32_t opcode){
        return ((opcode <= FUSE::OP_MAX) ? (m_opcodes[opcode] = true) : false);
      }

      inline bool is_enable(uint32_t opcode){
        return ( (opcode <= FUSE::OP_MAX) ? m_opcodes[opcode] : false );
      }

      // ------------------------------------------------------------

      inline virtual void on_logging(sp<FuseRequest>& /*req*/
                                   , sp<Retval>& /*retval*/, int /*level*/){
      }

      // ------------------------------------------------------------

      virtual sp<Retval> do_init(sp<FuseRequest>& req);
      virtual sp<Retval> do_getattr(sp<FuseRequest>& req);
      virtual sp<Retval> do_opendir(sp<FuseRequest>& req);
      virtual sp<Retval> do_readdir(sp<FuseRequest>& req);
      virtual sp<Retval> do_releasedir(sp<FuseRequest>& req);
      virtual sp<Retval> do_lookup(sp<FuseRequest>& req);
      virtual sp<Retval> do_open(sp<FuseRequest>& req);
      virtual sp<Retval> do_flush(sp<FuseRequest>& req);
      virtual sp<Retval> do_read(sp<FuseRequest>& req);
      virtual sp<Retval> do_write(sp<FuseRequest>& req);
      virtual sp<Retval> do_release(sp<FuseRequest>& req);

      virtual sp<Retval> do_forget(sp<FuseRequest>& req);
      virtual sp<Retval> do_interrupt(sp<FuseRequest>& req);
      virtual sp<Retval> do_batchforget(sp<FuseRequest>& req);

      // ------------------------------------------------------------

      inline virtual sp<Retval> on_lookup(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, struct stat* /*st*/) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_getattr(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, struct stat* /*st*/) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_opendir(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseDirCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_readdir(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseDirCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_releasedir(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseDirCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_open(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseOpenCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_flush(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseOpenCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_read(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/
                            , unsigned* /*out_size*/
                            , char* /*buf*/
                            , uint32_t /*size*/
                            , uint64_t /*offset*/, sp<FuseOpenCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_release(sp<FuseRequest>& /*req*/
                            , const char* /*fullpath*/, sp<FuseOpenCtx>&) {
          return DFW_RETVAL_NEW(DFW_ERROR, -ENOSYS);
      }

      inline virtual sp<Retval> on_init(sp<FuseRequest>& /*req*/) {
          return NULL;
      }

      inline virtual sp<Retval> on_forget(sp<FuseRequest>& /*req*/
                                , uint64_t /*nodeid*/, uint64_t /*nlookup*/) {
          return NULL;
      }

      inline virtual sp<Retval> on_batchforget(sp<FuseRequest>& /*req*/) {
          return NULL;
      }

      inline virtual sp<Retval> on_interrupt(sp<FuseRequest>& /*req*/
                                           , uint64_t /*target_unique*/) {
          return NULL;
      }

    };

}; // end namespace dframework
#endif // end if cpluscplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEWORKER_H */

