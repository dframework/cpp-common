#include <dframework/util/NamedValue.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(NamedValue);

    NamedValue::NamedValue(){
        DFW_SAFE_ADD(NamedValue, l);
    }

    NamedValue::NamedValue(const char *name) : dframework::Object::Object()
    {
        DFW_SAFE_ADD(NamedValue, l);
        m_sName = name;
    }

    NamedValue::NamedValue(const String& name):dframework::Object::Object()
    {
        DFW_SAFE_ADD(NamedValue, l);
        m_sName = name;
    }

    NamedValue::NamedValue(const char *name, const char *value)
            : dframework::Object::Object()
    {
        DFW_SAFE_ADD(NamedValue, l);
        m_sName = name;
        m_sValue = value;
    }

    NamedValue::NamedValue(const String& name, const String &value)
            : dframework::Object::Object()
    {
        DFW_SAFE_ADD(NamedValue, l);
        m_sName = name;
        m_sValue = value;
    }

    NamedValue::~NamedValue(){
        DFW_SAFE_REMOVE(NamedValue, l);
    }

};

