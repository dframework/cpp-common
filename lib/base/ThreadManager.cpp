#include <dframework/base/ThreadManager.h>
#include <dframework/lang/String.h>
#include <stdarg.h>
#include <ctype.h>

namespace dframework {

    ThreadManager::Node::Node(sp<Thread>& thread)
            : Object()
    {
        m_thread = thread;
        m_handle = thread->handle();
    }

    ThreadManager::Node::Node(pthread_t handle)
            : Object()
    {
        m_handle = handle;
    }

    // --------------------------------------------------------------
    
    sp<ThreadManager> ThreadManager::m_manager = NULL;
    Object            ThreadManager::m_safe;

    sp<ThreadManager>& ThreadManager::instance(){
        AutoLock _l(&m_safe);
        return (m_manager.has()
               ? m_manager
               : (m_manager = new ThreadManager()));
    }

    ThreadManager::ThreadManager(){
        m_iscancel = false;
        m_refcount = 0;
    }

    ThreadManager::~ThreadManager(){
    }

    int ThreadManager::size() {
        AutoLock _l(this);
        return m_aList.size();
    }

    sp<Retval> ThreadManager::joinAll(){
        sp<Retval> retval;
        while(true){
            sp<Thread> thd = get(0);
            if(!thd.has()) return NULL;
            if( DFW_RET(retval, thd->join()) ){
            }
        }
    }

    sp<Retval> ThreadManager::cancelAll(){
        sp<Retval> retval;
        {
            AutoLock _l(this);
            m_iscancel = true;
            for(int k=0; k<size(); k++){
                sp<Thread> thd = get(k);
                if(!thd.has()) continue;
                if( DFW_RET(retval, thd->cancel()) ){
                }
            }
        }
        return joinAll();
    }

    sp<Thread> ThreadManager::get(int position) {
        AutoLock _l(this);
        sp<Node> node = m_aList.get(position);
        if(node.has())
            return node->m_thread;
        return NULL;
    }

    sp<Thread> ThreadManager::getThread(pthread_t id) {
        AutoLock _l(this);
        sp<Node> node = new Node(id);
        sp<Node> find = m_aList.get(node);
        if(find.has())
            return find->m_thread;
        return NULL;
    }

    sp<Retval> ThreadManager::insert(sp<Thread>& running_thread){
        AutoLock _l(this);
        m_refcount++;
        if( m_iscancel )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "This is cancelAll.");
        sp<Node> node = new Node(running_thread);
        return m_aList.insert(node);
    }

    sp<Thread> ThreadManager::remove(pthread_t stoping_thread_id){
        AutoLock _l(this);
        m_refcount--;
        sp<Node> node = new Node(stoping_thread_id);
        sp<Node> find = m_aList.remove(node);
        if(find.has())
            return find->m_thread;
        return NULL;
    }

    int ThreadManager::refcount(){
        AutoLock _l(this);
       return m_refcount;
    }

    // --------------------------------------------------------------

    AutoTrace::AutoTrace(bool enable, const char* file
                       , const char* func, int line, const char* fmt, ...)
    {
        if(enable){
            char* msg = NULL;
            sp<ThreadManager> mana = ThreadManager::instance();
            m_thread = mana->getThread(pthread_self());
            if(m_thread.has()){
                if(String::strlen(fmt)>0){
                    char *buf = NULL;
                    size_t size;
                    va_list ap, ap2;
                    DFW_VALIST_SIZE(size, temp, fmt, ap);
                    DFW_VALIST_STRDUP(buf, size, fmt, ap2);
                    msg = buf;
                }
                m_trace = new Thread::Trace(file, func, line, msg);
                m_thread->addTrace(m_trace);
            }
            DFW_FREE(msg);
        }
    }

    AutoTrace::~AutoTrace(){
        if(m_thread.has())
            m_thread->removeTrace(m_trace);
    }

}; // end namespace dframework

