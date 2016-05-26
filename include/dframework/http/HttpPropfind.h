#ifndef DFRAMEWORK_HTTP_PROPFIND_H
#define DFRAMEWORK_HTTP_PROPFIND_H
#define DFRAMEWORK_HTTP_PROPFIND_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/util/Array.h>
#include <dframework/xml/XmlParser.h>
#include <dframework/http/HttpQuery.h>


enum dfw_dav_lockscope_t {
    DFW_DAV_LOCKSCOPE_EXCLUSIVE = 1,
    DFW_DAV_LOCKSCOPE_SHARED = 3,
};

enum dfw_dav_locktype_t {
    DFW_DAV_LOCKTYPE_WRITE = 1,
};

#ifdef __cplusplus
namespace dframework {

  class HttpPropfind : public HttpQuery 
  {
  public:
    class Prop : public Object
    {
    public:
        struct lockentry {
            int lockscope;
            int locktype;
        };
        typedef struct lockentry lockentry;

    public:
        String     m_sName;
        String     m_sContentType;
        String     m_sETag;

        int        m_iHttpStatus;
        int        m_iExclusiveLock;
        int        m_iSharedLock;
        bool       m_bDir;
        bool       m_bFile;
        dfw_time_t m_CreateDate;
        dfw_time_t m_LastModified;
        dfw_size_t m_uContentLength;

    public:
        Prop();
        virtual ~Prop();
    };

    class OnPropfindListener : public Object {
    public:
      inline virtual sp<Retval> onPropfind(sp<HttpConnection>&, sp<Prop>&){
          return NULL;
      }
    };

  private:
    sp<OnPropfindListener> m_pPropfindListener;
    XmlParser   m_XmlParser;
    String      m_sPath;

    sp<Retval> propfindSupportedlock(
            sp<Prop>& prop
          , Prop::lockentry *ple
          , sp<XmlNode>& node);
    sp<Retval> propfindProp(sp<Prop>& prop, sp<XmlNode>& node);
    sp<Retval> propfindStat(sp<Prop>& prop, sp<XmlNode>& node);
    sp<Retval> propfindResponse(sp<HttpConnection>&, sp<XmlNode>&);
    sp<Retval> propfindComplete(sp<HttpConnection>&, Array<XmlNode>*);


  public:
    HttpPropfind();
    virtual ~HttpPropfind();

    virtual sp<Retval> query(const URI& uri, const char* method=NULL);
    virtual sp<Retval> query(const char* uri, const char* method=NULL);

    virtual sp<Retval> onRequest(sp<HttpConnection>& c);
    virtual sp<Retval> onResponse(sp<HttpConnection>& c
                                , const char*, dfw_size_t);

    inline void setOnPropfindListener(sp<OnPropfindListener>& l){
      m_pPropfindListener = l;
    }
    inline sp<OnPropfindListener>& getOnPropfindListener(){
      return m_pPropfindListener;
    }

    inline virtual sp<Retval> onPropfind(sp<HttpConnection>& c
                                       , sp<Prop>& prop)
    {
        return (m_pPropfindListener.has()
                   ? m_pPropfindListener->onPropfind(c, prop) : NULL);
    }

  };

};
#endif

#endif /* DFRAMEWORK_HTTP_PROPFIND_H */

