#include <dframework/net/URI.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>

namespace dframework {

    String URI::PathInfo::path(int no) const{
        sp<String> val = m_aPaths.get(no);
        if(val.has())
            return *val;
        return NULL;
    }

    String URI::PathInfo::fullpath(int no) const{
        String rpath;
        int max = ( (m_aPaths.size() < (no+1))
                   ? m_aPaths.size()
                   : (no+1));
        for(int k=0; k<max; k++){
            sp<String> val = m_aPaths.get(k);
            if( 0==k && val->empty() ){
                //FIXME: rpath = "/";
            }else if(k>0){
                rpath.appendFmt("/%s", val->toChars());
            }
        }
        return rpath;
    };

    void URI::PathInfo::parse(const char *path, int len){
        int off = 0;
        bool buslash = false;
        bool reslash = false;
        char temp[1024];
        int tk = 0;
        sp<String> name;
        if(-1==len) len = strlen(path);
        if(len>1023) len = 1023;
        while(off<len){
            if(!buslash && path[off]=='\\'){
                buslash = true;
                ++off;
                continue;
            }

            if((!buslash&&'/'==path[off]) && !reslash){
                reslash = true;
                name = new String(temp, tk);
                m_aPaths.insert(name);
                tk = 0;
            }else if((!buslash&&'/'==path[off]) && reslash){
            }else if(reslash){
                reslash = false;
                temp[tk++] = path[off];
            }else{
                if(buslash){
                    temp[tk++] = '\\';
                    buslash = false;
                }
                temp[tk++] = path[off];
            }

            if(buslash){
                buslash = false;
            }
            ++off;
        }

        if(tk>0){
            name = new String(temp,tk);
            m_aPaths.insert(name);
        }

        m_sPath = "";
        m_sWinPath = "";
        m_sPrefixPath = "";
        m_sWinPrefixPath = "";
        m_sName = "";
        int s = m_aPaths.size();

        if(s>0){
            sp<String> p = m_aPaths.get(0);
            if(p->empty()){
                m_sPath.append("/", 1);
                m_sPrefixPath.append("/", 1);
                m_sWinPath.append("\\", 1);
                m_sWinPrefixPath.append("\\", 1);
            }else{
                m_sPath.append(p->toChars(), p->length());
                m_sWinPath.append(p->toChars(), p->length());
                char ch = p->toChars()[p->length()-1];
                if(':'==ch){
                    m_sWinPath.append("\\\\", 2);
                    m_sWinPrefixPath.append("\\\\", 2);
                }
                if(s>1){
                    m_sPath.append("/", 1);
                    m_sPrefixPath.append("/", 1);
                }
            }
            for(int k=1; k<s; k++){
                p = m_aPaths.get(k);
                m_sPath.append(p->toChars(), p->length());
                m_sWinPath.append(p->toChars(), p->length());
                if(k<(s-1)){
                    m_sPath.append("/", 1);
                    m_sWinPath.append("\\", 1);
                    m_sPrefixPath.append(p->toChars(), p->length());
                    m_sWinPrefixPath.append(p->toChars(), p->length());
                    if(k<(s-2)){
                        m_sPrefixPath.append("/", 1);
                        m_sWinPrefixPath.append("\\", 1);
                    }
                }else{
                    m_sName = p->toChars();
                }
            }
        }
    }

    // -------------------------------------------------------------

    URI::FileInfo::FileInfo() : m_bUse(false) {
    }

    void URI::FileInfo::clean(){
        m_bUse = false;
        m_sDirname = "";
        m_sFilename = "";
        m_sName = "";
        m_sExtension = "";
    }

    void URI::FileInfo::parse(const char *path){
        clean();
        m_bUse = true;
        if(!path){
            return;
        }
        dfw_size_t length = strlen(path);
        dfw_size_t off = 0;
        dfw_size_t last = 0;
        bool buslash = false;
        while(off<length){
            if(!buslash && path[off]=='\\'){
                buslash = true;
            }else if(buslash){
                buslash = false;
            }else if('/'==path[off]){
                last = off;
            }
            ++off;
        }
        if(0==last){
            m_sDirname = "";
            m_sFilename = path;
        }else{
            m_sDirname.set(path, last);
            m_sFilename = path+last+1;
        }
        int lidx = m_sFilename.lastIndexOf('.');
        if(-1==lidx){
            m_sName = m_sFilename;
            m_sExtension = "";
        }else if(0==lidx){
            m_sName = "";
            m_sExtension = m_sFilename;
        }else{
            m_sName.set(m_sFilename.toChars(), lidx);
            m_sExtension = m_sFilename.toChars()+lidx+1;
        }
    }

    String URI::FileInfo::path(){
        URI::PathInfo path;
        path.set(m_sDirname.toChars(), m_sDirname.length());
        return path.path();
    }

    String URI::FileInfo::winPath(){
        URI::PathInfo path;
        path.set(m_sDirname.toChars(), m_sDirname.length());
        return path.winPath();
    }

    URI::FileInfo& URI::FileInfo::operator=(const char *path){
        set(path);
        return (*this);
    }

    URI::FileInfo& URI::FileInfo::operator=(const FileInfo &info){
        clean();
        m_bUse = info.m_bUse;
        m_sDirname = info.m_sDirname;
        m_sFilename = info.m_sFilename;
        m_sName = info.m_sName;
        m_sExtension = info.m_sExtension;
        return (*this);
    }

    // -------------------------------------------------------------
    
    URI::Attr::Attr(int type)
            : Object()
    {
        m_type = type;
        m_integer = 0;
    }

    URI::Attr::Attr(int type, int value)
            : Object()
    {
        m_type = type;
        m_integer = value;
    }

    URI::Attr::Attr(int type, const char* value)
            : Object()
    {
        m_type = type;
        m_integer = 0;
        m_value = value;
    }

    URI::Attr::Attr(int type, const char* name, const char* value)
            : Object()
    {
        m_type = type;
        m_integer = 0;
        m_name = name;
        m_value = value;
    }

    URI::Attr::~Attr(){
    }
    
    // -------------------------------------------------------------

    URI::URI(){
    }

    URI::URI(const char* uri) : Object::Object()
    {
        parse(uri);
    }

    URI::URI(const String& uri) : Object::Object()
    {
        parse(uri);
    }

    URI::URI(const URI& uri) : Object::Object()
    {
        set(uri);
    }

    URI::URI(sp<URI>& uri) : Object::Object()
    {
        set(uri);
    }

    URI::~URI(){
    }

    String URI::toString() const {
      String to;
      if(!m_sScheme.empty()) to.appendFmt("%s://", m_sScheme.toChars());
      if(!m_sUser.empty()) to.append(m_sUser);
      if(!m_sPass.empty()) to.appendFmt(":****"/*,m_sPass.toChars()*/);
      if(!m_sUser.empty() || !m_sPass.empty()) to.append("@");
      if(!m_sHost.empty()) to.append(m_sHost);
      if( m_iPort) to.appendFmt(":%d", m_iPort);
      if(!m_sPath.empty()) to.append(m_sPath);
      if(!m_sQuery.empty()) to.appendFmt("?%s",m_sQuery.toChars());
      if(!m_sFragment.empty()) to.appendFmt("#%s",m_sFragment.toChars());
      return to;
    }

    void URI::clear(){
        m_sUri = "";
        m_sScheme = "";
        m_sHost = "";
        m_iPort = 0;
        m_sUser = "";
        m_sPass = "";
        m_sPath = "";
        m_sQuery = "";
        m_sFragment = "";
    }

    void URI::set(const URI& uri){
        m_sUri = uri.getUri();
        m_sScheme = uri.getScheme();
        m_sHost = uri.getHost();
        m_iPort = uri.getRealPort();
        m_sUser = uri.getUser();
        m_sPass = uri.getPass();
        m_sPath = uri.getPath();
        m_sQuery = uri.getQuery();
        m_sFragment = uri.getFragment();
    }

    void URI::set(sp<URI>& uri){
        m_sUri = uri->getUri();
        m_sScheme = uri->getScheme();
        m_sHost = uri->getHost();
        m_iPort = uri->getRealPort();
        m_sUser = uri->getUser();
        m_sPass = uri->getPass();
        m_sPath = uri->getPath();
        m_sQuery = uri->getQuery();
        m_sFragment = uri->getFragment();
    }

    sp<Retval> URI::parse(const char* uri){
        String s = uri;
//        return parse(s);

        sp<Retval> retval;
        return DFW_RET_C(retval, parse(s));
    }

    sp<Retval> URI::parse(const String& uri){
        clear();
        m_sUri = uri;
        m_sUri.trim();

        if( m_sUri.empty() )
            return DFW_RETVAL_NEW(DFW_E_INVAL,0);

        Regexp regexp("(^[a-zA-Z]{1}[a-zA-Z0-9]*)://([\\s\\S]*$)");
        sp<Retval> retval = regexp.regexp(m_sUri);
        if( !retval.has() ){
            m_sScheme = regexp.getMatchString(1);
            return ___parseHostAndPath(regexp.getMatchString(2));
        }else{
            m_sScheme = "file";
            return ___parseHostAndPath(m_sUri);
        }
    }

    sp<Retval> URI::___parseHostAndPath(const String& hp){
        return ___parseHostAndPath(hp.toChars());
    }

    sp<Retval> URI::___parseHostAndPath(const char* hp){
        sp<Retval> retval;
        Regexp uphp_exp("^([a-zA-Z0-9-_]*)@([^\\~\\`\\!\\@\\#\\$\\%\\\\\\^\\&\\*\\(\\)\\_\\+\\-\\=\\{\\}\\[\\]\\;\"\'\\<\\>\\,\\?\\/]*)([/]?)([\\s\\S]*)");
        // user:pass@host.name/folder/file.txt
        // user:pass@host.name/
        // --------- --------- ---------------
        // (1)       (2)     (3)(4)
        // user:pass@a-folder/b-folder/c.txt
        // --------- -------- ----------------
        // (1)       (2)    (3)(4)
        retval = uphp_exp.regexp(hp);
        if( !retval.has() ){
            ___parse_UP_HP(uphp_exp, hp);
            return NULL;
        }

        // host.name/folder/file.txt
        // host.name/
        // --------- ---------------
        // (1)     (2)(3)
        Regexp hp_exp("^([^\\~\\`\\!\\@\\#\\$\\%\\\\\\^\\&\\*\\(\\)\\_\\+\\-\\=\\{\\}\\[\\]\\;\"\'\\<\\>\\,\\?\\/]*)([/]?)([\\s\\S]*)"); // .:
        retval = hp_exp.regexp(hp);
        if( !retval.has() ){
            ___parse_HP(hp_exp, hp);
            return NULL;
        }

        return retval;
    }

    void URI::___parse_HP_FileScheme(
              Regexp& regexp, int last
            , const char* str
            , String& host, String& slash, String& path
         )
    {
        const char *p = regexp.getMatch(last);
        const dfw_ulong_t len = regexp.getMatchLength(last);

        if(m_sScheme.equals("file")){
            if(len>0){
                if(p){
                    path.set(p, len);
                }else{
                    /* this is pcre2 bug? p is not null !!! T.T */
                    dfw_ulong_t off = regexp.getOffset(3);
                    if(!slash.empty())
                        off++;
                    path.set(str+off);
                }
            }
            path = String::format(
                       "%s%s%s",
                       (host.empty())?"":host.toChars(),
                       (slash.empty())?"":slash.toChars(),
                       (path.empty())?"":path.toChars()
            );
            host = "";
        }else{
            if(len>0)
                path.set(--p, len+1);
            else
                path.set("", 0);
        }
    }

    void URI::___parse_UP_HP(Regexp& regexp, const char *str){
        sp<Retval> retval;
        String slash;
        String path;
        String account = regexp.getMatchString(1);
        String host = regexp.getMatchString(2);
        if(regexp.getMatchLength(3)){
           slash.set("/", 1);
        }

        ___parse_HP_FileScheme(regexp, 4, str, host, slash, path);
        ___parse_account( account );
        ___parse_host( host.toChars() );
        ___parse_path( path.toChars() );
    }

    void URI::___parse_HP(Regexp& regexp, const char *str){
        String path;
        String slash;
        String host = regexp.getMatchString(1);
        if(regexp.getMatchLength(2)){
            slash.set("/", 1);
        }

        ___parse_HP_FileScheme(regexp, 3, str, host, slash, path);
        ___parse_host( host.toChars() );
        ___parse_path( path.toChars() );
    }

    void URI::___parse_account(const String& str){
        if( str.empty() ) return;
        const char* p = strstr(str.toChars(), ":");
        if(!p){
            m_sUser = str;
        }else{
            dfw_ulong_t len = p - str.toChars();
            m_sUser.set(str.toChars(), len);
            m_sPass.set(str.toChars()+len+1);
        }
    }

    void URI::___parse_host(const char *str){
        if(str && '/'==str[0]) return;
        dfw_ulong_t index = String::lastIndexOf(str, ":");
        if((dfw_ulong_t)-1==index){
            m_sHost = str;
        }else{
            m_sHost.set(str, index);
            String port; port.set(str+index+1);
            m_iPort = Integer::parseInt(port.toChars());
        }
    }

    void URI::___parse_path(const char *str){
        dfw_ulong_t index = String::indexOf(str, "?");
        if((dfw_ulong_t)-1==index){
            m_sPath.set(str);
        }else{
            m_sPath.set(str, index);
            const char *p = str+index+1;
            dfw_ulong_t index2 = String::indexOf(p, "#");
            if((dfw_ulong_t)-1==index2){
                m_sQuery.set(p);
            }else{
                m_sQuery.set(p, index2);
                m_sFragment.set(p, index2 +index2+1);
            }
        }
    }

    sp<Retval> URI::setAttribute(int type, int value){
        sp<Retval> retval;
        sp<Attr> attr = new Attr(type, value);
        if( DFW_RET(retval, m_aAttrs.insert(attr)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> URI::setAttribute(int type, const char* value){
        sp<Retval> retval;
        sp<Attr> attr = new Attr(type, value);
        if( DFW_RET( retval, m_aAttrs.insert(attr)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> URI::setAttribute(int type
                              , const char* name, const char* value){
        sp<Retval> retval;
        sp<Attr> attr = new Attr(type, name, value);
        if( DFW_RET( retval, m_aAttrs.insert(attr)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    int URI::getDefaultPort(const char* scheme, int defaultPort){
        String s = scheme;
        return getDefaultPort(s, defaultPort);
    }

    int URI::getDefaultPort(const String& scheme, int defaultPort){
        if(scheme.empty()) 
            return defaultPort;

        if(scheme.equals("http")){
            return 80;
        }else if(scheme.equals("file")){
            return 0;
        }else if(scheme.equals("https")){
            return 443;
        }else if(scheme.equals("ftp")){
            return 21;
        }else if(scheme.equals("ssh")){
            return 22;
        }else if(scheme.equals("sftp")){
            return 22;
        }else if(scheme.equals("telnet")){
            return 23;
        }else if(scheme.equals("smtp")){
            return 25;
        }else if(scheme.equals("dns")){
            return 53;
        }else if(scheme.equals("dhcp")){
            return 68;
        }else if(scheme.equals("finger")){
            return 79;
        }else if(scheme.equals("pop3")){
            return 110;
        }else if(scheme.equals("nntp")){
            return 119;
        }else if(scheme.equals("imap")){
            return 143;
        }else if(scheme.equals("snmp")){
            return 161;
        }else if(scheme.equals("irc")){
            return 194;
        }else if(scheme.equals("imap3")){
            return 220;
        }else if(scheme.equals("ldap")){
            return 389;
        }else if(scheme.equals("ssl")){
            return 443;
        }else if(scheme.equals("smb")){
            return 445;
        }else if(scheme.equals("mssql")){
            return 1433;
        }else if(scheme.equals("oracle")){
            return 1521;
        }else if(scheme.equals("nfs")){
            return 2049;
        }else if(scheme.equals("mysql")){
            return 3306;
        }

        return defaultPort;
    }

    URI& URI::operator=(const char* uri){
        parse(uri);
        return *this;
    }

    URI& URI::operator=(const String& uri){
        parse(uri);
        return *this;
    }

    URI& URI::operator=(const URI& uri){
        set(uri);
        return *this;
    }

    URI& URI::operator=(sp<URI>& uri){
        set(uri);
        return *this;
    }

    bool URI::operator == (const URI &from){
        return (!((*this) != from));
    }

    bool URI::operator != (const URI &from){
        if( (m_sScheme == from.m_sScheme)
                && (m_sHost == from.m_sHost)
                && (m_iPort == from.m_iPort)
                && (m_sPath == from.m_sPath)
                && (m_sQuery == from.m_sQuery)
                && (m_sFragment == from.m_sFragment)
                && (m_sUser == from.m_sUser)
                && (m_sPass == from.m_sPass) ){
            return false;
        }
        return true;
    }

    bool URI::operator >  (const URI &from){
        return (!((*this) <= from));
    }

    bool URI::operator <  (const URI &from){
        return (!((*this) >= from));
    }

    bool URI::operator >= (const URI &from){
        if( m_sScheme < from.m_sScheme ) return false;
        if( m_sHost < from.m_sHost ) return false;
        if( m_iPort < from.m_iPort ) return false;
        if( m_sPath < from.m_sPath ) return false;
        if( m_sQuery < from.m_sQuery ) return false;
        if( m_sFragment < from.m_sFragment ) return false;
        if( m_sUser < from.m_sUser ) return false;
        if( m_sPass < from.m_sPass ) return false;
        return true;
    }

    bool URI::operator <= (const URI &from){
        if( m_sScheme > from.m_sScheme ) return false;
        if( m_sHost > from.m_sHost ) return false;
        if( m_iPort > from.m_iPort ) return false;
        if( m_sPath > from.m_sPath ) return false;
        if( m_sQuery > from.m_sQuery ) return false;
        if( m_sFragment > from.m_sFragment ) return false;
        if( m_sUser > from.m_sUser ) return false;
        if( m_sPass > from.m_sPass ) return false;
        return true;
    }

};

