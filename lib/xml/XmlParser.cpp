#include <dframework/xml/XmlParser.h>
#include <dframework/util/Regexp.h>
#include <dframework/util/StringArray.h>

//#define DFW_XML_DEBUG

namespace dframework {

    XmlNode::XmlNode() :
            m_bCloseNode(false)
    {
    }

    XmlNode::~XmlNode(){
    }

    String XmlNode::getText(){
        sp<XmlNode> thiz = this;
        return Xml::text(thiz);
    }

    String XmlNode::getTrimText(){
        String ret = getText();
        return ret.trim();
    }

    String XmlNode::getAttribute(const char* name){
        String ret;
        for(int k=0; k<m_Attributes.size(); k++){
            sp<XmlAttribute> attr = m_Attributes.get(k);
            if(attr->m_sName.equals(name))
                return attr->m_sValue;
        }
        return ret;
    }

    sp<XmlNode> XmlNode::getChildNode(const char* name){
        for(int k=0; k<m_Childs.size(); k++){
            sp<XmlNode> node = m_Childs.get(k);
            if(node->m_sName.equals(name))
                return node;
        }
        return NULL;
    }

    sp<XmlNode> XmlNode::getChildNode(int position){
        return m_Childs.get(position);
    }


    // -----------------------------------------------------------------

    XmlParser::XmlParser() :
            m_Type(DFW_XMLPARSERTYPE_XML)
    {
    }

    XmlParser::~XmlParser(){
    }

    void XmlParser::setXmlParserType(dfw_xmlparsertype_t type){
        m_Type = type;
    }

    void XmlParser::cleanResult(){
        m_sBuffer = "";
        m_pResult = NULL;
    }

    dfw_xmlstatus_t XmlParser::parseBuffer(const char *buffer, dfw_size_t size)
    {
        m_sBuffer.append(buffer, size);
        return parseBuffer();
    }

    dfw_size_t XmlParser::findCharactor(const char *buf){
        Regexp regexp("(^[\\s]+)");
        sp<Retval> retval =regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)) return 0;
        return regexp.getMatchLength(1);
    }

    dfw_size_t XmlParser::findCloseTag(
            int *olen, const char *buf, dfw_size_t size)
    {
        dfw_size_t k;
        for(k=0; k<size; k++){
            char ch = buf[k];
            if(' '==ch||'\t'==ch||'\r'==ch||'\n'==ch){
                continue;
            }else if('/'==ch && k<(size-1) && '>'==buf[k+1] ){
                *olen = 2;
                return k;
            }else if('>'==ch){
                *olen = 1;
                return k;
            }else{
                return -1;
            }
        }
        return -1;
    }

    dfw_xmlattrtype_t XmlParser::testAttributes(
            dfw_size_t *closeTag, int *closeLen,
            const char *buf, dfw_size_t size, String& sName)
    {
        const char *p = buf;
        do{
            if(0==size) {
                break;
            }

            String sAttrName, sAttrValue;
            dfw_size_t oize = 0;

            parseAttributes_real(sAttrName, sAttrValue
                            , &oize, p, size, sName, 1);

            if(oize){
                p += oize;
                size -= oize;
            }else{
                break;
            }

        }while(true);

        dfw_size_t uclosetag;
        int ucloselen = 0;
        uclosetag = findCloseTag(&ucloselen, p, size);

        if(((dfw_size_t)-1)!=uclosetag){
             uclosetag += (p-buf);
             if(closeLen && closeTag){
                 *closeLen = ucloselen;
                 *closeTag = uclosetag;
             }
             return DFW_XMLATTRTYPE_CLOSED;
        }

        return DFW_XMLATTRTYPE_NO_CLOSED;
    }

    dfw_xmlstatus_t XmlParser::parseAttributes_real(
            String& sAttrName, String& sAttrValue,
            dfw_size_t *o_size, 
            const char *buf, dfw_size_t size, String& sName, int isTest)
    {
        DFW_UNUSED(sName);
        DFW_UNUSED(isTest);

        sp<Retval> retval;
        *o_size = 0;
        if(!buf){
            return DFW_XML_OK;
        }

        Regexp regexp("^[\\s]*([\\-\\_\\:a-zA-Z0-9]*)[\\s]*=[\\s]*");
        retval = regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)){
            Regexp exp2("^[\\s]*([\\-\\_\\:a-zA-Z0-9]*)");
            retval = exp2.regexp(buf);
            if(DFW_RETVAL_H(retval)){
                return DFW_XML_OK;
            }
            sAttrName.set(exp2.getMatch(1), exp2.getMatchLength(1));
            *o_size = exp2.getMatchLength(0);
            return DFW_XML_OK;
        }

        dfw_size_t outsize = 0;
        dfw_size_t length = regexp.getMatchLength(0);
        String sVal;
        if(size-length>0){
            bool hasQ = false;
            int lastQ = 0;
            char startQ = -1;
            const char *val = buf+length;
            dfw_size_t last = size-length;
            dfw_size_t toff = 0;
            dfw_size_t k;
            if('\"'==val[0] || '\''==val[0]){
                startQ = val[0];
                toff++;
            }
            for(k=toff; k<last; k++){
                char ch = val[k];
                if(hasQ) {
                    hasQ = false;
                }else if(ch=='\\'){
                    hasQ = true;
                }else if( -1==startQ ){
                    if(' '==ch||'\t'==ch||'\r'==ch||'\n'==ch||'>'==ch){
                        break;
                    }else if( '/'==ch && k<(size-1) && '>'==buf[k+1] ){
                        break;
                    }
                }else if(startQ==ch){
                    k++;
                    lastQ = 1;
                    break;
                }
            }
            sAttrValue.set( val+toff, k-toff-lastQ ); 
            outsize = k;
        } // if[size-length>0]{

        sAttrName.set( regexp.getMatch(1), regexp.getMatchLength(1) ); 
        *o_size = length + outsize;
        return DFW_XML_OK;
    }

    dfw_xmlstatus_t XmlParser::parseEndedAttributes(
            sp<XmlNode>& node, const char *buf, dfw_size_t size, String& sName)
    {
        const char *p = buf;
        do{
            if(0==size) {
                break;
            }

            String sAttrName, sAttrValue;
            dfw_size_t oize = 0;

            parseAttributes_real(sAttrName, sAttrValue
                            , &oize, p, size, sName, 0);
            if( !sAttrName.empty() ){
                sp<XmlAttribute> attr = new XmlAttribute();
                attr->m_sName = sAttrName;
                if( !sAttrValue.empty() ){
                    attr->m_sValue = sAttrValue;
                }

//printf("%s : n=[%s], v=[%s]\n", sName.toChars(), attr->m_sName.toChars(), attr->m_sValue.toChars());

                node->m_Attributes.insert(attr);
            }

            if(oize){
                p += oize;
                size -= oize;
            }else{
                break;
            }

        }while(true);

        return DFW_XML_OK;
    }

    dfw_xmlstatus_t XmlParser::parseUnknownAttributes(
            sp<XmlNode>& o_node, dfw_size_t *o_size, 
            const char *buf, dfw_size_t size, String& sName)
    {
        DFW_UNUSED(o_node);

        dfw_xmlstatus_t status;
        dfw_size_t closeTag = 0;
        int closeLen = 0;

        if( DFW_XMLATTRTYPE_NO_CLOSED == testAttributes(
                &closeTag, &closeLen, buf, size, sName) ){
            return DFW_XML_NEED_BUFFER;
        }

        sp<XmlNode> node = new XmlNode();
        if( (status = parseEndedAttributes(node, buf, size, sName) )){
            return status;
        }

        //if(use_out_node){
            if( closeLen>1 /* "/>" */ ){
                node->m_bCloseNode = true;
            }
            o_node = node;
        //}

        if(o_size){
            *o_size = closeTag + closeLen;
        }

        return DFW_XML_OK;
    }

    dfw_xmlstatus_t XmlParser::parseDeclaration(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        dfw_xmlstatus_t status;
        const char *buf = m_sBuffer.toChars();
        Regexp regexp("^<\\?([\\-\\_\\:a-zA-Z0-9]*)(\\s*[\\s\\S]*?)\\?>");
        sp<Retval> retval = regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)){
            return DFW_XML_OK;
        }

        sp<XmlNode> node = new XmlNode();
        node->m_iType = DFW_XMLNODETYPE_DECLARATION;
        node->m_sText.set(regexp.getMatch(0), regexp.getMatchLength(0));
        node->m_sName.set(regexp.getMatch(1), regexp.getMatchLength(1));

        if( (status = parseEndedAttributes(
                node
              , regexp.getMatch(2)
              , regexp.getMatchLength(2)
              , node->m_sName )) ){
            return status;
        }

#ifdef DFW_XML_DEBUG
printf("parse declaration nodename=%s\r\n", node->m_sName.toChars());
#endif
        o_node = node;
        *o_size = regexp.getMatchLength(0);
        return DFW_XML_FINDNODE;
    }


    dfw_xmlstatus_t XmlParser::parseTag(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        dfw_xmlstatus_t status;
        const char *buf = m_sBuffer.toChars();
        Regexp regexp("^<([\\-\\_\\:a-zA-Z0-9]+)");
        sp<Retval> retval =regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)) return DFW_XML_OK;

        String sName;
        sName.set(regexp.getMatch(1), regexp.getMatchLength(1));
        dfw_size_t ssize = regexp.getMatchLength(0);
        dfw_size_t testoffset = 0;
        if( (status = parseUnknownAttributes(o_node, &testoffset, 
                buf+ssize, m_sBuffer.length()-ssize, sName)) )
        {
            return status;
        }

        sp<XmlNode> node = o_node;
        node->m_iType = DFW_XMLNODETYPE_TAG;
        node->m_sName = sName;

#if 0
if(node->m_sName.equals("D:multistatus")){
printf("%d, => %d\r\n", node->m_iType, DFW_XMLNODETYPE_TAG);
}
#endif

#ifdef DFW_XML_DEBUG
printf("parse-tag nodename=%s\r\n", node->m_sName.toChars());
#endif

        *o_size = ssize + testoffset;
        return DFW_XML_FINDNODE;
    }

    dfw_xmlstatus_t XmlParser::parseCloseTag(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        dfw_xmlstatus_t status;
        const char *buf = m_sBuffer.toChars();
        Regexp regexp("^</([\\-\\_\\:a-zA-Z0-9]+)");
        sp<Retval> retval =regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)) return DFW_XML_OK;

        String sName;
        sName.set(regexp.getMatch(1), regexp.getMatchLength(1));
        dfw_size_t ssize = regexp.getMatchLength(0);
        dfw_size_t testoffset = 0;
        if( (status = parseUnknownAttributes(o_node, &testoffset, 
                buf+ssize, m_sBuffer.length()-ssize, sName)) )
        {
            return status;
        }

        sp<XmlNode> node = o_node;
        node->m_iType = DFW_XMLNODETYPE_CLOSE_TAG;
        node->m_sName = sName;

#ifdef DFW_XML_DEBUG
printf("close-tag nodename=%s\r\n", node->m_sName.toChars());
#endif

        *o_size = ssize + testoffset;
        return DFW_XML_FINDNODE;
    }

    dfw_xmlstatus_t XmlParser::parseCommentTag(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        //FIXME: unused: dfw_xmlstatus_t status;
        const char *buf = m_sBuffer.toChars();
        Regexp regexp("^<!--([\\s\\S]*?)-->");
        sp<Retval> retval =regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)) return DFW_XML_OK;

        sp<XmlNode> node = new XmlNode();
        node->m_iType = DFW_XMLNODETYPE_COMMENT;
        node->m_sText.set(regexp.getMatch(0), regexp.getMatchLength(0));
        node->m_sName.set("#comment", 8);

#ifdef DFW_XML_DEBUG
printf("comment nodename=%s\r\n", node->m_sName.toChars());
#endif
        o_node = node;
        *o_size = regexp.getMatchLength(0);

        return DFW_XML_FINDNODE;
    }

    dfw_xmlstatus_t XmlParser::parseNoneTag(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        DFW_UNUSED(o_node);

        const char *buf = m_sBuffer.toChars();
        Regexp regexp("^<!([\\-\\_a-zA-Z0-9]*)([\\s\\S]*?)>");
        sp<Retval> retval =regexp.regexp(buf);
        if(DFW_RETVAL_H(retval)) return DFW_XML_OK;

        sp<XmlNode> node = new XmlNode();
        node->m_iType = DFW_XMLNODETYPE_NONE_TAG;
        node->m_sName.set(regexp.getMatch(1), regexp.getMatchLength(1));
        node->m_sText.set(regexp.getMatch(2), regexp.getMatchLength(2));

#ifdef DFW_XML_DEBUG
printf("none-tag nodename=%s\r\n", node->m_sName.toChars());
#endif

        *o_size = regexp.getMatchLength(0);
        return DFW_XML_FINDNODE;
    }


    dfw_xmlstatus_t XmlParser::parseText(
            sp<XmlNode>& o_node, dfw_size_t *o_size)
    {
        const char *buf = m_sBuffer.toChars();
        const char *p = strstr(buf, "<");
        if(buf==p){
            return DFW_XML_OK;
        }

        if(NULL==p){
            *o_size = m_sBuffer.length();
        }else{
            *o_size = p-buf;
        }

        sp<XmlNode> node = new XmlNode();
        node->m_iType = DFW_XMLNODETYPE_TEXT;
        node->m_sText.set(buf, *o_size);
        node->m_sName.set("#text", 5);

#ifdef DFW_XML_DEBUG
//printf("nodename=%s\r\n", node->m_sName.toChars());
//printf("text=[%s]\r\n", node->m_sText.toChars());
#endif

        o_node = node;
        return DFW_XML_FINDNODE;
    }

    dfw_xmlstatus_t XmlParser::parseBuffer()
    {
        dfw_xmlstatus_t status;

        if(!m_pResult.has()){
            m_pResult = new Xml();
        }

        if(m_sBuffer.empty()){
            return DFW_XML_EMPTY_BUFFER;
        }

        do{
            dfw_size_t outsize = 0;
            sp<XmlNode> node = NULL;

            if(m_sBuffer.empty()){
                return DFW_XML_OK;
            }

            if( (status = parseDeclaration(node, &outsize)) ){
            }else if( (status = parseTag(node, &outsize)) ){
            }else if( (status = parseCloseTag(node, &outsize)) ){
            }else if( (status = parseCommentTag(node, &outsize)) ){
            }else if( (status = parseNoneTag(node, &outsize)) ){
            }else if( (status = parseText(node, &outsize)) ){
            }else{
                return DFW_XML_ESYNTAX;
            }

            if(DFW_XML_FINDNODE!=status){
                return status;
            }

            if(!outsize || !node.has()){
                return DFW_XML_ERROR;
            }

            m_sBuffer.shift(outsize);

            if( (status = m_pResult->append(node)) ){
                return status;
            }

        }while(true);

        return DFW_XML_OK;
    }

    // ---------------------------------------------------------------

    XmlNodes::XmlNodes(){
    }

    XmlNodes::~XmlNodes(){
    }

    int XmlNodes::size(){
        AutoLock _l(this);
        return m_aNodes.size();
    }

    sp<XmlNode> XmlNodes::getNode(int position){
        AutoLock _l(this);
        return m_aNodes.get(position);
    }

    // ---------------------------------------------------------------

    Xml::Xml(){
    }

    Xml::~Xml(){
    }

    sp<XmlNode> Xml::getParentNode(){
        int size = m_OpenList.size();
        if(0==size){
            return NULL;
        }
        return m_OpenList.get(size-1);
    }

    void Xml::setOpenedNode(sp<XmlNode>& node)
    {
        m_OpenList.insert(node);
    }

    void Xml::setClosedNode(sp<XmlNode>& node)
    {
        int find = -1;
        int size = m_OpenList.size();
        for(int k=size-1; k>=0; k--){
            sp<XmlNode> dst = m_OpenList.get(k);
            if( dst->m_sName.equals(node->m_sName) ){
                find = k;
                break;
            }
        }
        if(-1==find){
            return;
        }
        for(int k=size-1; k>=find; k--){
            sp<XmlNode> dst = m_OpenList.remove(k);
#ifdef DFW_XML_DEBUG
printf("size=%d, k=%d, node=%s, dst=%s\r\n", size, k, node->m_sName.toChars(), dst->m_sName.toChars());
#endif
        }
    }

    dfw_xmlstatus_t Xml::append(sp<XmlNode>& node)
    {
#ifdef DFW_XML_DEBUG
if(node->m_bCloseNode){
printf("closenode nodename=[%s]\r\n", node->m_sName.toChars());
}
#endif
        sp<XmlNode> parent = getParentNode();
        if(parent.has()){
            parent->m_Childs.insert(node);
        }else{
            m_NodeList.insert(node);
        }

        if(node->m_iType==DFW_XMLNODETYPE_TAG && !node->m_bCloseNode){
            setOpenedNode(node);
        }else if(node->m_iType==DFW_XMLNODETYPE_CLOSE_TAG){
            setClosedNode(node);
        }

        return DFW_XML_OK;
    }

    DFW_STATIC
    String Xml::text(sp<XmlNode>& node){
        String ret;
        if(!node.has()){
            return ret;
        }
        int size = node->m_Childs.size();
        for(int k=0; k<size; k++){
            sp<XmlNode> chd = node->m_Childs.get(k);
            if(chd->m_iType==DFW_XMLNODETYPE_TEXT){
                ret.append(chd->m_sText.toChars(), chd->m_sText.length());
            }else if(chd->m_iType==DFW_XMLNODETYPE_TAG){
                String test = Xml::text(chd);
                ret.append(test.toChars(), test.length());
            }
        }
        return ret;
    }

    DFW_STATIC
    bool Xml::hasNode(sp<XmlNode>& node, const char *name){
        if(!node.has()){
            return false;
        }
        Array<XmlNode> *childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> chd = childs->get(k);
            if( chd->m_iType==DFW_XMLNODETYPE_TAG
                    && chd->m_sName.equals(name)){
                return true;
            }
        }
        return false;
    }

    DFW_STATIC
    String Xml::getAttribute(sp<XmlNode>& node, const char* name){
        String ret;
        if(!node.has()) return ret;
        for(int k=0; k<node->m_Attributes.size(); k++){
            sp<XmlAttribute> attr = node->m_Attributes.get(k);
            if(attr->m_sName.equals(name))
                return attr->m_sValue;
        }
        return ret;
    }

    DFW_STATIC
    sp<XmlNodes> Xml::search(sp<XmlNode>& node, const char* query){
        sp<Retval> retval;
        sp<XmlNodes> nodes = new XmlNodes();
        sp<StringArray> sa = new StringArray();

        if( DFW_RET(retval, sa->split(query, " ")) ){
            return nodes;
        }
        if( 0==sa->size() ){
            return nodes;
        }
        search(nodes, node, sa, 0);
        return nodes;
    }

    DFW_STATIC DFW_PRIVATE
    void Xml::search(sp<XmlNodes>& nodes, sp<XmlNode>& node
                   , sp<StringArray>& query, int depth)
    {
        sp<Retval> retval;
        if( !(depth < query->size()) )
            return;
        bool isattr = false;
        bool isfind = false;
        String sAttrName, sAttrValue;
        sp<String> sCur = query->getString(depth);
        char* p = (char*)sCur->toChars();
        char* test_opt = strstr(p, "[");
        if(test_opt){
            Regexp regexp("\\[([\\-\\_\\:a-zA-Z0-9]+)='[\\'\\\"]*([\\S]+)[\\'\\\"]*\\]");
            if( !DFW_RET(retval, regexp.regexp(p)) ){
                isattr = false;
                sAttrName.set(regexp.getMatch(1), regexp.getMatchLength(1));
                sAttrValue.set(regexp.getMatch(2), regexp.getMatchLength(2));
            }
            test_opt[0] = '\0';
        }

        if( p[0] == '#' ){
            const char* pn = p+1;
            String sId = node->getAttribute("id");
            if(sId.equals(pn))
                isfind = true;
        }else if( p[0] == '.' ){
            const char* pn = p+1;
            String sClass = node->getAttribute("class");
            if(sClass.equals(pn))
                isfind = true;
        }else{
            if(node->m_sName.equals(p))
                isfind = true;
        }

        if(isfind){
            if( isattr && sAttrName.length() ){
                String sTest = node->getAttribute(sAttrName);
                if(!sTest.equals(sAttrValue)){
                    isfind = false;
                }
            }
        }

        if(isfind){
            bool islast = ((query->size()-1)==depth) ? true : false;
            if(islast)
                nodes->m_aNodes.insert(node);
            else
                search_childs(nodes, node, query, depth+1);
            if(depth!=0){
                search_childs(nodes, node, query, 0);
            }
        }else{
            search_childs(nodes, node, query, 0);
        }
    }

    DFW_STATIC DFW_PRIVATE
    void Xml::search_childs(sp<XmlNodes>& nodes, sp<XmlNode>& node
                   , sp<StringArray>& query, int depth)
    {
        Array<XmlNode>* childs = &node->m_Childs;
        for(int k=0; k<childs->size(); k++){
            sp<XmlNode> child = childs->get(k);
            if( child->m_iType == DFW_XMLNODETYPE_TAG)
                search(nodes, child, query, depth);
        }
    }

};

