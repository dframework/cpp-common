#ifndef DFRAMEWORK_UTIL_REGEXP_H
#define DFRAMEWORK_UTIL_REGEXP_H
#define DFRAMEWORK_UTIL_REGEXP_VERSION 1

#include <dframework/base/type.h>
#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>

typedef dfw_point_t   dfw_regexp_t;
typedef dfw_point_t   dfw_pcre2_code_t;
typedef dfw_point_t   dfw_pcre2_match_data_t;


#ifdef __cplusplus
namespace dframework {
    
    class Regexp : public Object
    {
    public:
        Regexp(const char *regexp);
        Regexp(const String &regexp);
        virtual ~Regexp();
        
    private:
        String m_sExp;
        String m_sValue;
        dfw_pcre2_code_t/*pcre2_code*/ *m_pPattern;
        dfw_pcre2_match_data_t/*pcre2_match_data*/ *m_pMatchData;
        
        String m_sErrorMessage;
        int m_iErrorCode;
        size_t/*PCRE2_SIZE*/ m_iErrorOffset;
        
        int m_iMatchCount;
        size_t/*PCRE2_SIZE*/ *m_iOffset; /*this is mMatchData's*/
        
        
        void ___init();
        sp<Retval> ___compile();
        sp<Retval> ___regexp();
        
    public:
        sp<Retval> regexp(const char *value);
        sp<Retval> regexp(const String &value);
        void clear();

        const char *getMatch(int index) const;
        String getMatchString(int index) const;
        dfw_ulong_t getMatchLength(int index) const;
        dfw_ulong_t getOffset(int index) const;
    
        inline int getMatchCount() const {
            return m_iMatchCount;
        }
        inline dfw_ulong_t getErrorOffset() const{
            return m_iErrorOffset;
        }
        inline int getErrorCode() const{
            return m_iErrorCode;
        }
        inline const String& getErrorMessage() const{
            return m_sErrorMessage;
        }
    };
    
    
    
    
};
#endif


#endif /* DFRAMEWORK_UTIL_REGEXP_H */
