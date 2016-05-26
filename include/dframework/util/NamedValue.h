#ifndef DFRAMEWORK_UTIL_NAMEDVALUE_H
#define DFRAMEWORK_UTIL_NAMEDVALUE_H
#define DFRAMEWORK_UTIL_NAMEDVALUE_VERSION 1

#include <dframework/base/Object.h>
#include <dframework/lang/String.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>

#ifdef __cplusplus
namespace dframework {

    class NamedValue : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(NamedValue);

        NamedValue();
        NamedValue(const char *name);
        NamedValue(const String& name);
        NamedValue(const char *name, const char *value);
        NamedValue(const String& name, const String& value);
        virtual ~NamedValue();
        String m_sName;
        String m_sValue;

        inline int getInt() {
            return Integer::parseInt(m_sValue);
        }

        inline unsigned getUnsigned() {
            return (unsigned)Integer::parseInt(m_sValue);
        }

        inline unsigned getUnsignedInt() {
            return (unsigned)Integer::parseInt(m_sValue);
        }

        inline long getLong() {
            return Long::parseLong(m_sValue);
        }

        inline unsigned long getUnsignedLong() {
            return (unsigned long)Long::parseLong(m_sValue);
        }

        inline const char* getChars() {
            return m_sValue.toChars();
        }

        DFW_OPERATOR_EX_DECLARATION(NamedValue, m_sName);
    };

};
#endif

#endif /* DFRAMEWORK_UTIL_NAMEDVALUE_H */

