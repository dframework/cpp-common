#ifndef DFRAMEWORK_HTTP_HTTPDAV_H
#define DFRAMEWORK_HTTP_HTTPDAV_H


#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/util/Array.h>
#include <dframework/xml/XmlParser.h>


enum dfw_dav_lockscope_t {
    DFW_DAV_LOCKSCOPE_EXCLUSIVE = 1,
    DFW_DAV_LOCKSCOPE_SHARED = 3,
};

enum dfw_dav_locktype_t {
    DFW_DAV_LOCKTYPE_WRITE = 1,
};


#ifdef __cplusplus
namespace dframework {

    class HttpConnection;
    class HttpPropfind;
    class HttpDav;

    class OnHttpPropfindListener {
    public:
        virtual sp<Retval> onPropfind(HttpDav *, HttpPropfind*)=0;
    };

    class HttpPropfind
    {
    public:
        struct lockentry {
            int lockscope;
            int locktype;
        };
        typedef struct lockentry lockentry;

    public:
        String m_sName;
        String m_sContentType;
        String m_sETag;

        int    m_iHttpStatus;
        int    m_iExclusiveLock;
        int    m_iSharedLock;
        bool   m_bDir;
        bool   m_bFile;
        dfw_time_t m_CreateDate;
        dfw_time_t m_LastModified;
        dfw_size_t m_uContentLength;

    public:
        HttpPropfind();
        virtual ~HttpPropfind();
    };

    class HttpDav
    {
    private:
        XmlParser m_XmlParser;
        OnHttpPropfindListener *m_pHttpPropfindListener;
        String    m_sPath;

        sp<Retval> propfindSupportedlock(
                HttpPropfind *prop
              , HttpPropfind::lockentry *ple
              , sp<XmlNode> node);
        sp<Retval> propfindProp(HttpPropfind *prop, sp<XmlNode> node);
        sp<Retval> propfindStat(HttpPropfind *prop, sp<XmlNode> node);
        sp<Retval> propfindResponse(sp<XmlNode> node);
        sp<Retval> propfindComplete(Array<XmlNode> *list);

    public:
        HttpDav();
        virtual ~HttpDav();

        void clear();

        inline void setOnHttpPropfindListener(OnHttpPropfindListener *l){
            m_pHttpPropfindListener = l;
        }
        inline OnHttpPropfindListener* getOnHttpPropfindListener(){
            return m_pHttpPropfindListener;
        }

        inline void setPath(const char *path) { m_sPath = path; }
        sp<Retval> sendPropfind(
                sp<HttpConnection>& conn, const char *buf, dfw_size_t size);
    };

};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPDAV_H */

