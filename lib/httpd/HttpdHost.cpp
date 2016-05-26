#include <dframework/httpd/HttpdConfigure.h>
#include <dframework/httpd/HttpdHost.h>
#include <dframework/net/Net.h>
#include <dframework/log/Logger.h>

namespace dframework {

    HttpdHost::AliasUri::AliasUri(){
    }

    HttpdHost::AliasUri::~AliasUri(){
    }

    // --------------------------------------------------------------

    HttpdHost::HttpdHost(sp<Object>& configure){
        m_configure = configure;
        m_aBaseDocumentRoot = new AliasUri();
    }

    sp<Retval> HttpdHost::ready(int port){
        AutoLock _l(this);
        sp<Retval> retval;
        return DFW_RET_C(retval, ready("", port, port));
    }

    sp<Retval> HttpdHost::ready(const char* pHost, int port){
        AutoLock _l(this);
        sp<Retval> retval;
        return DFW_RET_C(retval, ready(pHost, port, port));
    }

    sp<Retval> HttpdHost::ready(int sport, int eport){
        AutoLock _l(this);
        sp<Retval> retval;
        return DFW_RET_C(retval, ready("", sport, eport));
    }

    sp<Retval> HttpdHost::ready(const char* pHost, int sport, int eport){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<ServerSocket> sock = new ServerSocket();
        if( DFW_RET(retval, sock->ready(sport, eport)) )
            return DFW_RETVAL_D(retval);

        m_serverSocket = sock;
        m_sHost = pHost;
        return NULL;
    }

    HttpdHost::~HttpdHost(){
    }

    int HttpdHost::getPort(){
        AutoLock _l(this);
        if( m_serverSocket.has())
            return m_serverSocket->getPort();
        return 0;
    }

    String& HttpdHost::getHost(){
        AutoLock _l(this);
        return m_sHost;
    }

    /////////////////////////////////////////////////////////////////

    sp<HttpdHost::AliasUri> HttpdHost::getAliasUri(const char* path){
        AutoLock _l(this);
        unsigned pathlen = strlen(path);
        if( 1==pathlen )
            return m_aBaseDocumentRoot;

        for(int k=0; k<m_aAliasDocumentRoot.size(); k++){
            sp<HttpdHost::AliasUri> alias = m_aAliasDocumentRoot.get(k);

            if( alias->m_sAliasName.empty() ) continue;
            if( pathlen < (alias->m_sAliasName.length()+1) ) continue;

            if( ::strstr(path+1, alias->m_sAliasName.toChars())==(path+1) ){
                if( (alias->m_sAliasName.length()+1) == pathlen ){
                    return alias;
                }else if( path[alias->m_sAliasName.length()+1] == '/' ){
                    return alias;
                }
            }
        }

        return m_aBaseDocumentRoot;
    }

    sp<Retval> HttpdHost::addDocumentRoot(const char* document
                                        , const char* palias)
    {
        sp<Retval> retval;
        sp<AliasUri> alias;
        if( palias == NULL || (palias!=NULL && ::strlen(palias)==0) ){
            alias = m_aBaseDocumentRoot;
        }else{
            for(int k=0; k<m_aAliasDocumentRoot.size(); k++){
                sp<HttpdHost::AliasUri> test = m_aAliasDocumentRoot.get(k);
                if( test->m_sAliasName.empty() ) continue;
                if( test->m_sAliasName.equals(palias)){
                    alias = test;
                    break;
                }
            }
            if( !alias.has() ){
                sp<AliasUri> test = new AliasUri();
                test->m_sAliasName = palias;
                if( DFW_RET(retval, m_aAliasDocumentRoot.insert(test)) )
                    return DFW_RETVAL_D(retval);
                alias = test;
            }
        }

        const char* useragent = NULL;
        if( m_sUserAgent.empty() ){
            sp<HttpdConfigure> config = m_configure;
            useragent = config->getUserAgent();
        }else{
            useragent = m_sUserAgent.toChars();
        }

        sp<URI> uri = new URI(document);
        if(useragent && (DFW_RET(retval, uri->setAttribute(BaseFs::ATTR_HEADER
                                     , "User-Agent", useragent))) ){
            return DFW_RETVAL_D(retval);
        }
        if( DFW_RET(retval, alias->m_aUris.insert(uri)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    /////////////////////////////////////////////////////////////////

    sp<Retval> HttpdHost::addModule(sp<HttpdMod>& mod){
        AutoLock _l(this);
        sp<Retval> retval;
        sp<HttpdMod> find = m_aMods.get(mod);
        if(find.has())
            return NULL;
        if( DFW_RET(retval, m_aMods.insert(mod)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    int HttpdHost::getModSize() {
        AutoLock _l(this);
        return m_aMods.size(); 
    }

    sp<HttpdMod> HttpdHost::getMod(int k) {
        AutoLock _l(this);
        return m_aMods.get(k); 
    }

};

