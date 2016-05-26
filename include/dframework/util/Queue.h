#ifndef DFRAMEWORK_UTIL_QUEUE_H
#define DFRAMEWORK_UTIL_QUEUE_H

#include <dframework/base/Object.h>

#ifdef __cplusplus
namespace dframework {
    
    template <typename T>
    class Queue : public Object
    {
    protected:
        struct QueueNode {
            sp<T> data;
            QueueNode *next;
        };
        typedef QueueNode* QueueNode_ptr;
        
    private:
        QueueNode_ptr m_pFront;
        QueueNode_ptr m_pEnd;
        unsigned long m_iSize;
        
    public:
        Queue();
        virtual ~Queue();
        
        sp<Retval> push(sp<T>& data);
        sp<T> pop();
        unsigned long size();
        bool empty();
    };
    
    
    
    template <typename T>
        Queue<T>::Queue() :
        m_pFront(NULL),
        m_pEnd(NULL),
        m_iSize(0)
    {
        
    }
    
    template <typename T>
    Queue<T>::~Queue(){
        while(!empty()) pop();
    }
    
    template <typename T>
    sp<Retval> Queue<T>::push(sp<T>& data){
        AutoLock _l(this);
        QueueNode_ptr tmp = new QueueNode;
        tmp->data = data;
        tmp->next = NULL;
        
        if(empty()){
            m_pFront = tmp;
        }else if(m_pEnd){
            m_pEnd->next = tmp;
        }
        m_pEnd = tmp;
        
        m_iSize++;
        
        return NULL;
    }
    
    template <typename T>
    sp<T> Queue<T>::pop(){
        AutoLock _l(this);
        if(empty()){ return NULL; }
        
        QueueNode_ptr tmp = m_pFront;
        
        m_pFront = m_pFront->next;
        if(NULL==m_pFront){
            m_pEnd = NULL;
        }
        
        --m_iSize;
        
        sp<T> res = tmp->data;
        DFW_DELETE(tmp);
        return res;
    }
    
    template <typename T>
    unsigned long Queue<T>::size(){
        AutoLock _l(this);
        return m_iSize;
    }
    
    template <typename T>
    bool Queue<T>::empty(){
        AutoLock _l(this);
        if(m_iSize>0)
            return false;
        return true;
    }
    
    
};
#endif // end ifdef __cplusplus

#endif /* DFRAMEWORK_UTIL_QUEUE_H */

