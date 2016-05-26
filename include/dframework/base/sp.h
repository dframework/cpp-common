#ifndef DFRAMEWORK_BASE_SP_H
#define DFRAMEWORK_BASE_SP_H
#define DFRAMEWORK_BASE_SP_VERSION 1

#include <dframework/util/D_Sorted.h>

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class sp
    {
    private:
        template<typename Y> friend class sp;
        T* m_ptr;
        bool m_bUndockingResponsibility;
        int dec();

    public:
        sp();
        sp(T* ptr);
        sp(const sp<T>& ref);

        template<typename U> sp(U* other);
        template<typename U> sp(const sp<U>& other);

        virtual ~sp();

        inline bool operator!  () const { return (m_ptr ? false:true); }
        inline T&   operator*  () const { return *m_ptr; }
        inline T*   operator-> () const { return m_ptr; }
        inline T*   get() const { return m_ptr; }
        inline bool has() const { return (m_ptr ? true:false); }

    private:
        void ___docking(T* ptr);
        T* ___undocking();

    public:
        static sp<T> docking(T* ptr);
        static T* undocking(T* ptr);

        T* undocking_responsibility();
        T* cancle_undocking_responsibility();

        sp& operator = (T* o);
        sp& operator = (const sp<T>& o);

        template<typename U> sp& operator = (U* other);
        template<typename U> sp& operator = (const sp<U>& other);

        DFW_OPERATOR_TL_EX_DECLARATION(sp<T>, m_ptr);

    };

    template <typename T>
    sp<T>::sp() {
        m_ptr = NULL;
        m_bUndockingResponsibility = false;
    }

    template <typename T>
    sp<T>::sp(T* ptr)
    {
        m_ptr = ptr;
        m_bUndockingResponsibility = false;
#ifdef DEBUG_TERMINAL
        printf("sp(T* ptr), s=%p, p=%p\n", this, m_ptr);
#endif
        if(m_ptr)
            m_ptr->incRefCount();
    }

    template <typename T>
    sp<T>::sp(const sp<T>& ref)
    {
        m_ptr = ref.get();
        m_bUndockingResponsibility = false;
#ifdef DEBUG_TERMINAL
        printf("sp(sp<T>& ref), s=%p, p=%p\n", this, m_ptr);
#endif
        if(m_ptr) m_ptr->incRefCount();
    }

    template <typename T>
    sp<T>::~sp(){
#ifdef DEBUG_TERMINAL
        printf("deleet sp, s=%p, p=%p\n", this, m_ptr);
#endif
        dec();
    }

    template<typename T> template<typename U>
    sp<T>::sp(U* other){
        m_ptr = (T*)other;
        m_bUndockingResponsibility = false;
        if(m_ptr) m_ptr->incRefCount();
    }

    template<typename T> template<typename U>
    sp<T>::sp(const sp<U>& other) {
        m_ptr = (T*)other.m_ptr;
        m_bUndockingResponsibility = false;
        if (m_ptr) m_ptr->incRefCount();
    }

    template <typename T>
    int sp<T>::dec(){
        int count = 0;
        if(m_ptr && !m_bUndockingResponsibility){
            count = m_ptr->decRefCount();
            if( 0 == count ){
#ifdef DEBUG_TERMINAL
                printf("sp::dec s=%p p=%p, count=0, delete\n", this, m_ptr);
#endif
                delete m_ptr;
            }
        }
#ifdef DEBUG_TERMINAL
        printf("sp::dec, s=%p, p=%p, count=%d\n", this, m_ptr, count);
#endif
        m_ptr = NULL;
        return count;
    }

    template <typename T>
    void sp<T>::___docking(T* ptr){
        dec();
        m_ptr = ptr;
    }

    template <typename T>
    T* sp<T>::___undocking(){
        T* ptr = m_ptr;
        m_ptr = NULL;
        return ptr;
    }

    template <typename T>
    T* sp<T>::undocking_responsibility(){
        m_bUndockingResponsibility = true;
        return m_ptr;
    }

    template <typename T>
    T* sp<T>::cancle_undocking_responsibility(){
        m_bUndockingResponsibility = false;
        return m_ptr;
    }

    /* static */
    template <typename T>
    sp<T> sp<T>::docking(T* ptr){
        sp<T> dock;
        sp<T> newz = ptr;
        dock.___docking(ptr);
        return newz;
    }

    /* static */
    template <typename T>
    T* sp<T>::undocking(T* ptr){
        sp<T> dock = ptr;
        return dock.___undocking();
    }

    template <typename T>
    sp<T>& sp<T>::operator = (T* o){
#ifdef DEBUG_TERMINAL
        printf("=(T* o), s=%p, p=%p\n",this, o);
#endif
        dec();
        m_ptr = o;
        if(m_ptr)
            m_ptr->incRefCount();
        return *this;
    }

    template <typename T>
    sp<T>& sp<T>::operator = (const sp<T>& o){
#ifdef DEBUG_TERMINAL
        printf("=(sp<T>& o), s=%p, p=%p\n", this, o.get());
#endif
        dec();
        m_ptr = o.get();
        if(m_ptr)
            m_ptr->incRefCount();
        return *this;
    }

    template <typename T> template<typename U> 
    sp<T>& sp<T>::operator = (U* other){
        dec();
        m_ptr = (T*)other;
        if(m_ptr) m_ptr->incRefCount();
        return *this;
    }
    template <typename T> template<typename U> 
    sp<T>& sp<T>::operator = (const sp<U>& other){
        dec();
        m_ptr = (T*)other.m_ptr;
        if(m_ptr) m_ptr->incRefCount();
        return *this;
    }

}; // end namespace dframework
#endif // end if cplusplus

#endif /* DFRAMEWORK_BASE_SP_H */

