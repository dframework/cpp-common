#include <dframework/util/Regexp.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#ifdef __APPLE__
#include "pcre2.h"
#else
#include <pcre2.h>
#endif

#ifdef _WIN32

#endif

namespace dframework {

    Regexp::Regexp(const char *regexp){
        ___init();
        m_sExp = regexp;
        ___compile();
    }
    
    Regexp::Regexp(const String &regexp){
        ___init();
        m_sExp = regexp;
        ___compile();
    }
    
    void Regexp::___init(){
        m_pPattern = NULL;
        m_pMatchData = NULL;
        m_iOffset = 0;
        m_iErrorCode = 0;
        m_iErrorOffset = 0;
        m_iMatchCount = 0;
    }
    
    Regexp::~Regexp(){
        clear();
    }
    
    void Regexp::clear(){
        if(m_pPattern){
            ::pcre2_code_free((pcre2_code*)m_pPattern);
            m_pPattern = NULL;
        }
        if(m_pMatchData){
            ::pcre2_match_data_free((pcre2_match_data *)m_pMatchData);
            m_pMatchData = NULL;
        }
        m_iOffset = 0;
        m_iErrorCode = 0;
        m_iErrorOffset = 0;
        m_sErrorMessage = "";
        m_iMatchCount = 0;
    }
    
    sp<Retval> Regexp::___compile(){
        clear();

        if(!m_sExp.length()){
            m_iErrorCode = -1;
            m_sErrorMessage = "RegExp string is empty.";
            return DFW_RETVAL_NEW(DFW_E_INVAL,0);
        }

        m_pPattern = (dfw_pcre2_code_t*)::pcre2_compile(
                                            (PCRE2_SPTR)m_sExp.toBytes()
                                            , -1, 0
                                            , &m_iErrorCode
                                            , &m_iErrorOffset, NULL);
        if(NULL==m_pPattern){
            PCRE2_UCHAR8 buffer[120];
            (void)::pcre2_get_error_message(m_iErrorCode, buffer, 120);
            if(!m_iErrorCode)
                m_iErrorCode = -1;
            m_sErrorMessage = ((char *)buffer);
            return DFW_RETVAL_NEW(DFW_E_REGEXP_NOT_PATTERN,0);
        }

        return NULL;
    }
    
    sp<Retval> Regexp::regexp(const char *value){
        m_sValue = value;
        return ___regexp();
    }
    
    sp<Retval> Regexp::regexp(const String &value){
        m_sValue = value;
        return ___regexp();
    }
    
    sp<Retval> Regexp::___regexp(){
        m_iMatchCount = 0;
        if(m_pMatchData){
            ::pcre2_match_data_free((pcre2_match_data *)m_pMatchData);
            m_pMatchData = NULL;
        }
        m_iOffset = 0;
        
        if(NULL==m_pPattern){
            return DFW_RETVAL_NEW(DFW_E_REGEXP_NOT_PATTERN,0);
        }else if(!m_sExp.length()){
            m_iErrorCode = -1;
            m_sErrorMessage = "RegExp string is empty.";
            return DFW_RETVAL_NEW(DFW_E_REGEXP_NOT_PATTERN,0);
        }else if(!m_sValue.length()){
            m_iErrorCode = -1;
            m_sErrorMessage = "Value string is empty.";
            return DFW_RETVAL_NEW(DFW_E_INVAL,0);
        }
        
        const char* value = m_sValue.toChars();
        m_pMatchData = (dfw_pcre2_match_data_t*)::pcre2_match_data_create(10, NULL);
        m_iMatchCount = ::pcre2_match(
                            (pcre2_code*)m_pPattern
                          , (PCRE2_SPTR)value
                          , -1, 0, 0
                          , (pcre2_match_data*)m_pMatchData
                          , NULL);
        if (m_iMatchCount <= 0){
            m_iErrorCode = -1;
            m_sErrorMessage = "No match.";
            return DFW_RETVAL_NEW(DFW_E_REGEXP_NOT_MATCH,0);
        }
        
        m_iOffset = ::pcre2_get_ovector_pointer(
                        (pcre2_match_data*)m_pMatchData);
        return NULL;
    }
    
    const char *Regexp::getMatch(int index) const{
        if(index<0) return NULL;
        if(m_sValue.length() && m_iOffset && (m_iMatchCount>index)){
            unsigned offidx = 2*index;
            if(m_sValue.length()>offidx){
                return ((char*) (
                    (PCRE2_SPTR)(m_sValue.toChars())
                    + m_iOffset[offidx]
                ));
            }
        }
        return NULL;
    }

    String Regexp::getMatchString(int index) const{
        String s;
        s.set(getMatch(index), getMatchLength(index));
        return s;
    }
    
    dfw_ulong_t Regexp::getMatchLength(int index) const{
        if(index<0) return -1;
        if(m_iOffset && (m_iMatchCount>index) ){
            return (m_iOffset[2*index+1] - m_iOffset[2*index]);
        }
        return -1;
    }
    
    dfw_ulong_t Regexp::getOffset(int index) const{
        if(m_iOffset && (m_iMatchCount>index)){
            return m_iOffset[index];
        }
        return -1;
    }
    
    
};

