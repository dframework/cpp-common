#ifndef DFRAMEWORK_XML_XMLPARSER_H
#define DFRAMEWORK_XML_XMLPARSER_H


#include <dframework/base/type.h>
#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/util/StringArray.h>


enum dfw_xmlparsertype_t {
    DFW_XMLPARSERTYPE_XML       = 1,
    DFW_XMLPARSERTYPE_HTML      = 2,
};

enum dfw_xmlnodetype_t {
    DFW_XMLNODETYPE_DECLARATION = 1,
    DFW_XMLNODETYPE_TEXT        = 2,
    DFW_XMLNODETYPE_TAG         = 3,
    DFW_XMLNODETYPE_CLOSE_TAG   = 4,
    DFW_XMLNODETYPE_NONE_TAG    = 5,
    DFW_XMLNODETYPE_COMMENT     = 6,
};

enum dfw_xmlattrtype_t {
    DFW_XMLATTRTYPE_NO_CLOSED   = 1,
    DFW_XMLATTRTYPE_CLOSED      = 2,
};

enum dfw_xmlstatus_t {
    DFW_XML_OK           = 0,
    DFW_XML_ERROR        = 1,
    DFW_XML_ESYNTAX      = 2,

    DFW_XML_EMPTY_BUFFER = 1001,
    DFW_XML_NEED_BUFFER  = 1002,
    DFW_XML_FINDNODE     = 1003,
};


// ------------------------------------------------------------------
//
typedef void* dfw_xmlnode_p;
typedef void* dfw_xmlnodes_p;
typedef void* dfw_xml_p;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
};
#endif
//
// ------------------------------------------------------------------


#ifdef __cplusplus
namespace dframework {

    class Xml;

    class XmlAttribute : public Object{
    public:
        String m_sName;
        String m_sValue;

        inline XmlAttribute() {}
        inline virtual ~XmlAttribute() {}

        DFW_OPERATOR_EX_DECLARATION(XmlAttribute, m_sName);
    };

    class XmlNode : public Object
    {
    public:
        dfw_xmlnodetype_t m_iType;
        String m_sText;
        String m_sName;
        Array<XmlAttribute> m_Attributes;
        Array<XmlNode>      m_Childs;

        bool m_bCloseNode;

        XmlNode();
        virtual ~XmlNode();

        String getText();
        String getTrimText();
        String getAttribute(const char* name);
        inline String getAttribute(String& name){
            return getAttribute(name.toChars());
        }
        sp<XmlNode> getChildNode(const char* nodename);
        inline sp<XmlNode> getChildNode(String& nodename){
            return getChildNode(nodename.toChars());
        }
        sp<XmlNode> getChildNode(int position);

        DFW_OPERATOR_EX_DECLARATION(XmlNode, m_sName);
    };

    class XmlNodes : public Object 
    {
    private:
        Array<XmlNode> m_aNodes;

    public:
        XmlNodes();
        virtual ~XmlNodes();

        int size();
        sp<XmlNode> getNode(int position);

        friend class Xml;
    };

    class Xml : public Object {
    private:
        Array<XmlNode> m_NodeList;
        Array<XmlNode> m_OpenList;

    public:
        Xml();
        virtual ~Xml();

        void setOpenedNode(sp<XmlNode>& node);
        void setClosedNode(sp<XmlNode>& node);

        sp<XmlNode> getParentNode();
        dfw_xmlstatus_t append(sp<XmlNode>& node);
        inline Array<XmlNode>* getRoot() { return &m_NodeList; }

        static String text(sp<XmlNode>& node);
        static bool hasNode(sp<XmlNode>& node, const char *name);
        static String getAttribute(sp<XmlNode>& node, const char* name);
        static sp<XmlNodes> search(sp<XmlNode>& node, const char* query);

    private:
        static void search(sp<XmlNodes>& nodes, sp<XmlNode>& node
                         , sp<StringArray>& query, int depth);
        static void search_childs(sp<XmlNodes>& nodes, sp<XmlNode>& node
                         , sp<StringArray>& query, int depth);
 
    };

    class XmlParser {
    private:
        String m_sBuffer;
        dfw_xmlparsertype_t m_Type;
        sp<Xml> m_pResult;

        dfw_xmlstatus_t parseAttributes_real(
                //char **name, dfw_size_t *o_nlen,
                //char **val, dfw_size_t *o_vlen,
                String& sAttrName, String& sAttrValue,
                dfw_size_t *, 
                const char *, dfw_size_t, String& sName, int isTest);
        dfw_xmlstatus_t parseEndedAttributes(
                sp<XmlNode>& node, const char *, dfw_size_t, String& sName);
        dfw_xmlstatus_t parseUnknownAttributes(
                sp<XmlNode>& o_node, dfw_size_t *o_size, 
                const char *, dfw_size_t, String& sName);
        dfw_xmlstatus_t parseDeclaration(sp<XmlNode>&, dfw_size_t *);
        dfw_xmlstatus_t parseTag(sp<XmlNode>&, dfw_size_t *);
        dfw_xmlstatus_t parseCloseTag(sp<XmlNode>&, dfw_size_t *);
        dfw_xmlstatus_t parseCommentTag(sp<XmlNode>&, dfw_size_t *);
        dfw_xmlstatus_t parseNoneTag(sp<XmlNode>&, dfw_size_t *);
        dfw_xmlstatus_t parseText(sp<XmlNode>&, dfw_size_t *);

    public:
        XmlParser();
        virtual ~XmlParser();

        void setXmlParserType(dfw_xmlparsertype_t type);
        void cleanResult();

        dfw_size_t findCloseTag(int *olen, const char *buf, dfw_size_t size);
        dfw_size_t findCharactor(const char *buf);
        dfw_xmlattrtype_t testAttributes(
                dfw_size_t *closeTag, int *closeLen,
                const char *, dfw_size_t, String& sName);

        dfw_xmlstatus_t parseBuffer(const char *buffer, dfw_size_t size);
        dfw_xmlstatus_t parseBuffer();

        inline Array<XmlNode>* getResult(){
            return (m_pResult.has() ? m_pResult->getRoot() : NULL);
        }
    };

};
#endif

#endif /* DFRAMEWORK_UTIL_XMLPARSER_H */

