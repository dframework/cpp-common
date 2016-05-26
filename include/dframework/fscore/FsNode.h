#ifndef DFRAMEWORK_IO_FSNODE_H
#define DFRAMEWORK_IO_FSNODE_H

#include <dframework/util/ArraySorted.h>
#include <dframework/base/Condition.h>
#include <dframework/lang/String.h>
#include <dframework/io/RecentlyFiles.h>
#include <sys/stat.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FsNodeList;

    class FsNode : public Object {
    public:
        DFW_DEFINITION_SAFE_COUNT(FsNode);

    public:
        uint64_t        m_nodeid;
        String          m_name;
        dfw_time_t      m_atime;
        uint64_t        m_nlookup;

    public:
        FsNode(uint64_t nodeid);
        FsNode(const char* name);
        FsNode(const char* name, uint64_t nodeid);
        virtual ~FsNode();

    private:
        void init();

    public:
        DFW_OPERATOR_EX_DECLARATION(FsNode, m_nodeid);

        friend class FsNodeList;
    };

    class FsNamedNode : public Object
    {
    private:
        sp<FsNode> m_node;

    public:
        FsNamedNode(sp<FsNode>& node){ m_node = node; }
        virtual ~FsNamedNode(){}

        DFW_OPERATOR_EX_DECLARATION(FsNamedNode, m_node->m_name);

    public:
        friend class FsNodeList;
    };

    // --------------------------------------------------------------

    class FsNodeList : public Object
    {
    private:
        uint32_t  m_generation;
        uint64_t  m_nextid;

        sp<FsNode>               m_rootNode;
        ArraySorted<FsNode>      m_aNodeList;
        ArraySorted<FsNamedNode> m_aNamedList;
        sp<RecentlyFiles>        m_recently;

        Object    m_nextid_safe;
        Object    m_generation_safe;

    protected:
        sp<Retval> init();

    public:
        FsNodeList();
        virtual ~FsNodeList();

        uint32_t   generation();

        sp<Retval> getNode(sp<FsNode>& node, uint64_t nodeid);
        sp<Retval> getNode(sp<FsNode>& node, const char* path);

        sp<Retval> insertNode(sp<FsNode>& out, const char* path);
        void       removeNode(sp<FsNode>& node);
        void       removeNode(const char* path);
        sp<FsNode> makeRootNode();
        void       updateAttr(sp<FsNode>& node);

        sp<Retval> forget(uint64_t nodeid, uint64_t lookup);


    private:
        uint64_t   nextid();
        sp<FsNode> _findNode(uint64_t nodeid);

        sp<Retval> patchRecently();

    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_IO_FSNODE_H */

