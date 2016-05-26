#ifndef DFRAMEWORK_UTIL_ARRAYSORTED_H
#define DFRAMEWORK_UTIL_ARRAYSORTED_H
#define DFRAMEWORK_UTIL_ARRAYSORTED_VERSION 1

#include <dframework/util/ArrayBase.h>
#include <dframework/util/D_Sorted.h>

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class ArraySorted : public ArrayBase<T>
    {
    private:
        virtual int insertId(const sp<T>& data) const;
    public:
        inline ArraySorted() {}
        inline virtual ~ArraySorted() {}

        virtual sp<Retval> insert(int *, const sp<T>& data);
        inline virtual sp<Retval> insert(const sp<T>& data){
            return insert(NULL, data);
        }

        virtual sp<T> get(int position) const;
        virtual sp<T> remove(int position);

        virtual sp<T> get(const sp<T>& u) const;
        virtual sp<T> remove(const sp<T>& u);

        virtual int index(const sp<T>& data) const;
    };

    template <typename T>
    sp<Retval> ArraySorted<T>::insert(int *position, const sp<T>& data){
        if(!data.has())
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        sp<Retval> retval = ArrayBase<T>::capacity();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        int iid = insertId(data);
        if(-1==iid)
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
        if(get(iid).has()){
            int len = ArrayBase<T>::m_iSize-iid;
            if(len>0){
              size_t size = len *sizeof(dfw_point_t);
              dfw_point_t *p = (dfw_point_t*)malloc(size);
              if(p){
                  memmove(p, ArrayBase<T>::m_pData+iid, size);
                  memmove(ArrayBase<T>::m_pData+iid+1, p, size);
                  free(p);
              }else{
                  int limit = ArrayBase<T>::m_iSize;
                  for(int k=limit; k>iid; k--)
                    ArrayBase<T>::m_pData[k] = ArrayBase<T>::m_pData[k-1];
              }
            }
        }

        if( position ) *position = iid;
        ArrayBase<T>::m_pData[iid] = sp<T>::undocking( data.get() );
        ArrayBase<T>::m_iSize++;

        return NULL;
    }

    template <typename T>
    sp<T> ArraySorted<T>::get(int position) const{
        if( position<ArrayBase<T>::m_iSize && position>=0 )
            return DFW_ARRAY_CONVERT_TP(position);
        return NULL;
    }

    template <typename T>
    sp<T> ArraySorted<T>::remove(int position){
        sp<T> val = get(position);
        if(!val.has()) return NULL;

        sp<T>::docking( val.get() );

        if(position==(ArrayBase<T>::m_iSize-1)){
            ArrayBase<T>::m_pData[position]=NULL;
        }else{
            int len = ArrayBase<T>::m_iSize-position;
            if(len>0){
                size_t size = sizeof(T*)*len;
                dfw_point_t *p = (dfw_point_t*)malloc(size);
                if(p){
                  memmove(p, ArrayBase<T>::m_pData+position+1, size);
                  memmove(ArrayBase<T>::m_pData+position, p, size);
                  free(p);
                }else{
                  int limit = position+len;
                  for(int k=position; k<limit; k++)
                    ArrayBase<T>::m_pData[k] = ArrayBase<T>::m_pData[k+1];
                }
            }
        }
        ArrayBase<T>::m_iSize--;

        return val;
    }

    template <typename T>
    sp<T> ArraySorted<T>::get(const sp<T>& data) const{
        return get(index(data));
    }

    template <typename T>
    sp<T> ArraySorted<T>::remove(const sp<T>& data){
        return remove(index(data));
    }

    template <typename T>
    int ArraySorted<T>::insertId(const sp<T>& data) const{
        if(!data.has()) return -1;
        if(ArrayBase<T>::m_iSize==0) return 0;
        if(*data <= DFW_ARRAY_CONVERT_TP(0)) return 0;
        if(*data >= DFW_ARRAY_CONVERT_TP(ArrayBase<T>::m_iSize-1)) return ArrayBase<T>::m_iSize;

        int sp = 0;
        int ep = ArrayBase<T>::m_iSize;
        while(true){
            int cp = ((ep-sp)/2) + sp;
            if(sp==cp || ep==cp){
                if( *data <= DFW_ARRAY_CONVERT_TP(sp)) return sp;
                return ep;
            }
            if( *data == DFW_ARRAY_CONVERT_TP(cp)){
                return cp;
            }else if( *data < DFW_ARRAY_CONVERT_TP(cp)){
                ep = cp;
            }else{
                sp = cp;
            }
        }
    }

    template <typename T>
    int ArraySorted<T>::index(const sp<T>& data) const{
        if(!data.has()) return -1;
        if(ArrayBase<T>::m_iSize==0) return -1;
        if( *data < DFW_ARRAY_CONVERT_TP(0)) return -1;
        if( *data > DFW_ARRAY_CONVERT_TP(ArrayBase<T>::m_iSize-1)) return -1;

        int sp = 0;
        int ep = ArrayBase<T>::m_iSize;
        while(true){
            int cp = ((ep-sp)/2) + sp;
            if(sp==cp || ep==cp){
                if( *data == DFW_ARRAY_CONVERT_TP(sp)) 
                    return sp;
                if( *data == DFW_ARRAY_CONVERT_TP(ep-1)) 
                    return (ep-1);
                return -1;
            }
            if( *data == DFW_ARRAY_CONVERT_TP(cp)){
                return cp;
            }else if( *data < DFW_ARRAY_CONVERT_TP(cp)){
                ep = cp;
            }else{
                sp = cp;
            }
        }
    }


};
#endif

#endif /* DFRAMEWORK_UTIL_SORTEDARRAY_H */

