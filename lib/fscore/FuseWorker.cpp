#ifndef _WIN32
#include <dframework/base/ThreadManager.h>
#include <dframework/fscore/FuseWorker.h>
#include <dframework/fscore/FuseUtil.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#define PARAM(inarg) (((char *)(inarg)) + sizeof(*(inarg)))

#define DFW_FUSEWORKER_E(retval,req,err)               \
    (!!(retval->error()                            \
    ? req->send_error(retval->error())             \
    : req->send_error(err)))                       \
    ?  DFW_RETVAL_D(retval) : DFW_RETVAL_D(retval)

namespace dframework {

    FuseWorker::FuseWorker(){
        ::memset(m_opcodes, 0, FUSE::OP_MAX);
    }

    FuseWorker::~FuseWorker(){
    }

    sp<Retval> FuseWorker::do_init(sp<FuseRequest>& req){
        sp<Retval> retval;

        req->m_nl->makeRootNode();

        if( is_enable(FUSE::OP_INIT) ){
            if( DFW_RET(retval, on_init(req)) )
                return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        return NULL;
    }

    sp<Retval> FuseWorker::do_getattr(sp<FuseRequest>& req){
        sp<Retval> retval;
        sp<FsNode> node;

        if( !is_enable(FUSE::OP_GETATTR) )
            return DFW_RET_C(retval, req->send_error(-ENOSYS));

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-ENOSYS);

        struct stat st;
        ::memset(&st, 0, sizeof(st));

        if(DFW_RET(retval, on_getattr(req, node->m_name.toChars(), &st))){
            return DFW_FUSEWORKER_E(retval,req,-ENOSYS);
        }

        struct fuse_attr_out arg;
        size_t size = req->proto_minor() < 9 ?
                FUSE_COMPAT_ATTR_OUT_SIZE : sizeof(arg);

        ::memset(&arg, 0, sizeof(arg));
        arg.attr_valid = 1;
        arg.attr_valid_nsec = 0;
        FuseUtil::statToFuse(&arg.attr, &st);

        req->m_nl->updateAttr(node);

        return DFW_RET_C(retval, req->send_ok(&arg, size));
    }

    sp<Retval> FuseWorker::do_opendir(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_open_in* in = (struct fuse_open_in*)req->arg();
        sp<FsNode> node;

        if(!is_enable(FUSE::OP_OPENDIR) && !is_enable(FUSE::OP_READDIR))
            return DFW_RET_C(retval, req->send_error(-ENOSYS));

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        struct fuse_open_out arg;
        ::memset(&arg, 0, sizeof(arg));

        sp<FuseDirCtx> ctx = new FuseDirCtx();
        ctx->m_flags = in->flags;

        if( is_enable(FUSE::OP_OPENDIR) &&
              DFW_RET(retval, on_opendir(req
                                       , node->m_name.toChars(), ctx))){
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        arg.fh = (uint64_t)sp<FuseDirCtx>::undocking(ctx.get());

        if( DFW_RET(retval, req->send_ok(&arg, sizeof(arg))) ){
            if( retval->error()==ENOENT ){
                if( is_enable(FUSE::OP_RELEASEDIR) )
                    on_releasedir(req, node->m_name.toChars(), ctx);
            }
            sp<FuseDirCtx>::docking(ctx.get());
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> FuseWorker::do_readdir(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_read_in* in = (struct fuse_read_in*)req->arg();
        sp<FsNode> node;

        if( !is_enable(FUSE::OP_READDIR) )
            return DFW_RET_C(retval, req->send_error(-ENOSYS));

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        sp<FuseDirCtx> ctx = (FuseDirCtx*)in->fh;
        if(!ctx.has()){
            retval = DFW_RETVAL_NEW_MSG(DFW_ERROR, -EIO
                          , "nodeid=%ld, Not find FsUserDir."
                          , req->nodeid());
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        if(in->size && !in->offset && !ctx->isFilled()){
          if(DFW_RET(retval, on_readdir(req, node->m_name.toChars(), ctx)))
              return DFW_FUSEWORKER_E(retval,req,-EIO);
          ctx->setFilled();
        }

        size_t size = in->size;
        if( in->offset >= ctx->length() ){
            size = 0;
        }else if( (in->offset+in->size) > ctx->length()){
            size = (size_t)(ctx->length() - in->offset);
        }

        if(DFW_RET(retval, req->send_ok(
                                   ctx->getBuffer((unsigned)in->offset), size))){
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> FuseWorker::do_releasedir(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_release_in* in 
                                = (struct fuse_release_in*)req->arg();
        sp<FsNode> node;

        sp<FuseDirCtx> ctx = (FuseDirCtx*) in->fh;
        if(in->fh)
            sp<FuseDirCtx>::docking((FuseDirCtx*)in->fh);

        if( !is_enable(FUSE::OP_RELEASEDIR) ) // TODO: No error.
            return DFW_RET_C(retval, req->send_error(0));

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        if(DFW_RET(retval, on_releasedir(req
                                       , node->m_name.toChars(), ctx))){
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        return DFW_RET_C(retval, req->send_error(0));
    }

    sp<Retval> FuseWorker::do_lookup(sp<FuseRequest>& req){
        sp<Retval> retval;
        sp<FsNode> node, parent;

        struct stat st;
        const char* name = (const char*)req->arg();

        if( DFW_RET(retval, req->m_nl->getNode(parent, req->nodeid())) )
            return DFW_RETVAL_D(retval);

        String sPath;
        if( parent->m_nodeid == 1 )
            sPath = String::format("/%s", name);
        else
            sPath = String::format("%s/%s", parent->m_name.toChars(), name);

        if( is_enable(FUSE::OP_LOOKUP) ){
            if(DFW_RET(retval, on_lookup(req, sPath.toChars(), &st))){
                return DFW_FUSEWORKER_E(retval,req,-EIO);
            }
        }else if( is_enable(FUSE::OP_GETATTR) ){
            if(DFW_RET(retval, on_getattr(req, sPath.toChars(), &st))){
                return DFW_FUSEWORKER_E(retval,req,-EIO);
            }
        }else{
            return DFW_RET_C(retval, req->send_error(-ENOSYS));
        }

        struct fuse_entry_param entry;
        ::memset(&entry, 0, sizeof(entry));
        entry.generation = req->m_nl->generation();
        entry.entry_timeout = 0.0;
        entry.attr_timeout  = 0.0;
        ::memcpy(&entry.attr, &st, sizeof(struct stat));

        if( DFW_RET(retval, req->m_nl->insertNode(node, sPath.toChars())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        entry.ino = node->m_nodeid;

        if( DFW_RET(retval, req->send_entry(&entry)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> FuseWorker::do_open(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_open_in* in = (struct fuse_open_in*)req->arg();
        sp<FsNode> node;

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        if( !is_enable(FUSE::OP_OPEN) 
                  && !is_enable(FUSE::OP_READ) ){
            retval = DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                      , "nodeid=%ld, Not enable open or read function."
                      , req->nodeid());
            return DFW_FUSEWORKER_E(retval,req,-ENOSYS);
        }

        struct fuse_open_out arg;
        ::memset(&arg, 0, sizeof(arg));

        sp<FuseOpenCtx> ctx = new FuseOpenCtx();
        ctx->m_flags = in->flags;

        if( is_enable(FUSE::OP_OPEN) 
                  && DFW_RET(retval, on_open(req
                                       , node->m_name.toChars(), ctx))){
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        arg.fh = (uint64_t)sp<FuseOpenCtx>::undocking(ctx.get());

        if( DFW_RET(retval, req->send_ok(&arg, sizeof(arg))) ){
            if( retval->error()==ENOENT ){
                if( is_enable(FUSE::OP_RELEASE) )
                    on_release(req, node->m_name.toChars(), ctx);
            }
            sp<FuseOpenCtx>::docking(ctx.get());
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> FuseWorker::do_flush(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_flush_in* in = (struct fuse_flush_in*)req->arg();
        sp<FsNode> node;

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        if( !is_enable(FUSE::OP_FLUSH) ){
            if( DFW_RET(retval, req->send_ok()) ){
                // FIXME:
            }
            return NULL;
        }

        sp<FuseOpenCtx> ctx = (FuseOpenCtx*)in->fh;
        if(!ctx.has()){
            retval = DFW_RETVAL_NEW_MSG(DFW_ERROR, -EIO
                          , "nodeid=%ld, Not find FuseOpenCtx."
                          , req->nodeid());
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        if(DFW_RET(retval, on_flush(req, node->m_name.toChars(), ctx))){
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        return NULL;
    }

    sp<Retval> FuseWorker::do_read(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_read_in* in = (struct fuse_read_in*)req->arg();
        sp<FsNode> node;

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        if( !is_enable(FUSE::OP_READ) ){
            return DFW_RET_C(retval, req->send_error(-ENOSYS));
        }

        char buf[in->size];
        sp<FuseOpenCtx> ctx = (FuseOpenCtx*)in->fh;
        if(!ctx.has()){
            retval = DFW_RETVAL_NEW_MSG(DFW_ERROR, -EIO
                          , "nodeid=%ld, Not find FuseOpenCtx."
                          , req->nodeid());
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        unsigned out_size = 0;
        if(DFW_RET(retval, on_read(req, node->m_name.toChars(), &out_size
                               , buf, in->size, in->offset, ctx))){
            return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        if(DFW_RET(retval, req->send_ok(buf, out_size)))
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> FuseWorker::do_write(sp<FuseRequest>& req){
        sp<Retval> retval;
        return DFW_RET_C(retval, req->send_error(-ENOSYS));
    }

    sp<Retval> FuseWorker::do_release(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_release_in* in = (struct fuse_release_in*)req->arg();
        sp<FsNode> node;

        sp<FuseOpenCtx> ctx;
        if(in->fh)
            ctx = sp<FuseOpenCtx>::docking((FuseOpenCtx*)in->fh);

        if( DFW_RET(retval, req->m_nl->getNode(node, req->nodeid())) )
            return DFW_FUSEWORKER_E(retval,req,-EIO);

        if( is_enable(FUSE::OP_RELEASE) ){
            if(DFW_RET(retval, on_release(req
                                        , node->m_name.toChars(), ctx)))
                return DFW_FUSEWORKER_E(retval,req,-EIO);
        }

        return DFW_RET_C(retval, req->send_error(0));
    }

    /** Not response. */
    sp<Retval> FuseWorker::do_forget(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_forget_in *in = (struct fuse_forget_in *)req->arg();
        uint64_t nlookup = in->nlookup;

        if( is_enable(FUSE::OP_FORGET)
            && DFW_RET(retval, on_forget(req, req->nodeid(), nlookup))){
        }

        req->m_nl->forget(req->nodeid(), nlookup);

        return NULL;
    }

    /** Not response. */
    sp<Retval> FuseWorker::do_batchforget(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_batch_forget_in *in
                         = (struct fuse_batch_forget_in*)req->arg();
        struct fuse_forget_one *param
                         = (struct fuse_forget_one*) PARAM(in);

        if( is_enable(FUSE::OP_BATCH_FORGET)
                && DFW_RET(retval, on_batchforget(req))){
        }

        for(unsigned i = 0; i < in->count; i++) {
            struct fuse_forget_one *forget = &param[i];
            req->m_nl->forget(forget->nodeid, forget->nlookup);
        }
        return NULL;
    }

    /** Not response. */
    sp<Retval> FuseWorker::do_interrupt(sp<FuseRequest>& req){
        sp<Retval> retval;
        struct fuse_interrupt_in *in
                             = (struct fuse_interrupt_in *)req->arg();

        if(is_enable(FUSE::OP_INTERRUPT)
                && DFW_RET(retval, on_interrupt(req, in->unique))){
        }

        sp<FuseRequest> intrReq = m_reqlist->getRequest(in->unique);
        if(intrReq.has()){
            {
                AutoLock _l(intrReq.get());
                intrReq->setInterrupt();
            }
            //intrReq->send_error(-EINTR);
        }

        return NULL;
    }

};
#endif

