#include <dframework/http/HttpGet.h>

namespace dframework {

    HttpGet::HttpGet()
    {
    }

    HttpGet::~HttpGet(){}

    sp<Retval> HttpGet::query(const URI& uri, const char* /*method*/){
        sp<Retval> retval;
        setMethod("GET");
        return DFW_RET_C(retval, HttpQuery::query(uri, NULL));
    }

    sp<Retval> HttpGet::query(const char* uri, const char* method){
        sp<Retval> retval;
        URI oUri = uri;
        return DFW_RET_C(retval, query(oUri, method));
    }

};

