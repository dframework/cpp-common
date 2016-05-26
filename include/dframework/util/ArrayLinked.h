#ifndef DFRAMEWORK_UTIL_ARRAYLINKED_H
#define DFRAMEWORK_UTIL_ARRAYLINKED_H
#define DFRAMEWORK_UTIL_ARRAYLINKED_VERSION 1

#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class ArrayLinked
    {
    public:
        class Node : public Object {
        public:
            Node* m_prev;
            Node* m_next;
            sp<T> m_data;

            Node(const sp<T>& data);
            inline virtual ~Node() {}

            DFW_OPERATOR_EX_DECLARATION(Node, m_data);
        };

    private:
        ArraySorted<Node> m_aList;

    public:
        ArrayLinked();
        virtual ~ArrayLinked();

        virtual sp<Retval> insert(int *position, const sp<T>& data);
        virtual sp<T> get(int index) const;
        virtual sp<T> get(const sp<T>& data) const;

        inline virtual sp<Retval> insert(const sp<T>& data){
            return insert(NULL, data);
        }
        inline int size() const { return m_aList.size(); }
    };

    // --------------------------------------------------------------

    template <typename T>
    ArrayLinked<T>::Node::Node(const sp<T>& data) 
            : Object()
    {
        m_data = data;
        m_prev = NULL;
        m_next = NULL;
    }

    // --------------------------------------------------------------

    template <typename T>
    ArrayLinked<T>::ArrayLinked() {
    }

    template <typename T>
    ArrayLinked<T>::~ArrayLinked(){
    }

    template <typename T>
    sp<Retval> ArrayLinked<T>::insert(int *position, const sp<T>& data){
        int pos = -1;

        sp<Node> node = new Node(data);
        sp<Retval> retval = m_aList.insert(&pos, node);
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        sp<Node> prev = m_aList.get(pos-1);
        sp<Node> next = m_aList.get(pos+1);
        if( prev.has() ) prev->m_next = node.get();
        if( next.has() ) next->m_prev = node.get();

        if(position) *position = pos;
        return NULL;
    }

    template <typename T>
    sp<T> ArrayLinked<T>::get(int index) const {
        sp<Node> node = m_aList.get(index);
        if(node.has())
            return node->m_data;
        return NULL;
    }

    template <typename T>
    sp<T> ArrayLinked<T>::get(const sp<T>& data) const{
        sp<Node> in = new Node(data);
        sp<Node> node = m_aList.get(in);
        if(node.has())
            return node->m_data;
        return NULL;
    }


};
#endif

#endif /* DFRAMEWORK_UTIL_ARRAYLINKED_H */

