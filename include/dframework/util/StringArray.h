#ifndef DFRAMEWORK_UTIL_STRINGARRAY_H
#define DFRAMEWORK_UTIL_STRINGARRAY_H

#include <dframework/util/Array.h>
#include <dframework/lang/String.h>
#include <dframework/util/StringArray.h>

#ifdef __cplusplus
namespace dframework {

    class StringArray : public Object
    {
    private:
        Array<String> m_aList;

    public:
        StringArray();
        virtual ~StringArray();

        inline int size() { return m_aList.size(); }
        inline sp<String> getString(int position) {
            return m_aList.get(position);
        }

        sp<Retval> split(const char* buffer, const char* str);
        inline sp<Retval> split(String& buffer, const char* str){
            return split(buffer.toChars(), str);
        }

    };

}; // end namespace dframework;
#endif // end if cplusplus

#endif /* DFRAMEWORK_UTIL_STRINGARRAY_H */

