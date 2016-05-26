#ifndef DFRAMEWORK_HTTP_HTTPLISTENER_H
#define DFRAMEWORK_HTTP_HTTPLISTENER_H

#include <dframework/base/Retval.h>
#include <dframework/base/Object.h>
#include <dframework/lang/String.h>
#include <dframework/http/HttpStatus.h>

#ifdef __cplusplus
namespace dframework {

  class HttpConnection;

  class OnHttpListener : public Object
  {
  public:
    virtual sp<Retval> onConnect(sp<HttpConnection>&)=0;
    virtual sp<Retval> onError(sp<HttpConnection>&, sp<Retval>&)=0;
    virtual sp<Retval> onRequest(sp<HttpConnection>&)=0;
    virtual sp<Retval> onResponse(sp<HttpConnection>&
                                , const char *, dfw_size_t)=0;
    virtual sp<Retval> onStatus(sp<HttpConnection>&, int)=0;
    virtual sp<Retval> onAuth(sp<HttpConnection>&
                            , String& user, String& pass)=0;
    virtual sp<Retval> onComplete(sp<HttpConnection>&)=0;
  };

};
#endif

#endif /* DFRAMEWORK_HTTP_HTTPLISTENER_H */

