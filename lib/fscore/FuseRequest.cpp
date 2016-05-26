#ifndef _WIN32
#include <dframework/fscore/FuseRequest.h>
#include <dframework/fscore/FuseUtil.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(FuseRequest);

    FuseRequest::FuseRequest(uint64_t unique)
            : Object()
    {
        DFW_SAFE_ADD(FuseRequest, l);

        m_head = NULL;
        m_buf = NULL;
        m_bufsize = 0;
        m_unique = unique;
        m_bInterrupt = false;
        m_bInterrupted = false;
    }

    FuseRequest::FuseRequest(sp<FuseMount>& mount, sp<FsNodeList>& nl)
            : Object()
    {
        DFW_SAFE_ADD(FuseRequest, l);
        m_mount = mount;
        m_nl = nl;
        m_head = NULL;
        m_buf = NULL;
        m_bufsize = 0;
        m_unique = 0;
        m_bInterrupt = false;
        m_bInterrupted = false;
    }

    FuseRequest::~FuseRequest(){
        DFW_SAFE_REMOVE(FuseRequest, l);
        DFW_FREE(m_buf);

        if( m_context.has() )
            m_context->clear();
    }

    sp<Retval> FuseRequest::set(const char* buf, size_t size){
        m_bufsize = size;
        if( !(m_buf = (char*)::malloc(m_bufsize)) )
            return DFW_RETVAL_NEW(DFW_E_NOMEM, 0);
        ::memcpy(m_buf, buf, size);
        m_head = (struct fuse_in_header*)m_buf;
        m_arg = (void*)(m_buf+sizeof(struct fuse_in_header));
        m_argsize = size - sizeof(struct fuse_in_header);
        m_unique = unique();
        return NULL;
    }

    void FuseRequest::setInterrupt()   {
        AutoLock _l(this);
        m_bInterrupt = true; 
    }
    void FuseRequest::setInterrupted() {
        AutoLock _l(this);
        m_bInterrupted = true; 
    }
    bool FuseRequest::getInterrupt()   {
        AutoLock _l(this);
        return m_bInterrupt; 
    }
    bool FuseRequest::getInterrupted() {
        AutoLock _l(this);
        return m_bInterrupted; 
    }

    sp<Retval> FuseRequest::send(uint64_t unique, int error
                            , const dfw_point_t arg, size_t argsize)
    {
        bool sendIntr = false;
        {
            AutoLock _l(this);
            if(m_bInterrupt){
                if(m_bInterrupted)
                    return NULL;
                setInterrupted();
                sendIntr = true;
            }
        }
        if(sendIntr)
            return m_mount->send(unique, -EINTR, NULL, 0);
        return m_mount->send(unique, error, arg, argsize);
    }


    sp<Retval> FuseRequest::send_entry(struct fuse_entry_param* entry)
    {
        struct fuse_entry_out arg;
        size_t size = proto_minor() < 9 ?
                FUSE_COMPAT_ENTRY_OUT_SIZE : sizeof(arg);

        /* before ABI 7.4 e->ino == 0 was invalid, only ENOENT meant
           negative entry */
        if (!entry->ino && proto_minor() < 4)
                return send_error(-ENOENT);

        memset(&arg, 0, sizeof(arg));

        arg.nodeid = entry->ino;
        arg.generation = entry->generation;
        arg.entry_valid = 1; //FIXME:calc_timeout_sec(e->entry_timeout);
        arg.entry_valid_nsec = 0; //FIXME:calc_timeout_nsec(e->entry_timeout);
        arg.attr_valid = 1; //FIXME:calc_timeout_sec(e->attr_timeout);
        arg.attr_valid_nsec = 0; //FIXME:calc_timeout_nsec(e->attr_timeout);
        FuseUtil::statToFuse(&arg.attr, &entry->attr);

        return send_ok(&arg, size);
    }

    // --------------------------------------------------------------

    FuseRequestList::FuseRequestList(){
    }

    FuseRequestList::~FuseRequestList(){
    }

    int FuseRequestList::size(){
        AutoLock _l(this);
        return m_aList.size();
    }

    sp<Retval> FuseRequestList::insertRequest(sp<FuseRequest>& req){
        AutoLock _l(this);
        sp<Retval> retval;
        return DFW_RET_C(retval, m_aList.insert(req));
    }

    sp<FuseRequest> FuseRequestList::removeRequest(sp<FuseRequest>& req){
        AutoLock _l(this);
        return m_aList.remove(req);
    }

    sp<FuseRequest> FuseRequestList::removeRequest(uint64_t unique){
        AutoLock _l(this);
        sp<FuseRequest> req = new FuseRequest(unique);
        return m_aList.remove(req);
    }

    sp<FuseRequest> FuseRequestList::getRequest(sp<FuseRequest>& req){
        AutoLock _l(this);
        return m_aList.get(req);
    }

    sp<FuseRequest> FuseRequestList::getRequest(uint64_t unique){
        AutoLock _l(this);
        sp<FuseRequest> req = new FuseRequest(unique);
        return m_aList.get(req);
    }

    // --------------------------------------------------------------

    int FuseRequest::contextSize(){
        AutoLock _l(this);
        return (m_context.has() ? m_context->contextSize() : 0);
    }

    sp<Retval> FuseRequest::insertContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        return (m_context.has()
                ? m_context->insertContext(ctx)
                : DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                      , "Has no FsContext at FuseRequest"));
    }

    sp<NamedObject> FuseRequest::removeContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        return (m_context.has() ? m_context->removeContext(ctx) : NULL);
    }

    sp<NamedObject> FuseRequest::getContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        return (m_context.has() ? m_context->getContext(ctx) : NULL);
    }

    sp<NamedObject> FuseRequest::getContext(int position){
        AutoLock _l(this);
        return (m_context.has() ? m_context->getContext(position) : NULL);
    }

    sp<Object> FuseRequest::getContext(const String& named) {
        AutoLock _l(this);
        return (m_context.has()
                ? m_context->getContext(named.toChars()) : NULL);
    }

    sp<Object> FuseRequest::getContext(const char* named){
        AutoLock _l(this);
        return (m_context.has()
                ? m_context->getContext(named) : NULL);
    }

};
#endif

