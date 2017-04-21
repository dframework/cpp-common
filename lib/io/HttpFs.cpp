#include <dframework/io/HttpFs.h>
#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpPropfind.h>
#include <dframework/http/types.h>
#include <dframework/util/Time.h>
#include <dframework/log/Logger.h>
#include <dframework/http/HttpContentType.h>
#include <dframework/base/System.h>

namespace dframework {

    class HttpFsPropfindListener : public HttpPropfind::OnPropfindListener
    {
    private:
        sp<DirBox> m_db;

    public:
        HttpFsPropfindListener(sp<DirBox>& db)
                : HttpPropfind::OnPropfindListener()
        {
            m_db = db;
        }
        virtual ~HttpFsPropfindListener(){
        }
        virtual sp<Retval> onPropfind(sp<HttpConnection>& conn
                                    , sp<HttpPropfind::Prop>& prop)
        {
          DFW_UNUSED(conn);
          sp<Retval> retval;

          int mode;
          if(prop->m_bDir){
              mode = S_IFDIR | S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH;
          }else{
              mode = S_IFREG | S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH;
          }

          sp<Stat> st = new Stat();
          st->setName(prop->m_sName);
          st->m_stat.st_ctime = (time_t)prop->m_CreateDate;
          st->m_stat.st_mtime = (time_t)prop->m_LastModified;
          st->m_stat.st_atime = (time_t)(Time::currentTimeMillis() / 1000);
          st->m_stat.st_size = prop->m_uContentLength;
          st->m_stat.st_mode = mode;
          st->m_stat.st_uid = 0;
          st->m_stat.st_gid = 0;
          return DFW_RET_C(retval, m_db->insert(st));
        }

        friend class HttpFs;
    };

    // --------------------------------------------------------------

    HttpFs::Listener::Listener(){
    }

    HttpFs::Listener::~Listener(){
    }

    sp<Retval> HttpFs::Listener::onStatus(sp<HttpConnection>& /*conn*/
                                        , int /*status*/)
    {
        return NULL;
    }

    sp<Retval> HttpFs::Listener::onResponse(sp<HttpConnection>& /*conn*/
                                        , const char* buf, dfw_size_t size)
    {
        m_sBuffer.append(buf, size);
        return NULL;
    }

    // --------------------------------------------------------------

    DFW_DECLARATION_SAFE_COUNT(HttpFs);

    HttpFs::HttpFs(){
        DFW_SAFE_ADD(HttpFs, l);
        m_size = 0;
        m_offset = 0;
        m_step = 0;
        m_bUseHead = true;
        m_uTimeout = 0;
    }

    HttpFs::~HttpFs(){
        close("");
        DFW_SAFE_REMOVE(HttpFs, l);
    }

    // --------------------------------------------------------------

    sp<Retval> HttpFs::ready(sp<URI>& uri){
        m_sUri = uri->toString();
        m_oUri = uri;
        int size = uri->sizeAttribute();
        for(int k=0; k<size; k++){
            sp<URI::Attr> attr = uri->getAttribute(k);
            switch(attr->m_type){
            case BaseFs::ATTR_NOHEAD :
                m_bUseHead = (attr->m_integer ? false : true);
                break;
            case BaseFs::ATTR_HEADER :
                if( !attr->m_name.empty() ){
                    if(attr->m_name.equals("User-Agent") ){
                        m_sUserAgent = attr->m_value;
                    }
                }
                break;
            }
        }
        return NULL;
    }

    void HttpFs::setTimeout(unsigned long value){
        m_uTimeout = value;
    }

    DFW_PRIVATE
    sp<Retval> HttpFs::getattr_l2(sp<HttpQuery>& http
                                , URI& uri, bool* isclosed){
        sp<Retval> retval;

        *isclosed = true;

        http->setOnlyHead(true);
        if( DFW_RET(retval, http->query(uri, "GET")) ){
            return DFW_RETVAL_D(retval);
        }
        http->setOnlyHead(false);
        return NULL;
    }

    DFW_PRIVATE
    sp<Retval> HttpFs::getattr_l(sp<HttpQuery>& http
                               , const char* path, struct stat* st, bool* isclosed)
    {
        sp<Retval> retval;
        String sPath;
        int status;

        if( !path || (path && strlen(path)==0) ) path = "/";

        if( m_oUri->getPath().empty() )
            sPath = String::format("%s", path);
        else
            sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);

        URI oUri = m_oUri;
        oUri.setPath(sPath.toChars());

        if( m_bUseHead ){
            if( DFW_RET(retval, http->query(oUri, "HEAD")) ){
                sp<Retval> retval2;
                if( DFW_RET(retval2, getattr_l2(http, oUri, isclosed)) )
                    return DFW_RETVAL_D(retval);
            }
        }else if( DFW_RET(retval, getattr_l2(http, oUri, isclosed)) ){
            return DFW_RETVAL_D(retval);
        }

        status = http->getStatus();
        if( (path[strlen(path)-1]=='/') && (status == 200) )
            status = 301;

        switch( status ){
        case 200 :
            {
                sp<HttpRound> round = http->getLastRound();
                if( !round.has() ){
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                               , "No getattr, Has not last-round"
                                 ", path=%s, uri=%s, http-status=%d"
                               , path, oUri.toString().toChars(), status);
                }
                sp<NamedValue> modify
                        = round->m_responseHeader->getHeader("Last-Modified");
                sp<NamedValue> clength
                        = round->m_responseHeader->getHeader("Content-Length");
                if( !modify.has() ){
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                               , "No getattr, Has not last-modified"
                                 ", path=%s, uri=%s, http-status=%d"
                               , path, oUri.toString().toChars(), status);
                }
                if( !clength.has() ){
                    return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                               , "No getattr, Has not content-length"
                                 ", path=%s, uri=%s, http-status=%d"
                               , path, oUri.toString().toChars(), status);
                }

                dfw_time_t time = 0;
                ::memset(st, 0, sizeof(struct stat));
                Time::parse(&time, modify->getChars());
                st->st_mtime = (time_t)time;
                st->st_size = clength->getUnsignedLong();
                st->st_mode = S_IFREG | S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH;
                st->st_atime = (time_t)(Time::currentTimeMillis() / 1000);
                st->st_ctime = st->st_mtime;
                st->st_uid = 0;
                st->st_gid = 0;
            }
            return NULL;

        case 404 :
            return DFW_RETVAL_NEW_MSG(DFW_E_NOENT, ENOENT
                       , "Not found file or dir. uri=%s"
                       , oUri.toString().toChars());

        case 301 :
        default :
            ::memset(st, 0, sizeof(struct stat));
            st->st_size = 0;
            st->st_mode = S_IFDIR|S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
            st->st_atime = (time_t)(Time::currentTimeMillis() / 1000);
            st->st_ctime = st->st_atime;
            st->st_mtime = st->st_atime;
            st->st_uid = 0;
            st->st_gid = 0;
            return NULL;
        }

        return DFW_RETVAL_NEW(DFW_ERROR, EIO);
    }

    sp<Retval> HttpFs::getattr(const char* path, struct stat* st){
        sp<Retval> retval;
        bool isclosed = false;
        m_sPath = path;

        sp<Listener> listener = new Listener();
        sp<HttpQuery::OnHttpListener> listener_ = listener;
        sp<HttpQuery> query = new HttpQuery();
        query->setOnHttpListener(listener_);
        if(!m_sUserAgent.empty()){
            query->setUserAgent(m_sUserAgent.toChars());
        }

        if( DFW_RET(retval, getattr_l(query, /*sUrl,*/ path, st, &isclosed)) ){
            query->close();
            return DFW_RETVAL_D(retval);
        }
        query->close();
        return NULL;
    }

    sp<Retval> HttpFs::readdir(const char* path, sp<DirBox>& db){
        sp<Retval> retval;

        String sPath;
        if( !path || (path && strlen(path)==0) ) path = "/";

        if( path[ strlen(path)-1 ] == '/' ){
            if( m_oUri->getPath().empty() )
                sPath = String::format("%s", path);
            else
                sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);
        }else{
            if( m_oUri->getPath().empty() )
                sPath = String::format("%s/", path);
            else
                sPath = String::format("%s%s/", m_oUri->getPath().toChars(), path);
        }

        URI oUri = m_oUri;
        oUri.setPath(sPath.toChars());

        sp<HttpQuery::OnHttpListener> http_l = new HttpQuery::OnHttpListener();
        sp<HttpFsPropfindListener> l = new HttpFsPropfindListener(db);
        sp<HttpPropfind::OnPropfindListener> _l = l;
        sp<HttpPropfind> propfind = new HttpPropfind();
        propfind->setOnHttpListener(http_l);
        propfind->setOnPropfindListener(_l);

        if( DFW_RET(retval, propfind->query(oUri)) ){
            propfind->close();
            return DFW_RETVAL_D(retval);
        }
        
        switch( propfind->getStatus()){
        case 200 :
        case 207 :
            break;
        default :
            propfind->close();
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                       , "No propfind, path=%s, status=%d"
                       , path, propfind->getStatus());
        }
        propfind->close();
        return NULL;
    }

    DFW_PRIVATE
    sp<Retval> HttpFs::open_l(const char* path){
        sp<Retval> retval;

        if( !path || (path && strlen(path)==0) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO, "path is none.");
        }

        bool isclosed = false;
        if( m_http.has() )
            m_http->close();
        m_http = new HttpQuery();
        if(!m_sUserAgent.empty()){
            m_http->setUserAgent(m_sUserAgent.toChars());
        }

        struct stat st;
        if( DFW_RET(retval, getattr_l(m_http, path, &st, &isclosed)) ){
            m_http->close();
            return DFW_RETVAL_D(retval);
        }

        int status = m_http->getStatus();
        if( status != 200 ){ // open is file.
            m_http->close();
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                       , "status is not 200. status=%d, path=%s"
                       , status, path);
        }

        m_size = st.st_size;

        if( isclosed ) {
            m_http->close();
            m_http = NULL;
            m_http = new HttpQuery();
            if(!m_sUserAgent.empty()){
                m_http->setUserAgent(m_sUserAgent.toChars());
            }
        }

        m_offset = 0;
        m_step = 1;
        m_http_listener = new Listener();
        sp<HttpQuery::OnHttpListener> listener_ = m_http_listener;
        m_http->setOnHttpListener(listener_);

        return NULL;
    }

    sp<Retval> HttpFs::open(const char* path, int flag, int mode)
    {
        DFW_UNUSED(flag);
        DFW_UNUSED(mode);
        AutoLock _l(this);
        sp<Retval> retval;
        m_sPath = path;
        return DFW_RET_C(retval, open_l(path));
    }

    sp<Retval> HttpFs::close(const char* path){
        DFW_UNUSED(path);
        AutoLock _l(this);
        m_step = 0;
        if( m_http.has() ){
            m_http->close();
            m_http = NULL;
        }
        return NULL;
    }

#define QUERY 0
    sp<Retval> HttpFs::read(const char* path
                          , unsigned *outsize
                          , char* buf, uint32_t size
                          , uint64_t offset)
    {
        AutoLock _l(this);
        sp<Retval> retval;
        uint32_t nsize;

        *outsize = 0;

#if QUERY == 1
        if( !m_http.has() ){
#else
        if( !m_http.has() || m_offset!=offset ){
//if( !m_http.has() )
//printf("read: offset=%ld !m_http.has()\n", offset);
//else if( m_offset != offset )
//printf("read: offset=%ld m_offset(%ld) != offset\n", offset, m_offset);
#endif
            if( DFW_RET(retval, open_l(path)) )
                return DFW_RETVAL_D(retval);
            if( !m_http.has() ){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "Has not http object. path=%s, offset=%lu"
                           , path, offset);
            }
        }

        if( offset >= m_size ){
            return NULL;
        }

        if( offset+size > m_size ){
            nsize = size - ((offset+size) - m_size);
        }else{
            nsize = size;
        }

        m_http_listener->m_sBuffer = NULL;
        if( DFW_RET(retval, request_l(path, outsize, buf, nsize, offset)) ){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpFs::request_l(const char* path
                               , unsigned* outsize
                               , char* buf, uint32_t size
                               , uint64_t offset)
    {
        sp<Retval> retval;

        if( !path || (path && strlen(path)==0) )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO, "path is none.");

#if QUERY != 1
        if( 2 == m_step ){
            if( DFW_RET(retval, m_http->read(outsize, buf, size)) ){
                m_offset += *outsize;
                return DFW_RETVAL_D(retval);
            }
//printf("read: offset=%ld step=2\n", offset);
            m_offset += *outsize;
            return NULL;
        }
        m_offset = offset;
#endif

        String sPath;
        if( m_oUri->getPath().empty() )
            sPath = String::format("%s", path);
        else
            sPath = String::format("%s%s", m_oUri->getPath().toChars(), path);

        URI oUri = m_oUri;
        oUri.setPath(sPath.toChars());

#if QUERY == 1
        String sRange = String::format("bytes=%llu-%llu", offset, (offset+size-1));
#else
        String sRange = String::format("bytes=%llu-", offset);
#endif

        m_http->clear();
        m_http->addHeader("Range", sRange);
#if QUERY == 1
        if( DFW_RET(retval, m_http->query(oUri)) )
#else
        if( DFW_RET(retval, m_http->request(oUri)) )
#endif
        {
            return DFW_RETVAL_D(retval);
        }

        int status = m_http->getStatus();
        switch(status){
        case 200 :
        case 206 :
#if QUERY == 1
            ::memcpy(buf
               , m_http_listener->m_sBuffer.toChars()
               , m_http_listener->m_sBuffer.length());
            *outsize = m_http_listener->m_sBuffer.length();
#else
            if( DFW_RET(retval, m_http->read(outsize, buf, size)) ){
                m_step = 2;
                m_offset += *outsize;
                return DFW_RETVAL_D(retval);
            }
//printf("read: offset=%ld step=1\n", offset);
            m_step = 2;
            m_offset += *outsize;
#endif
            return NULL;
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR, EIO
                   , "status=%d, path=%s", status, path);
    }

    sp<Retval> HttpFs::getContentType(String& sContentType){
        sContentType = "video/mp4";
        return NULL;
        /*
        URI::FileInfo fi;
        fi.parse(m_sPath.toChars());
        const char* ext = fi.m_sExtension.toChars();
        String sType = HttpContentType::getContentType(ext);
        bool bUseEncode = false;

        if( sType.indexOf("text/")==0 ){
            bUseEncode = true;
        }else if( sType.indexOf("xml/")==0 ){
            bUseEncode = true;
        }else if( sType.indexOf("/xml")!=((dfw_size_t)-1) ){
            bUseEncode = true;
        }

        if(bUseEncode){
            const char* enc = System::encoding();
            if(enc){
                sContentType = String::format("%s; charset=%s"
                                            , sType.toChars(), enc);
            }else{
                sContentType = sType;
            }
        }else{
            sContentType = sType;
        }

        DFWLOG(DFWLOG_I|DFWLOG_ID(DFWLOG_HTTPD_ID)
              , "Request:: getContentType=%s", sContentType.toChars());

        return NULL;
        // *
        if( !m_http.has() )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Has not http object.");
        sContentType = m_http->getContentType();
        return NULL;
        */
    }

    sp<Retval> HttpFs::setAttribute(int type, int value){
        switch(type){
        case BaseFs::ATTR_NOHEAD :
            m_bUseHead = value;
            break;
        }
        return NULL;
    }

    sp<Retval> HttpFs::setAttribute(int type, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(value);
        return NULL;
    }

    sp<Retval> HttpFs::setAttribute(int type
                         , const char* name, const char* value){
        DFW_UNUSED(type);
        DFW_UNUSED(name);
        DFW_UNUSED(value);
        return NULL;
    }

};

