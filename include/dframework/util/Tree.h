#ifndef DFRAMEWORK_UTIL_TREE_H
#define DFRAMEWORK_UTIL_TREE_H

#ifdef __cplusplus
namespace dframework {

    template <typename T>
    class Tree 
    {
    public:
        class Node {
        public:
            sp<Node> m_parent;
            sp<Node> m_left;   // <
            sp<Node> m_right;  // >
            sp<Node> m_center; // ==
            int      m_size;
            sp<T>    m_data;

            Node();
            virtual ~Node();
        };
    private:
        sp<Node> m_RootNode;
        sp<Node> m_FirstNode;
        sp<Node> m_LastNode;
        int      m_iSize;

    public:
        Tree();
        virtual ~Tree();

        sp<Retval> insert(const sp<T>& data);
        sp<T>      get(const sp<T>& data);
        sp<Node>   getLastLeft(const sp<Node>& node);
        sp<Node>   getLastRight(const sp<Node>& node);
        sp<T>      remove(const sp<T>& data);

    };

    template <typename T>
    Tree<T>::Node::Node(const sp<T>& obj){
        m_size = 0;
        m_data = obj;
    }

    template <typename T>
    Tree<T>::Node::~Node(){
        m_parent = m_left = m_right = NULL;
    }

    template <typename T>
    Tree<T>::Tree(){
        m_iSize = 0;
    }

    template <typename T>
    sp<Retval> Tree<T>::insert(const sp<T>& data){
        if( !data.has() ) 
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        sp<Node> node = new Node(data);
        if( !m_RootNode.has() ){
            m_RootNode = node;
            return NULL;
        }

        sp<Node> cur = m_RootNode;
        while(true){
            ++cur->m_size;
            if(      node <  cur ){
                if( !cur->m_left.has() ){
                    cur->m_left = node;
                    node->m_parent = cur;
                    break;
                }
                cur = cur->m_left;
            }else if( node >  cur ){
                if( !cur->m_right.has() ){
                    cur->m_right = node;
                    node->m_parent = cur;
                    break;
                }
                cur = cur->m_right;
            }else{
                if( cur->m_center )
                    node->m_center = cur;
                cur->m_center = node;
                node->m_parent = cur;
                break;
            }
        }

        return NULL;
    }

    template <typename T>
    sp<T> Tree<T>::get(const sp<T>& data){
        if( !data.has() ) return NULL;

        sp<Node> cur = m_RootNode;
        while(cur.has()){
            if(      node <  cur )
                cur = cur->m_left;
            else if( node >  cur )
                cur = cur->m_right;
            else if( node == cur )
                return cur;
        }

        return NULL;
    }

    template <typename T>
    sp<Node> Tree<T>::getLastLeft(const sp<Node>& node){
        sp<Node> cur = node;
        while(cur.has()){
            if(!cur->m_left.has())
                return cur;
            cur = cur->m_left;
        }
        return NULL;
    }

    template <typename T>
    sp<Node> Tree<T>::getLastRight(const sp<Node>& node){
        sp<Node> cur = node;
        while(cur.has()){
            if(!cur->m_right.has())
                return cur;
            cur = cur->m_right;
        }
        return node;
    }

    template <typename T>
    sp<T> Tree<T>::remove(const sp<T>& data){
        if( !data.has() ) return NULL;

        sp<Node> find;
        sp<Node> cur = m_RootNode;
        while(cur.has()){
            --cur->size;
            if(      node <  cur )
                cur = cur->m_left;
            else if( node >  cur )
                cur = cur->m_right;
            else if( node == cur ){
                find = cur;
                break;
            }
        }
        if( !find.has() ) return NULL;

        sp<Node> rep;
        sp<Node> parent = find->m_parent();

        if( find->m_left.has() ){
            if( find->m_right.has() ){
                rep = getLastRight(find->m_left);
                rep->m_right = find->m_right;
            }
            rep = find->m_left;
        }else if( find->m_right.has() ) {
            rep = find->m_right;
        }

        if( !parent.has() )
            m_RootNode = rep;
        else if( parent->m_left == find )
            parent->m_left = rep;
        else
            parent->m_right = rep;

        return find;
    }

};
#endif

#endif /* DFRAMEWORK_UTIL_TREE_H */

