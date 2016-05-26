#ifndef DFRAMEWORK_BASE_OBJECT_H
#define DFRAMEWORK_BASE_OBJECT_H

#include <dframework/base/type.h>
#include <dframework/base/sp.h>
#include <dframework/util/D_Sorted.h>
#include <pthread.h>


#define DFW_DEFINITION_SAFE_COUNT(x)       \
    static Object   ___m_##x##_safe;       \
    static uint64_t ___m_##x##_safe_count;

#define DFW_DECLARATION_SAFE_COUNT(x)      \
    Object   x::___m_##x##_safe;           \
    uint64_t x::___m_##x##_safe_count = 0;

#define DFW_DECLARATION_SAFE_C_COUNT(c,x)  \
    Object   c::___m_##x##_safe;           \
    uint64_t c::___m_##x##_safe_count = 0;

#define DFW_SAFE_ADD(x,l)                  \
    AutoLock ___##l(&x::___m_##x##_safe);  \
    x::___m_##x##_safe_count++;

#define DFW_SAFE_C_ADD(c,x,l)              \
    AutoLock ___##l(&c::___m_##x##_safe);  \
    c::___m_##x##_safe_count++;

#define DFW_SAFE_REMOVE(x,l)               \
    AutoLock ___##l(&x::___m_##x##_safe);  \
    x::___m_##x##_safe_count--;

#define DFW_SAFE_C_REMOVE(c,x,l)           \
    AutoLock ___##l(&c::___m_##x##_safe);  \
    c::___m_##x##_safe_count--;

#define DFW_SAFE_VALUE(x) (x::___m_##x##_safe_count)
#define DFW_SAFE_C_VALUE(c,x) (c::___m_##x##_safe_count)


#define  DFW_SAFE_RECURSIVE 1


#ifdef __cplusplus
namespace dframework {

    class Object
    {
    public:
        
    private:
        int              ___m_ref_count;
#if (defined(_WIN32) || defined(_WIN64)) && (!defined(__MINGW32__))
        CRITICAL_SECTION ___m_handle;
#else
#  if defined(DFW_SAFE_RECURSIVE)
        pthread_mutexattr_t ___m_attr;
#  else
        int              ___m_count;
        pthread_t        ___m_thread_id;
#  endif
        pthread_mutex_t  ___m_handle;
#endif
        int              ___m_status;
        int              ___m_init();

    public:
        Object();
        virtual ~Object();

        virtual int incRefCount();
        virtual int decRefCount();
        inline int getRefCount() const {
            return ___m_ref_count;
        }

        int     lock();
        int     unlock();

        DFW_D_IVBO(==,Object,*) { return (this ==  p); }
        DFW_D_IVBO(!=,Object,*) { return (this !=  p); }
        DFW_D_IVBO(> ,Object,*) { return (this >   p); }
        DFW_D_IVBO(< ,Object,*) { return (this <   p); }
        DFW_D_IVBO(>=,Object,*) { return (this >=  p); }
        DFW_D_IVBO(<=,Object,*) { return (this <=  p); }
        DFW_D_IVBO(==,Object,&) { return (this == &p); }
        DFW_D_IVBO(!=,Object,&) { return (this != &p); }
        DFW_D_IVBO(> ,Object,&) { return (this >  &p); }
        DFW_D_IVBO(< ,Object,&) { return (this <  &p); }
        DFW_D_IVBO(>=,Object,&) { return (this >= &p); }
        DFW_D_IVBO(<=,Object,&) { return (this <= &p); }
    };

    class AutoLock {
    private:
        Object& m_obj;
    public:
        inline AutoLock(Object& obj) : m_obj(obj)  { m_obj.lock(); }
        inline AutoLock(Object* obj) : m_obj(*obj) { m_obj.lock(); }
        inline ~AutoLock() { m_obj.unlock(); }
    };

}; // end namespace dframework
#endif /* end if cplusplus */

#endif /* DFRAMEWORK_BASE_OBJECT_H */

