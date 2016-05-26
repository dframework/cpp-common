#include <dframework/http/HttpFormData.h>

namespace dframework{

    HttpFormData::HttpFormData(const char *name, const char *value)
            : Object()
    {
        m_oUri = NULL;
        m_sName = name;
        m_sValue = value;
        m_bIsUri = false;
    }

    HttpFormData::HttpFormData(String& name, String& value)
            : Object()
    {
        m_oUri = NULL;
        m_sName = name;
        m_sValue = value;
        m_bIsUri = false;
    }

    HttpFormData::HttpFormData(
          const char *name, const char *uri, const char *type)
            : Object()
    {
        m_oUri = uri;
        m_sName = name;
        m_sType = type;
        m_bIsUri = true;
    }

    HttpFormData::HttpFormData(String& name, String& uri, String& type)
            : Object()
    {
        m_oUri = uri;
        m_sName = name;
        m_sType = type;
        m_bIsUri = true;
    }

    HttpFormData::~HttpFormData(){
    }

};

