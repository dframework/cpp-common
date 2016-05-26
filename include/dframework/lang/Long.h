#ifndef DFRAMEWORK_LANG_LONG_H
#define DFRAMEWORK_LANG_LONG_H
#define DFRAMEWORK_LANG_LONG_VERSION 1

#include <dframework/base/Object.h>
#include <dframework/util/D_Sorted.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/String.h>

#ifdef __cplusplus
namespace dframework {
    
    class Long : public Object
    {
    private:
        long m_iValue;
        
    public:
        Long();
        Long(long val);
        Long(Long& val);
        inline virtual ~Long() {}
        
        inline long value() const { return m_iValue; }

        static long parseLong(const char* str, long defaultVal=0);
        inline static long parseLong(const String& str, long defaultVal=0){
            return parseLong(str.toChars(), defaultVal);
        }

        virtual Long& operator = (const long val);
        virtual Long& operator = (const Integer& val);
        virtual Long& operator = (const Long& val);

        DFW_OPERATOR_EX_DECLARATION(Long, m_iValue);

    };
    
};
#endif


#endif /* DFRAMEWORK_LANG_LONG_H */
