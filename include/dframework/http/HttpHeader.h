#ifndef DFRAMEWORK_HTTP_HTTPHEADER_H
#define DFRAMEWORK_HTTP_HTTPHEADER_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/util/NamedValue.h>

#ifdef __cplusplus
namespace dframework {

    class HttpHeader : public Object
    {
    public:
        ArraySorted<NamedValue> m_aList;

        HttpHeader();
        virtual ~HttpHeader();

        virtual void clear();

        virtual sp<Retval> appendHeader(const char* name, const char* value);
        virtual inline sp<Retval> appendHeader(const String& name
                                     , const char* value){
            return appendHeader(name.toChars(), value);
        }
        virtual inline sp<Retval> appendHeader(const char* name
                                     , const String& value){
            return appendHeader(name, value.toChars());
        }
        virtual inline sp<Retval> appendHeader(const String& name
                                     , const String& value){
            return appendHeader(name.toChars(), value.toChars());
        }

        virtual inline ArraySorted<NamedValue> *getHeaders() { return &m_aList; }
        inline int getHeaderSize() const { return m_aList.size(); }

        virtual sp<NamedValue> getHeader(const char* name) const;
        virtual inline sp<NamedValue> getHeader(const String& name) const{
            return getHeader(name.toChars());
        }
        virtual inline sp<NamedValue> getHeader(int num) const {
            return m_aList.get(num); 
        }

    };

};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPHEADER_H */

