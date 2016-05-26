#include <dframework/util/NamedObject.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(NamedObject);

    NamedObject::NamedObject(){
        DFW_SAFE_ADD(NamedObject, l);
    }

    NamedObject::NamedObject(const char *name) 
            : Object()
    {
        DFW_SAFE_ADD(NamedObject, l);
        m_sName = name;
        m_object = NULL;
    }

    NamedObject::NamedObject(const char *name, sp<Object>& obj)
            : Object()
    {
        DFW_SAFE_ADD(NamedObject, l);
        m_sName = name;
        m_object = obj;
    }

    NamedObject::NamedObject(const String& name, sp<Object>& obj)
            : Object()
    {
        DFW_SAFE_ADD(NamedObject, l);
        m_sName = name;
        m_object = obj;
    }

    NamedObject::NamedObject(const char *name, Object* obj)
            : Object()
    {
        DFW_SAFE_ADD(NamedObject, l);
        m_sName = name;
        m_object = obj;
    }

    NamedObject::NamedObject(const String& name, Object* obj)
            : Object()
    {
        DFW_SAFE_ADD(NamedObject, l);
        m_sName = name;
        m_object = obj;
    }

    NamedObject::~NamedObject(){
        DFW_SAFE_REMOVE(NamedObject, l);
    }

};

