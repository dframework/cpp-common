#ifndef DFRAMEWORK_LANG_STRING_H
#define DFRAMEWORK_LANG_STRING_H
#define DFRAMEWORK_LANG_STRING_VERSION 1


# define DFW_VALIST_SIZE(size,temp,fmt,ap) {            \
    va_start(ap, fmt);                                  \
    size = ::vsnprintf(NULL, 0, fmt, ap);               \
    va_end(ap); }

#define DFW_VALIST_STRDUP(buf,size,fmt,ap) {            \
    va_start(ap, fmt);                                  \
    buf = ::dfw_strdup_valist(size, fmt, ap);           \
    va_end(ap); }

#define DFW_VALIST_BEGIN_DECLARE(fmt) {                 \
    char *buf = NULL;                                   \
    size_t size;                                        \
    va_list ap, ap2;                                    \
    DFW_VALIST_SIZE(size, temp, fmt, ap);               \
    DFW_VALIST_STRDUP(buf, size, fmt, ap2);

#define DFW_VALIST_GET_VALUE(x) x = buf;

#define DFW_VALIST_END_DECLARE() }


#include <dframework/base/Object.h>
#include <dframework/util/D_Sorted.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

char *dfw_strdup_valist(dfw_size_t size, const char* fmt, va_list ap);
#ifdef _WIN32
char* strcasestr(const char* s1, const char* s2);
#endif

#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
namespace dframework {

    class String : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(String);

    private:
        char* m_pBuffer;
        dfw_size_t m_iLength;
        dfw_size_t m_iCapacity;

        void ___init();
        bool ___alloc(const char*, dfw_size_t, dfw_size_t);

    public:
        String();
        String(const char* str);
        String(const char* str, dfw_size_t len);
        String(const String& str);
        virtual ~String();

        bool set(const char* str);
        bool set(const char* str, dfw_size_t len);
        bool append(const char* str);
        bool append(const char* str, dfw_size_t len);
        bool append(const String& str);
        bool append(const String& str, dfw_size_t len);
        bool appendFmt(const char* fmt, ...);

        inline dfw_size_t length() const { 
            return m_iLength; 
        }

        inline const dfw_byte_t* toBytes() const { 
            return (m_iLength==0 ? NULL : (dfw_byte_t*)m_pBuffer);
        }

        inline const char* toChars() const { 
            return (m_iLength==0 ? NULL : m_pBuffer);
        }

        inline bool empty() const {
            return (m_iLength==0 ? true : false);
        }

        bool equals(const char* from) const;
        bool equals(const char* from, dfw_size_t size) const;
        bool equals(const String& from) const;

        int indexOf(const char) const;
        int indexOf(const char*) const;
        int lastIndexOf(const char) const;
        int lastIndexOf(const char*) const;

        String& trim();

        String& replace(const char from, const char to);

        String& shift(dfw_size_t offset);
        String& chuck(dfw_size_t offset);

        String& substring(dfw_size_t start);
        String& substring(dfw_size_t start, dfw_size_t length);
       
        String& operator = (const char* str); 
        String& operator = (const String& str); 

        static dfw_size_t strlen(const char* str);
        static String format(const char* fmt, ...);
        static char* strdup(const char* str);
        static char* strndup(const char* str, int len);
        static bool isspace(const char ch);

        static int indexOf(const char *hay, const char need);
        static int indexOf(const char *hay, const char* need);
        static int lastIndexOf(const char *hay, const char need);
        static int lastIndexOf(const char *hay, const char* need);

        int strncmp_s(const char* s, dfw_size_t size);
        inline int strncmp_s(const String& s) { 
            return strncmp_s(s.toChars(), s.length());
        }

        inline dfw_byte_t getByte(dfw_size_t pos){
            return ((dfw_byte_t)(m_iLength>pos ? m_pBuffer[pos] : -1));
        }
        inline char getChar(dfw_size_t pos){
            return ((char)(m_iLength>pos ? m_pBuffer[pos] : -1));
        }

        DFW_OPERATOR_EXP_DECLARATION(String);

    };

};
#endif

#endif /* DFRAMEWORK_LANG_STRING_H */

