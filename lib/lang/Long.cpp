#include <dframework/lang/Long.h>

namespace dframework {
    
    Long::Long(){
        m_iValue = 0l;
    }
    
    Long::Long(long val)
            : Object()
    {
        m_iValue = val;
    }
    
    Long::Long(Long& val)
            : Object()
    {
        m_iValue = val.value();
    }

    /* static */
    long Long::parseLong(const char* str, long defaultVal){
        if(!str) return defaultVal;
        return ::atol(str);
    }

    Long& Long::operator = (const long val) {
        m_iValue = val;
        return *this;
    }

    Long& Long::operator = (const Integer& val) {
        m_iValue = val.value();
        return *this;
    }

    Long& Long::operator = (const Long& val) {
        m_iValue = val.m_iValue;
        return *this;
    }

};

