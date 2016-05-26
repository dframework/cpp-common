#include <dframework/lang/Integer.h>

namespace dframework {
    
    Integer::Integer(){
        m_iValue = 0;
    }
    
    Integer::Integer(int val)
            : Object()
    {
        m_iValue = val;
    }
    
    Integer::Integer(Integer& val)
            : Object()
    {
        m_iValue = val.value();
    }

    /* static */
    int Integer::parseInt(const char* str, int defaultVal){
        if(!str) return defaultVal;
        return ::atoi(str);
    }

    Integer& Integer::operator = (const int val) {
        m_iValue = val;
        return *this;
    }

    Integer& Integer::operator = (const Integer& val) {
        m_iValue = val.m_iValue;
        return *this;
    }

};

