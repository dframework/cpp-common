#ifndef DFRAMEWORK_LANG_INTEGER_H
#define DFRAMEWORK_LANG_INTEGER_H
#define DFRAMEWORK_LANG_INTEGER_VERSION 1

#include <dframework/base/Object.h>
#include <dframework/util/D_Sorted.h>
#include <dframework/lang/String.h>

#ifdef __cplusplus
namespace dframework {
    
    class Integer : public Object
    {
    private:
        int m_iValue;
        
    public:
        Integer();
        Integer(int val);
        Integer(Integer& val);
        inline virtual ~Integer() {}
        
        inline int value() const { return m_iValue; }

        static int parseInt(const char* str, int defaultVal=0);
        inline static int parseInt(const String& str, int defaultVal=0){
            return parseInt(str.toChars(), defaultVal);
        }

        virtual Integer& operator = (const int val);
        virtual Integer& operator = (const Integer& val);

        DFW_OPERATOR_EX_DECLARATION(Integer, m_iValue);

    };
    
};
#endif


#endif /* DFRAMEWORK_LANG_INTEGER_H */
