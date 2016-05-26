#ifndef DFRAMEWORK_HTTP_HTTPQUERY_H
#define DFRAMEWORK_HTTP_HTTPQUERY_H

#include <dframework/base/Retval.h>
#include <dframework/base/Object.h>
#include <dframework/lang/String.h>
#include <dframework/net/Hostname.h>
#include <dframework/net/URI.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/http/HttpFormData.h>
#include <dframework/http/HttpRound.h>
#include <dframework/http/HttpListener.h>
#include <dframework/http/HttpHeader.h>


#ifdef __cplusplus
namespace dframework {

  class HttpConnection;
  class HttpMakeClientBuffer;
  class HttpAuth;
  class HttpBasic;
  class HttpDigest;

  class HttpQuery : public Object
  {
  public:
    DFW_DEFINITION_SAFE_COUNT(HttpQuery);

    friend class HttpConnection;
    friend class HttpMakeClientBuffer;
    friend class HttpAuth;
    friend class HttpBasic;
    friend class HttpDigest;

    static const int RESET_DISCONNECT = 1;
    static const int RESET_BASIC = 2;

    class OnHttpListener : public Object
    {
    public:
      inline virtual sp<Retval> onConnect(sp<HttpConnection>&) {
          return NULL; 
      }
      inline virtual sp<Retval> onError(sp<HttpConnection>&, sp<Retval>&) {
          return NULL; 
      }
      inline virtual sp<Retval> onRequest(sp<HttpConnection>&){
          return NULL; 
      }
      inline virtual sp<Retval> onResponse(sp<HttpConnection>&
                                , const char *, dfw_size_t){
          return NULL; 
      }
      inline virtual sp<Retval> onStatus(sp<HttpConnection>&, int){
          return NULL; 
      }
      inline virtual sp<Retval> onAuth(sp<HttpConnection>&
                            , String& /*user*/, String& /*pass*/){
          return NULL; 
      }
      inline virtual sp<Retval> onComplete(sp<HttpConnection>&){
          return NULL; 
      }
    };

  private:
    int       m_iBlockSize;
    int       m_iMaxRetryRequestCount;
    bool      m_bOnlyHead;

    sp<HttpConnection>   m_conn;
    Array<HttpFormData>  m_aFormDataList;
    Array<HttpRound>     m_aRoundList;
    sp<HttpRound>        m_pFirstRound;
    sp<HttpRound>        m_pLastRound;

    sp<OnHttpListener>   m_pHttpListener;

    String    m_sMethod;
    String    m_sVirtualHost;
    int       m_iVirtualPort;
    String    m_sUserAgent;

    // -----------------------------------
    unsigned* m_readerOutSize;
    char*     m_readerBuffer;
    unsigned  m_readerBufferSize;
    String    m_readerSavedBuffer;

  public:
    sp<HttpHeader>  m_requestHeader;

  public:
    HttpQuery();
    virtual ~HttpQuery();

    virtual void clear();
    inline sp<HttpConnection>& getConnection() {
        return m_conn;
    }
    virtual void close();

    inline void setOnlyHead(bool bOnly){ m_bOnlyHead = bOnly; }
    inline bool isOnlyHead(){ return m_bOnlyHead; }

    // --------------------------------------------------------------

    virtual sp<Retval> query(const URI& uri, const char* method=NULL);
    inline virtual sp<Retval> query(const URI& uri, const String& method){
        return query(uri, method.toChars());
    }
    virtual sp<Retval> query(const char* uri, const char* method=NULL);
    inline virtual sp<Retval> query(const String& uri,const String& method){
        return query(uri.toChars(), method.toChars());
    }
    inline virtual sp<Retval> query(const String& uri
                                    , const char* method=NULL){
        return query(uri.toChars(), method);
    }
    inline virtual sp<Retval> query(const char* uri, const String& method){
        return query(uri, method.toChars());
    }

    // --------------------------------------------------------------

    virtual sp<Retval> request(const URI& uri, const char* method=NULL);
    inline virtual sp<Retval> request(const URI& uri, const String& method){
        return request(uri, method.toChars());
    }
    virtual sp<Retval> request(const char* uri, const char* method=NULL);
    inline virtual sp<Retval> request(const String& uri,const String& method){
        return request(uri.toChars(), method.toChars());
    }
    inline virtual sp<Retval> request(const String& uri
                                    , const char* method=NULL){
        return request(uri.toChars(), method);
    }
    inline virtual sp<Retval> request(const char* uri, const String& method){
        return request(uri, method.toChars());
    }

    // --------------------------------------------------------------

    virtual sp<Retval> read(unsigned* outsize, char* buf, unsigned size);

    // --------------------------------------------------------------

    virtual sp<Retval> response();

    // --------------------------------------------------------------

    sp<Retval> createRound(const URI& uri);

    sp<Retval> makeRequestBuffer(sp<HttpRound>& round);

    sp<Retval> onAuth(sp<HttpConnection>& conn, String& user, String& pass);

    // --------------------------------------------------------------
    // about HttpFormData
    sp<Retval> addFormData(const char* name, const char* value);
    inline sp<Retval> addFormData(const String& name, const char* value){
        return addFormData(name.toChars(), value);
    }
    inline sp<Retval> addFormData(const char* name, const String& value){
        return addFormData(name, value.toChars());
    }
    inline sp<Retval> addFormData(const String& name, const String& value){
        return addFormData(name.toChars(), value.toChars());
    }
    inline sp<HttpFormData> removeFormData(int pos) { 
        return m_aFormDataList.remove(pos); 
    }
    inline int sizeFormData() { return m_aFormDataList.size(); }
    inline sp<HttpFormData> getFormData(int pos) { 
        return m_aFormDataList.get(pos); 
    }
    //
    // --------------------------------------------------------------

    // --------------------------------------------------------------
    // about request header
    inline sp<Retval> addHeader(const char* name, const char* val){
        return m_requestHeader->appendHeader(name, val);
    }
    inline sp<Retval> addHeader(const String& name, const char* val){
        return m_requestHeader->appendHeader(name.toChars(), val);
    }
    inline sp<Retval> addHeader(const char* name, const String& val){
        return m_requestHeader->appendHeader(name, val.toChars());
    }
    inline sp<Retval> addHeader(const String& name, const String& val){
        return m_requestHeader->appendHeader(name, val);
    }
    // 
    // --------------------------------------------------------------

    // --------------------------------------------------------------
    // about events
    inline virtual sp<Retval> onConnect(sp<HttpConnection>& c) {
        return (m_pHttpListener.has() 
                   ? m_pHttpListener->onConnect(c) : NULL);
    }

    inline virtual sp<Retval> onError(sp<HttpConnection>& c
                                    , sp<Retval>& retval)
    {
        return (m_pHttpListener.has() 
                   ? m_pHttpListener->onError(c, retval) : NULL);
    }

    inline virtual sp<Retval> onRequest(sp<HttpConnection>& c){
        return (m_pHttpListener.has() 
                   ? m_pHttpListener->onRequest(c) : NULL);
    }

    inline virtual sp<Retval> onStatus(sp<HttpConnection>& c, int status){
        return (m_pHttpListener.has()
                   ? m_pHttpListener->onStatus(c, status) : NULL);
    }


    virtual sp<Retval> onResponse(sp<HttpConnection>& c
                                   , const char *b, dfw_size_t s);

    inline virtual sp<Retval> onComplete(sp<HttpConnection>& c){
        return (m_pHttpListener.has() 
                   ? m_pHttpListener->onComplete(c) : NULL);
    }
    //
    // --------------------------------------------------------------

    inline sp<HttpRound> getLastRound() { return m_pLastRound; }

    inline void setMethod(const char* method) { m_sMethod = method; }
    inline void setMethod(const String& method) { m_sMethod = method; }
    inline const char* getMethod()const{return m_sMethod.toChars();}

    inline void setOnHttpListener(sp<OnHttpListener>& l){
      m_pHttpListener = l;
    }

    inline sp<OnHttpListener>& getOnHttpListener() {
      return m_pHttpListener;
    }

    // --------------------------------------------------------------
    //
    inline void setVirtualHost(const char *host, int port){
      m_sVirtualHost = host;
      m_iVirtualPort = port;
    }

    inline const char* getVirtualHost() const {
      return m_sVirtualHost.toChars();
    }

    inline int getVirtualPort() const { return m_iVirtualPort; }
    //
    // --------------------------------------------------------------

    void setUserAgent(const char *agent);
    void appendUserAgent(const char *agent);

    inline const char* getUserAgent() const {
      return m_sUserAgent.toChars();
    }

    inline void setMaxRetryRequestCount(int count) {
      m_iMaxRetryRequestCount = count;
    }

    inline int getMaxRetryRequestCount() const{
      return m_iMaxRetryRequestCount;
    }

    inline void setBlockSize(int blockSize){ m_iBlockSize = blockSize; }

    inline int getBlockSize() const{ return m_iBlockSize; }


    inline const char* getFirstPath() const {
      return (m_pFirstRound.has() 
              ? m_pFirstRound->m_oUri.getPath().toChars() : NULL); 
    }

    inline const char* getLastPath() const { 
      return (m_pLastRound.has() 
              ? m_pLastRound->m_oUri.getPath().toChars() : NULL); 
    }

    inline const char* getPath() const{ return getLastPath(); }

    inline int getStatus() const {
        return (m_pLastRound.has() ? m_pLastRound->m_iStatus : -1);
    }

    inline const char* getContentType() const {
        return (m_pLastRound.has() 
               ? m_pLastRound->m_sContentTypeLine.toChars() 
               : NULL);
    }

  };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPQUERY_H */

