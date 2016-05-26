#ifndef _WIN32
#include <dframework/fscore/Fuse_d.h>
#include <dframework/fscore/FsNode.h>
#include <dframework/fscore/FuseRequest.h>
#include <dframework/fscore/FsConfig.h>
#include <dframework/util/Time.h>
#include <dframework/io/Stat.h>
#include <dframework/base/Thread.h>

//#define RECENTLY_TEST_ERROR 0
//#define RECENTLY_TEST 0
#define USE_RECENTLY

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(FsNode);

    FsNode::FsNode(uint64_t nodeid)
            : Object()
    {
        DFW_SAFE_ADD(FsNode, l);
        m_nodeid = nodeid;
        m_atime = 0;
        m_nlookup = 0;
    }

    FsNode::FsNode(const char* name)
            : Object()
    {
        DFW_SAFE_ADD(FsNode, l);
        m_nodeid = 0;
        m_atime = 0;
        m_nlookup = 0;
        m_name = name;
    }

    FsNode::FsNode(const char* name, uint64_t nodeid)
            : Object()
    {
        DFW_SAFE_ADD(FsNode, l);
        m_nodeid = nodeid;
        m_atime = 0;
        m_nlookup = 0;
        m_name = name;
    }

    FsNode::~FsNode(){
        DFW_SAFE_REMOVE(FsNode, l);
    }

    // --------------------------------------------------------------

    FsNodeList::FsNodeList(){
        m_recently = new RecentlyFiles();
        m_generation = 0;
        m_nextid = 1;
        init();
    }

    FsNodeList::~FsNodeList(){
    }

    sp<Retval> FsNodeList::init(){
        return NULL;
    }

    uint64_t FsNodeList::nextid() {
        AutoLock _l(&m_nextid_safe);
        do {
            m_nextid = (m_nextid + 1) & 0xffffffff;
            if (!m_nextid){
                AutoLock _gen(&m_generation_safe);
                m_generation ++;
            }
        } while(    m_nextid == 0 || m_nextid==1 
                 || m_nextid == FUSE_UNKNOWN_INO 
                 || _findNode(m_nextid).has() );
        return m_nextid;
    }

    uint32_t FsNodeList::generation() {
        AutoLock _l(&m_generation_safe);
        return m_generation;
    }

    sp<FsNode> FsNodeList::_findNode(uint64_t nodeid){
        AutoLock _l(this);
        sp<FsNode> test = new FsNode(nodeid);
        return m_aNodeList.get(test);
    }

    sp<Retval> FsNodeList::getNode(sp<FsNode>& node, uint64_t nodeid)
    {
        AutoLock _l(this);
        if( 1==nodeid ){
            node = m_rootNode;
            return NULL;
        }

        sp<FsNode> find = new FsNode(nodeid);
        node = m_aNodeList.get(find);
        if(!node.has()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, -EIO
                , "nodeid=%ld, Not find node.", nodeid);
        }
        return NULL;     
    }

    sp<Retval> FsNodeList::getNode(sp<FsNode>& node, const char* path)
    {
        AutoLock _l(this);
        if( ::strcmp(path, "/")==0 ){
            node = m_rootNode;
            return NULL;
        }

        sp<FsNode> fnode = new FsNode(path);
        sp<FsNamedNode> find = new FsNamedNode(fnode);
        sp<FsNamedNode> named = m_aNamedList.get(find);
        if(!named.has()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, -EIO
                , "path=%s, Not find named-node.", path);
        }
        return NULL;     
    }

    sp<FsNode> FsNodeList::makeRootNode(){
        AutoLock _l(this);
        if(m_rootNode.has())
            return m_rootNode;
        m_rootNode = new FsNode("/", 1);
        //node->m_stat.st_size = 4096;
        //node->m_stat.st_mode |= (Stat::D_IFDIR);
        //node->m_stat.st_mode |= (S_IRWXU|S_IRWXG|S_IRWXO);
        return m_rootNode;
    }

    void FsNodeList::updateAttr(sp<FsNode>& node){
        AutoLock _l(this);
        node->m_atime = Time::currentTimeMillis();
        //FIXME: atime
    }

    sp<Retval> FsNodeList::insertNode(sp<FsNode>& out, const char* path){
        AutoLock _l(this);
        sp<Retval> retval;

        sp<FsNode> node = new FsNode(path);
        sp<FsNamedNode> find = new FsNamedNode(node);
        sp<FsNamedNode> named = m_aNamedList.get(find);
        if( named.has() ){
             node = named->m_node;
             node->m_nlookup++;
             node->m_atime = Time::currentTimeMillis();
             // FIXME: atime
             out = node;
#if 0
printf("insert-has: %s, %lu, named=%d, size=%d, str=%lu\n"
, node->m_name.toChars(), node->m_nlookup
, m_aNamedList.size(), m_aNodeList.size()
, DFW_SAFE_VALUE(String)
);
#endif
             return NULL;
        }

        named = find;
        node->m_nodeid = nextid();
        node->m_nlookup = 1;
        node->m_atime = Time::currentTimeMillis();

        if( DFW_RET(retval, m_aNodeList.insert(node)) )
           return DFW_RETVAL_D(retval);
        if( DFW_RET(retval, m_aNamedList.insert(named)) ){
           m_aNodeList.remove(node);
           return DFW_RETVAL_D(retval);
        }

#ifdef USE_RECENTLY
        sp<Object> ctx = node;
        if( DFW_RET(retval, m_recently->add(node->m_name
                                          , node->m_atime, ctx) ) ){
            //FIXME
        }
        patchRecently();
#endif

        out = node;
#if 0
printf("insert-no: %s, %lu, named=%d, size=%d, str=%lu\n"
, node->m_name.toChars(), node->m_nlookup
, m_aNamedList.size(), m_aNodeList.size()
, DFW_SAFE_VALUE(String)
);
#endif
        return NULL;
    }

    void FsNodeList::removeNode(sp<FsNode>& node){
        AutoLock _l(this);
        AutoLock _node(node.get());
        if(node->m_nodeid!=1){
            sp<FsNamedNode> named = new FsNamedNode(node);
            m_aNamedList.remove(named);
            m_aNodeList.remove(node);
#ifdef USE_RECENTLY
            m_recently->remove(node->m_name);
#endif
        }
    }

    void FsNodeList::removeNode(const char* path){
        AutoLock _l(this);
        sp<FsNode> find = new FsNode(path);
        sp<FsNamedNode> findNamed = new FsNamedNode(find);
        sp<FsNamedNode> searchNamed = m_aNamedList.get(findNamed);
        if( searchNamed.has() ){
            removeNode(searchNamed->m_node);
        }
    }

    // --------------------------------------------------------------

    sp<Retval> FsNodeList::forget(uint64_t nodeid, uint64_t lookup){
        AutoLock _l(this);
#ifdef USE_RECENTLY
        if( FsConfig::useRecently() )
            return NULL;
#endif

        if(nodeid!=1){
            sp<FsNode> node;
            sp<Retval> retval;
            if(DFW_RET(retval, getNode(node, nodeid))){
//printf("forget: error: no getnode, nodeid=%lu\n", nodeid);
                return DFW_RETVAL_D(retval);
            }
            if( (lookup!=0) && (node->m_nlookup != lookup) ){
//printf("forget: nodeid=%lu, path=%s, lookup=%lu, %lu\n", nodeid, node->m_name.toChars(), node->m_nlookup, lookup);
                node->m_nlookup = lookup;
                return NULL;
            }else{
//printf("forget: remove nodeid=%lu, path=%s, lookup=%lu, %lu\n", nodeid, node->m_name.toChars(), node->m_nlookup, lookup);
                removeNode(node);
            }
        }
        return NULL;
    }

    // --------------------------------------------------------------

    sp<Retval> FsNodeList::patchRecently(){
        AutoLock _l(this);
        sp<Retval> retval;

#ifdef RECENTLY_TEST
printf("*** patch: max:%d, recent-size:%d, named=%d, size=%d, str=%lu\n"
, FsConfig::getRecentlyMax(), m_recently->size()
, m_aNamedList.size(), m_aNodeList.size()
, DFW_SAFE_VALUE(String)
);
#endif

        if( !(FsConfig::getRecentlyMax() < m_recently->size()) ){
            return NULL;
        }

        int pcount = 0;
        int pdepth = m_recently->size() - FsConfig::getRecentlyMax();
        int psize = m_recently->size();
        if( pdepth > 10 ) pdepth = 10;
        for(int k=0; k<psize; k++){
            sp<RecentlyFiles::Time> time = m_recently->m_aTimes.get(k);
            if( !time.has() ) {
#ifdef RECENTLY_TEST_ERROR
printf("*** patch: error: !time.has(), k=%d, psize=%d, pdepth=%d, pcount=%d\n"
, k, psize, pdepth, pcount);
#endif
                continue;
            }

            sp<FsNode> node = time->m_context;
            if( !node.has() ) {
                // FIXME:
#ifdef RECENTLY_TEST_ERROR
printf("*** patch: error: !node.has()\n");
#endif
                continue;
            }

            AutoLock _node(node.get());
            dfw_time_t depth = Time::currentTimeMillis() - node->m_atime;
            if( depth < (1000*5) ){
                continue;
            }

            removeNode(node);

            psize--;
            k--;
            pcount++;

            if( pcount >= pdepth )
                return NULL;
        } // end for(int k=0; k<psize; k++
        return NULL;
    }

}; // end namespace dframework
#endif

