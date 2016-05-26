#ifndef DFRAMEWORK_UTIL_ARRAYBASE_H
#define DFRAMEWORK_UTIL_ARRAYBASE_H
#define DFRAMEWORK_UTIL_ARRAYBASE_VERSION 1

#include <dframework/base/Retval.h>

#define DFW_ARRAY_CONVERT_TP(k)   (ArrayBase<T>::m_pData[(k)])
#define DFW_ARRAY_CONVERT_T(k)  (*(ArrayBase<T>::m_pData[(k)]))

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class ArrayBase : public Object
    {
    protected:
        int          m_iCapacity;
        int          m_iSize;
        T** m_pData;        

    public:
        ArrayBase();
        virtual ~ArrayBase();

        virtual void clear();

        sp<Retval> capacity();

        inline bool isCapacity() const{
            return (m_iSize<m_iCapacity ? true : false);
        }

        inline int size() const { return m_iSize; }

    protected:
        virtual void ___drop();

    private:
        int getNewCapacity(int current) const;
    };

    template <typename T>
    ArrayBase<T>::ArrayBase() :
       m_iCapacity(0),
       m_iSize(0),
       m_pData(NULL)
    {
    }

    template <typename T>
    ArrayBase<T>::~ArrayBase(){
        ___drop();
        DFW_FREE(m_pData);
    }

    template <typename T>
    void ArrayBase<T>::___drop(){
        AutoLock _l(this);
        for(int k=0; k<m_iSize; k++){
            if(m_pData[k]){
                sp<T>::docking( m_pData[k] );
                m_pData[k] = NULL;
            }
        }
    }

    template <typename T>
    void ArrayBase<T>::clear(){
        AutoLock _l(this);
        ___drop();
        if(m_iCapacity>0){
            ::memset(m_pData, 0, m_iCapacity*sizeof(T*));
        }
        m_iSize = 0;
    }

    template <typename T>
    sp<Retval> ArrayBase<T>::capacity(){
        AutoLock _l(this);
        if(!isCapacity()){
            int newcapa = m_iCapacity+getNewCapacity(m_iCapacity);
            size_t usize = sizeof(T*);
            size_t newsize = usize*newcapa;
            T** test = NULL;
            if(!m_pData){
                if( !(test = (T**)malloc(newsize)) )
                    return DFW_RETVAL_NEW(DFW_E_NOMEM,0);
            }else{
                test = (T**)realloc(m_pData, newsize);
                if(!test){
                    test = (T**)malloc(newsize);
                    if(!test)
                        return DFW_RETVAL_NEW(DFW_E_NOMEM,0);
                    if(m_iCapacity>0){
                        ::memmove(test, m_pData, m_iCapacity*sizeof(T*));
                    }
                }
                ::memset(test+m_iCapacity,0,(newcapa-m_iCapacity)*sizeof(T*));
            }
            m_pData = test;
            m_iCapacity = newcapa;
        }
        return NULL;
    }

    template <typename T>
    int ArrayBase<T>::getNewCapacity(int current) const{
        int capa = current/10;
        if(capa<100)
            capa = 100;
        else if(capa<500)
            capa = 500;
        else
            capa = 1000;
        return capa;
    }


};
#endif

#endif /* DFRAMEWORK_UTIL_ARRAYBASE_H */

