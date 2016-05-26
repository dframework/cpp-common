#ifndef DFRAMEWORK_HTTP_HTTPFORMDATA_H
#define DFRAMEWORK_HTTP_HTTPFORMDATA_H


#include <dframework/base/Retval.h>
#include <dframework/base/Object.h>
#include <dframework/lang/String.h>
#include <dframework/net/URI.h>


#ifdef __cplusplus
namespace dframework {

    class HttpFormData : public Object
    {
    public:
        String        m_sName;
        String        m_sValue;
        String        m_sUri;
        String        m_sType;

        bool          m_bIsUri;
        URI           m_oUri;
        URI::FileInfo m_oFileInfo;
        String        m_sBuffer;
        dfw_size_t    m_nContentLength; /* sValue or sUri contents length */

        HttpFormData(const char *name, const char *value);
        HttpFormData(String& name, String& value);
        HttpFormData(const char *name, const char *uri, const char *type);
        HttpFormData(String& name, String& uri, String& type);
        virtual ~HttpFormData();
    };

};
#endif


#endif /* DFRAMEWORK_HTTP_HTTPFORMDATA_H */

