#include <dframework/http/HttpHeader.h>

namespace dframework {

    HttpHeader::HttpHeader(){
    }

    HttpHeader::~HttpHeader(){
    }

    void HttpHeader::clear(){
        m_aList.clear();
    }

    sp<Retval> HttpHeader::appendHeader(
            const char *name, const char *value)
    {
        sp<NamedValue> nv = new NamedValue(name, value);
        m_aList.remove(nv);
        return m_aList.insert(nv);
    }

    sp<NamedValue> HttpHeader::getHeader(const char *name) const{
        sp<NamedValue> nv = new NamedValue(name, NULL);
        return m_aList.get(nv);
    }

};

