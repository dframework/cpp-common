#ifndef DFRAMEWORK_UTIL_ARRAY_H
#define DFRAMEWORK_UTIL_ARRAY_H
#define DFRAMEWORK_UTIL_ARRAY_VERSION 1

#include <dframework/util/ArrayBase.h>

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class Array : public ArrayBase<T>
    {
    public:
        inline Array() {}
        inline virtual ~Array() {}

        sp<Retval> insert(sp<T>& data);

        sp<T> get(int position) const;
        sp<T> get(const sp<T>& data) const;
        sp<T> remove(int position);
        sp<T> remove(const sp<T>& data);
        int   index(const sp<T>& data) const;
    };

    template <typename T>
    sp<Retval> Array<T>::insert(sp<T>& data){
        sp<Retval> retval;
        if(!data.has())
            return DFW_RETVAL_NEW(DFW_E_INVAL, EINVAL);
        if( DFW_RET(retval, ArrayBase<T>::capacity()) )
            return DFW_RETVAL_D(retval);

        ArrayBase<T>::m_pData[ArrayBase<T>::m_iSize] 
                                     = sp<T>::undocking( data.get() );
        ArrayBase<T>::m_iSize++;
        return NULL;
    }

    template <typename T>
    sp<T> Array<T>::get(int position) const{
        if( position<ArrayBase<T>::m_iSize && position>=0 )
            return DFW_ARRAY_CONVERT_TP(position);
        return NULL;
    }

    template <typename T>
    sp<T> Array<T>::get(const sp<T>& data) const{
        int idx = index(data);
        if( -1 == idx ) return NULL;
        return Array<T>::m_pData[idx];
    }

    template <typename T>
    int Array<T>::index(const sp<T>& data) const{
        if(!data.has()) return -1;
        int size = ArrayBase<T>::size(); 
        for(int k=0; k<size; k++){
            if( *data == ArrayBase<T>::m_pData[k] )
                return k;
        }
        return -1;
    }

    template <typename T>
    sp<T> Array<T>::remove(int position){
        sp<T> val = get(position);
        if(!val.has()) return NULL;

        sp<T>::docking(val.get());

        ArrayBase<T>::m_pData[position] = ArrayBase<T>::m_pData[ArrayBase<T>::m_iSize-1];
        ArrayBase<T>::m_pData[ArrayBase<T>::m_iSize-1] = NULL;
        ArrayBase<T>::m_iSize--;

        return val;
    }

    template <typename T>
    sp<T> Array<T>::remove(const sp<T>& data){
        int idx = index(data);
        if( -1 == idx ) return NULL;
        return remove(idx);
    }

};
#endif

#endif /* DFRAMEWORK_UTIL_ARRAY_H*/

