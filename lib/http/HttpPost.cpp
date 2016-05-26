#include <dframework/http/HttpPost.h>

namespace dframework {

    HttpPost::HttpPost(){
    }

    HttpPost::~HttpPost(){}

    sp<Retval> HttpPost::query(const URI& uri, const char* /*method*/){
        sp<Retval> retval;
        setMethod("POST");
        return DFW_RET_C(retval, HttpQuery::query(uri, NULL));
    }

    sp<Retval> HttpPost::query(const char* uri, const char* method){
        sp<Retval> retval;
        URI oUri = uri;
        return DFW_RET_C(retval, query(oUri, method));
    }

};

