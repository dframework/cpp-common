#ifndef _WIN32
#include <dframework/fscore/FsContext.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(FsContext);

    FsContext::FsContext(){
        DFW_SAFE_ADD(FsContext, l);
    }

    FsContext::~FsContext(){
        DFW_SAFE_REMOVE(FsContext, l);
    }

    int FsContext::contextSize(){
        AutoLock _l(this);
        return m_aList.size();
    }

    sp<Retval> FsContext::insertContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        sp<Retval> retval;
        if(!ctx.has()) return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
        return DFW_RET_C(retval, m_aList.insert(ctx));
    }

    sp<NamedObject> FsContext::removeContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        return m_aList.remove(ctx);
    }

    sp<NamedObject> FsContext::getContext(sp<NamedObject>& ctx){
        AutoLock _l(this);
        return m_aList.get(ctx);
    }

    sp<NamedObject> FsContext::getContext(int position){
        AutoLock _l(this);
        return m_aList.get(position);
    }

    sp<Object> FsContext::getContext(const char* name){
        AutoLock _l(this);
        sp<NamedObject> find = new NamedObject(name);
        sp<NamedObject> dst = getContext(find);
        if(dst.has())
            return dst->m_object;
        return NULL;
    }

    void FsContext::clear(){
        AutoLock _l(this);
        m_aList.clear();
    }

}; // end namespace dframework
#endif

