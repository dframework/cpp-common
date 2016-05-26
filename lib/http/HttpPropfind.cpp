#include <dframework/http/HttpPropfind.h>
#include <dframework/util/Time.h>
#include <dframework/lang/Long.h>

namespace dframework{

    HttpPropfind::Prop::Prop(){
        m_iHttpStatus = 0;
        m_iExclusiveLock = 0;
        m_iSharedLock = 0;
        m_bDir = false;
        m_bFile = false;
    }

    HttpPropfind::Prop::~Prop(){
    }

    HttpPropfind::HttpPropfind(){
    }

    HttpPropfind::~HttpPropfind(){
    }

    sp<Retval> HttpPropfind::query(const URI& uri, const char* /*method*/){
        sp<Retval> retval;
        m_sPath = uri.getPath();
        const char* p = m_sPath.toChars();
        if(m_sPath.empty())
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "path is empty.");
        if( p[m_sPath.length()-1] != '/' ){
            m_sPath.append("/",1);
        }
        return DFW_RET_C(retval, HttpQuery::query(uri, "PROPFIND"));
    }

    sp<Retval> HttpPropfind::query(const char* uri, const char* /*method*/){
        sp<Retval> retval;
        URI oUri = uri;
        return DFW_RET_C(retval, query(oUri, "PROPFIND"));
    }

    // --------------------------------------------------------------

    sp<Retval> HttpPropfind::propfindSupportedlock(
            sp<Prop>& prop
          , Prop::lockentry *ple
          , sp<XmlNode>& node)
    {
        if(!node.has()) return NULL;

        Array<XmlNode> *childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> chd = childs->get(k);
            if(chd->m_iType!=DFW_XMLNODETYPE_TAG){
                continue;
            }
            if(chd->m_sName.equals("D:lockentry")){
                Prop::lockentry le;
                le.lockscope = 0;
                le.locktype = 0;
                propfindSupportedlock(prop, &le, chd);
                if(le.lockscope & DFW_DAV_LOCKSCOPE_EXCLUSIVE){
                    prop->m_iExclusiveLock = le.locktype;
                }
                if(le.lockscope & DFW_DAV_LOCKSCOPE_SHARED){
                    prop->m_iSharedLock = le.locktype;
                }
            }else if(chd->m_sName.equals("D:lockscope")){
                if( ple && Xml::hasNode(chd, "D:exclusive") ){
                    ple->lockscope = DFW_DAV_LOCKSCOPE_EXCLUSIVE;
                }else if( ple && Xml::hasNode(chd, "D:shared") ){
                    ple->lockscope = DFW_DAV_LOCKSCOPE_SHARED;
                }
            }else if(chd->m_sName.equals("D:locktype")){
                if( ple && Xml::hasNode(chd, "D:write") ){
                    ple->locktype = DFW_DAV_LOCKTYPE_WRITE;
                }
            }
        }
        return NULL;
    }

    sp<Retval> HttpPropfind::propfindProp(
            sp<Prop>& prop, sp<XmlNode>& node)
    {
        if(!node.has()) return NULL;

        Array<XmlNode> *childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> chd = childs->get(k);
            if(chd->m_iType!=DFW_XMLNODETYPE_TAG){
                continue;
            }
            if(chd->m_sName.equals("lp1:resourcetype")){
                if( Xml::hasNode(chd, "D:collection") ){
                    prop->m_bDir = true;
                }
            }else if(chd->m_sName.equals("lp1:creationdate")){
                String date = Xml::text(chd);
                Time::iso8601(&prop->m_CreateDate, date.toChars());
            }else if(chd->m_sName.equals("lp1:getlastmodified")){
                String modified = Xml::text(chd);
                Time::parse(&prop->m_LastModified, modified.toChars());
            }else if(chd->m_sName.equals("lp1:getetag")){
                prop->m_sETag = Xml::text(chd);
            }else if(chd->m_sName.equals("D:getcontenttype")){
                prop->m_sContentType = Xml::text(chd);
            }else if(chd->m_sName.equals("D:supportedlock")){
                propfindSupportedlock(prop, NULL, chd);
            }else if(chd->m_sName.equals("lp1:getcontentlength")){
                prop->m_uContentLength = Long::parseLong(Xml::text(chd));
            }else if(chd->m_sName.equals("lp2:executable")){
                if( Xml::text(chd).equals("F") ){
                    prop->m_bFile = true;
                }
            }
        }
        return NULL;
    }

    sp<Retval> HttpPropfind::propfindStat(
            sp<Prop>& prop, sp<XmlNode>& node)
    {
        sp<Retval> retval;
        if(!node.has()) return NULL;

        Array<XmlNode> *childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> chd = childs->get(k);
            if(chd->m_iType!=DFW_XMLNODETYPE_TAG){
                continue;
            }
            if(chd->m_sName.equals("D:status")){
                String hstatus = Xml::text(chd);
                Regexp exp("^([a-zA-Z]+)\\/([0-9]+)\\.([0-9]+)[\\s]+([0-9]+)[\\s]+([\\s\\S]*)");
                retval = exp.regexp(hstatus.toChars());
                if(DFW_RETVAL_H(retval)){
                    prop->m_iHttpStatus = 0;
                }else{
                    String sistatus;
                    sistatus.set(exp.getMatch(4), exp.getMatchLength(4));
                    prop->m_iHttpStatus = Integer::parseInt(sistatus);
                }
            }else if(chd->m_sName.equals("D:prop")){
                propfindProp(prop, chd);
            }
        }
        return NULL;
    }

    sp<Retval> HttpPropfind::propfindResponse(
            sp<HttpConnection>& conn, sp<XmlNode>& node)
    {
        if(!node.has()) return NULL;

        sp<Prop> prop = new Prop();

        Array<XmlNode> *childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> chd = childs->get(k);
            if(chd->m_iType!=DFW_XMLNODETYPE_TAG){
                continue;
            }
            if(chd->m_sName.equals("D:href")){
                prop->m_sName = Xml::text(chd);
                prop->m_sName.trim();
                if( !m_sPath.empty() ){
                    if( 0==prop->m_sName.indexOf(m_sPath.toChars()) ){
                        prop->m_sName.shift(m_sPath.length());
                    }
                }
                if(prop->m_sName.length()>0){
                    const char *path = prop->m_sName.toChars();
                    if(path[prop->m_sName.length()-1] == '/'){
                        prop->m_sName.chuck(prop->m_sName.length()-1);
                    }
                }
                if(prop->m_sName.length()==0){
                    prop->m_sName = ".";
                }
            }else if(chd->m_sName.equals("D:propstat")){
                propfindStat(prop, chd);
            }
        }

        return onPropfind(conn, prop);
    }

    sp<Retval> HttpPropfind::propfindComplete(
            sp<HttpConnection>& conn, Array<XmlNode> *list)
    {
        if(!list) return NULL;

        for(int k=0; k<list->size(); k++){
            sp<XmlNode> node = list->get(k);
            if(node->m_iType!=DFW_XMLNODETYPE_TAG){
                continue;
            }
            if(node->m_sName.equals("D:response")){
                propfindResponse(conn, node);
            }else if(node->m_sName.equals("D:multistatus")){
                propfindComplete(conn, &node->m_Childs);
            }
        }

        return NULL;
    }

    sp<Retval> HttpPropfind::onRequest(sp<HttpConnection>& c){
        sp<Retval> retval;
        if(DFW_RET(retval, HttpQuery::onRequest(c)))
            return DFW_RETVAL_D(retval);

        m_XmlParser.cleanResult();

        return NULL;
    }

    sp<Retval> HttpPropfind::onResponse(
            sp<HttpConnection>& c, const char *b, dfw_size_t s)
    {
        sp<Retval> retval;
        if(DFW_RET(retval, HttpQuery::onResponse(c, b, s)))
            return DFW_RETVAL_D(retval);

        if(!b && !s)
            return propfindComplete(c, m_XmlParser.getResult());

        dfw_xmlstatus_t xmlstatus = m_XmlParser.parseBuffer(b, s);
        switch(xmlstatus){
        case DFW_XML_OK:
            return propfindComplete(c, m_XmlParser.getResult());

        case DFW_XML_EMPTY_BUFFER:
        case DFW_XML_NEED_BUFFER:
            return NULL;
        default:
            break;
        }

        return DFW_RETVAL_NEW(DFW_ERROR,0);
    }

};

