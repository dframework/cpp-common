#ifndef DFRAMEWORK_UTIL_ARRAYSELFSORTED_H
#define DFRAMEWORK_UTIL_ARRAYSELFSORTED_H
#define DFRAMEWORK_UTIL_ARRAYSELFSORTED_VERSION 1

#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class ArraySelfSorted : public ArraySorted<T>
    {
    private:
        int insertId(const sp<T>& data) const;
    public:
        inline ArraySelfSorted() {}
        inline virtual ~ArraySelfSorted() {}

        virtual int index(const sp<T>& data) const;
    };

    template <typename T>
    int ArraySelfSorted<T>::insertId(const sp<T>& data) const{
        if(!data.has()) return -1;
        if(ArrayBase<T>::m_iSize==0) return 0;
        if(data.get() <= DFW_ARRAY_CONVERT_TP(0)) return 0;
        if(data.get() >= DFW_ARRAY_CONVERT_TP(ArrayBase<T>::m_iSize-1))
            return ArrayBase<T>::m_iSize;

        int sp = 0;
        int ep = ArrayBase<T>::m_iSize;
        while(true){
            int cp = ((ep-sp)/2) + sp;
            if(sp==cp || ep==cp){
                if( data.get() <= DFW_ARRAY_CONVERT_TP(sp)) return sp;
                return ep;
            }
            if( data.get() == DFW_ARRAY_CONVERT_TP(cp)){
                return cp;
            }else if( data.get() < DFW_ARRAY_CONVERT_TP(cp)){
                ep = cp;
            }else{
                sp = cp;
            }
        }
    }

    template <typename T>
    int ArraySelfSorted<T>::index(const sp<T>& data) const{
        if(!data.has()) return -1;
        if(ArrayBase<T>::m_iSize==0) return -1;
        if( data.get() < DFW_ARRAY_CONVERT_TP(0)) return -1;
        if( data.get() > DFW_ARRAY_CONVERT_TP(ArrayBase<T>::m_iSize-1)) return -1;

        int sp = 0;
        int ep = ArrayBase<T>::m_iSize;
        while(true){
            int cp = ((ep-sp)/2) + sp;
            if(sp==cp || ep==cp){
                if( data.get() == DFW_ARRAY_CONVERT_TP(sp)) 
                    return sp;
                if( data.get() == DFW_ARRAY_CONVERT_TP(ep-1)) 
                    return (ep-1);
                return -1;
            }
            if( data.get() == DFW_ARRAY_CONVERT_TP(cp)){
                return cp;
            }else if( data.get() < DFW_ARRAY_CONVERT_TP(cp)){
                ep = cp;
            }else{
                sp = cp;
            }
        }
    }

};
#endif

#endif /* DFRAMEWORK_UTIL_ARRAYSELFSORTED_H */

