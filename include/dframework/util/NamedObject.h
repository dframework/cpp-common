#ifndef DFRAMEWORK_UTIL_NAMEDOBJECT_H
#define DFRAMEWORK_UTIL_NAMEDOBJECT_H

#include <dframework/base/Object.h>
#include <dframework/lang/String.h>

#ifdef __cplusplus
namespace dframework {

    class NamedObject : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(NamedObject);

        NamedObject();
        NamedObject(const char *name);
        NamedObject(const char *name, sp<Object>& obj);
        NamedObject(const String& name, sp<Object>& obj);
        NamedObject(const char *name, Object* obj/*cast strong pointer*/);
        NamedObject(const String& name, Object* obj/*cast strong pointer*/);
        virtual ~NamedObject();

        String m_sName;
        sp<Object> m_object;

        DFW_OPERATOR_EX_DECLARATION(NamedObject, m_sName);
    };

};
#endif

#endif /* DFRAMEWORK_UTIL_NAMEDOBJECT_H */

