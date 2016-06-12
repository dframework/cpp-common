#include <dframework/lang/String.h>
#include <stdarg.h>
#include <ctype.h>

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(String);

    String::String(){
        DFW_SAFE_ADD(String, l);
        ___init();
    }

    String::String(const char* str)
            : Object()
    {
        DFW_SAFE_ADD(String, l);
        ___init();
        set(str, String::strlen(str));
    }

    String::String(const char* str, dfw_size_t len)
            : Object()
    {
        DFW_SAFE_ADD(String, l);
        ___init();
        set(str, len);
    }

    String::String(const String& str)
            : Object()
    {
        DFW_SAFE_ADD(String, l);
        ___init();
        set(str.toChars(), str.length());
    }

    String::~String(){
        DFW_SAFE_REMOVE(String, l);
        DFW_FREE(m_pBuffer);
    }

    void String::___init(){
        m_pBuffer = NULL;
        m_iLength = 0;
        m_iCapacity = 0;
    }

    bool String::___alloc(const char* str, dfw_size_t len
                        , dfw_size_t curlen)
    {
        char* p  = NULL;
        char* np = NULL;
        dfw_size_t nlen = curlen + len;

        if(!nlen) {
            if(m_iLength>0){
                ::memset(m_pBuffer, 0, m_iLength);
                m_iLength = 0;
            }
            return true;
        }

        if(0==m_iCapacity){
            p = (char*)::malloc(nlen+1);
            if(!p) {
                //memset(m_pBuffer, 0, m_iLength);
                //m_iLength = 0;
                return false;
            }
            m_iCapacity = nlen;
        }else if(m_iCapacity<nlen){
            p = (char*)::realloc( (void*)m_pBuffer, nlen + 1);
            if(!p){
                if( !(np = (char*)::malloc(nlen+1)) ){
                    ::memset(m_pBuffer, 0, m_iLength);
                    m_iLength = 0;
                    return false;
                }
                if( curlen > 0 )
                    ::memcpy(np, m_pBuffer, curlen);
                DFW_FREE(m_pBuffer);
                p = np;
            }
            m_iCapacity = nlen;
        }else if( (m_iCapacity-nlen)!=0 ){
            p = m_pBuffer;
            ::memset(p+nlen, 0, m_iCapacity-nlen);
        }else{
            p = m_pBuffer;
        }

        ::memcpy(p+curlen, str, len);
        p[nlen] = '\0';
        m_pBuffer = p;
        m_iLength = nlen;

        return true;
    }

    bool String::set(const char* str){
        return ___alloc(str, String::strlen(str), 0);
    }

    bool String::set(const char* str, dfw_size_t len){
        //int sl = String::strlen(str);
        //return ___alloc(str, (sl<len ? sl : len), 0);
        return ___alloc(str, len, 0);
    }

    bool String::append(const char* str){
        return ___alloc(str, String::strlen(str), m_iLength);
    }

    bool String::append(const char* str, dfw_size_t len){
        //int sl = String::strlen(str);
        //return ___alloc(str, (sl<len ? sl : len), m_iLength);
        return ___alloc(str, len, m_iLength);
    }

    bool String::append(const String& str){
        return ___alloc(str.toChars(), str.length(), m_iLength);
    }

    bool String::append(const String& str, dfw_size_t len){
        return ___alloc(
                   str.toChars()
                 , (str.length()<len ? str.length() : len)
                 , m_iLength
        );
    }

    bool String::appendFmt(const char* fmt, ...){
        if(String::strlen(fmt)==0) return true;

        char *buf = NULL;
        size_t size;
        va_list ap, ap2;

        DFW_VALIST_SIZE(size, temp, fmt, ap);
        DFW_VALIST_STRDUP(buf, size, fmt, ap2);

        bool r = ___alloc(buf, size, m_iLength);
        DFW_FREE(buf);
        return r;
    }

    bool String::equals(const char *str) const{
        return equals(str, String::strlen(str));
    }

    bool String::equals(const char *str, dfw_size_t size) const{
        if(str==m_pBuffer)
            return true;
        if(empty()){
            if(size==0 || !str)
                return true;
            return false;
        }
        return (::strcasecmp(str, m_pBuffer)==0 ? true:false);
    }

    bool String::equals(const String &from) const{
        if(this==&from)
            return true;
        if(m_pBuffer==from.toChars())
            return true;
        if(m_iLength!=from.length()){
            return false;
        }
        return equals(from.toChars(), from.length());
    }

    dfw_size_t String::indexOf(const char needle) const{
        return indexOf(m_pBuffer, needle);
    }

    dfw_size_t String::indexOf(const char* needle) const{
        return indexOf(m_pBuffer, needle);
    }

    dfw_size_t String::lastIndexOf(const char needle) const{
        return lastIndexOf(m_pBuffer, needle);
    }

    dfw_size_t String::lastIndexOf(const char* needle) const{
        return lastIndexOf(m_pBuffer, needle);
    }

    String& String::trim(){
        if(length()==0){
            return *this;
        }

        dfw_size_t i = 0;
        while (i < m_iLength && isspace(m_pBuffer[i])) {
            ++i;
        }

        dfw_size_t j = m_iLength;
        while (j > i && isspace(m_pBuffer[j - 1])) {
            --j;
        }

        ::memmove(m_pBuffer, &m_pBuffer[i], j - i);
        m_iLength = j - i;
        m_pBuffer[m_iLength] = '\0';
        return (*this);
    }

    String& String::replace(const char from, const char to){
        for(dfw_size_t k=0; k<m_iLength; k++){
            if( from == m_pBuffer[k] )
                m_pBuffer[k] = to;
        }
        return (*this);
    }

    String& String::shift(dfw_size_t offset){
        if(0==offset){
            return (*this);
        }
        if(m_iLength<offset){
            offset = m_iLength;
        }
        if(m_iLength==offset){
            if( m_iLength != 0 ){
                m_pBuffer[0] = '\0';
            }
            m_iLength = 0;
            return (*this);
        }
        ::memmove(m_pBuffer, m_pBuffer+offset, m_iLength-offset+1);
        m_iLength -= offset;
        m_pBuffer[m_iLength] = '\0';
        return (*this);
    }

    String& String::chuck(dfw_size_t offset){
        if(m_iLength<offset){
            return (*this);
        }
        m_pBuffer[offset] = '\0';
        m_iLength = offset;
        return (*this);
    }

    String& String::substring(dfw_size_t start){
        return substring(start, m_iLength - start);
    }

    String& String::substring(dfw_size_t start, dfw_size_t length){
        if( m_iLength <= start ){
            DFW_FREE(m_pBuffer);
            m_iLength = 0;
            return (*this);
        }
        if( (length+start) > m_iLength )
            length = m_iLength - start;
        
        ::memmove(m_pBuffer, m_pBuffer+start, length);
        m_iLength = length;
        m_pBuffer[length] = '\0';
        return (*this);
    }

#if 0
    //
    sp<StringArray> String::splite(const char* str){
        sp<StringArray> arr = new StringArray();
        const char* buf = m_pBuffer;
        if(m_iLength){
            dfw_size_t size;
            const char* p;
            while(true){
                if( !(p = strstr(buf, str)) ){
                    sp<String> ele = new String();
                    ele.set(buf);
                    arr->m_aList.insert(ele);
                    break;
                }
                if(buf==p){
                    buf++;
                    continue;
                }
                size = p - buf;
                sp<String> ele = new String();
                ele.set(buf, size);
                arr->m_aList.insert(ele);
                buf += size;
            }
        }
        return arr;
    }
#endif

    String& String::operator=(const char *str){
        set(str, String::strlen(str));
        return (*this);
    }

    String& String::operator=(const String &from){
        set(from.toChars(), from.length());
        return (*this);
    }

    /* static */
    dfw_size_t String::strlen(const char *str){
        if(str)
            return ::strlen(str);
        return 0;
    }

    /* static */
    String String::format(const char *fmt, ...){
        if(String::strlen(fmt)==0)
            return NULL;

        char *buf = NULL;
        size_t isize;
        va_list ap, ap2;

        va_start(ap, fmt);
        isize = ::vsnprintf(NULL, 0, fmt, ap);
        va_end(ap);
        if((size_t)-1==isize)
            return NULL;

        if( NULL == (buf = (char*)::malloc(isize+2)) )
            return NULL;

        va_start(ap2, fmt);
        ::vsnprintf(buf, isize+2, fmt, ap2);
        buf[isize+1] = 0;
        va_end(ap2);

        String r = buf;
        DFW_FREE(buf);
        return r;
    }

    /* static */
    char* String::strdup(const char* str)
    {
        char* buf;
        unsigned int size;
        if( !str )
            return NULL;
        if( !(size = strlen(str)) )
            return NULL;
        if( !(buf = (char*)::malloc(size+1)) )
            return NULL;
        ::memcpy(buf, str, size);
        buf[size] = '\0';
        return buf;
    }

    char* String::strndup(const char* str, int len)
    {
        char* buf;
        if( !str )
            return NULL;
        if( !(buf = (char*)malloc(len+1)) )
            return NULL;
        memcpy(buf, str, len);
        buf[len] = '\0';
        return buf;
    }

    /* static */
    bool String::isspace(const char ch){
        switch(ch){
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                return true;
        }
        return false;
    }

    /* static */
    dfw_size_t String::indexOf(const char *hay, const char needle){
        if(!hay)
            return -1;
        char needles[2]; needles[0] = needle; needles[1] = '\0';
        const char *p = ::strstr(hay, (const char *)needles);
        if (p)
            return p - hay;
        return -1;
    }

    /* static */
    dfw_size_t String::indexOf(const char *hay, const char *needle){
        if(!hay || !needle)
            return -1;
        const char *p = ::strstr(hay, (const char *)needle);
        if (p)
            return p - hay;
        return -1;
    }

    /* static */
    dfw_size_t String::lastIndexOf(const char *hay, const char needle){
        if(!hay || !needle)
            return -1;
        char needles[2]; needles[0] = needle; needles[1] = '\0';
        return lastIndexOf(hay, needles);
    }

    /* static */
    dfw_size_t String::lastIndexOf(const char *hay, const char *needle){
        if(!hay || !needle)
            return -1;
#ifdef HAVE_STRRSTR
        char *p = ::strrstr(hay, needle);
        if(!p){
            return -1;
        }
        return (p-hay);
#else
        dfw_size_t stringlen, findlen;
        char *cp;

        findlen = ::strlen(needle);
        stringlen = ::strlen(hay);
        if (findlen > stringlen)
                return -1;

        for (cp = (char*)(hay + stringlen - findlen); cp >= hay; cp--){
            if (::strncmp(cp, needle, findlen) == 0){
                return (cp-hay);
            }
        }

        return -1;
#endif
    }

    int String::strncmp_s(const char* dst, dfw_size_t size){
        dfw_size_t min = (m_iLength<size) ? m_iLength : size;
        char *s = m_pBuffer;
        char *d = (char*)dst;
        dfw_size_t k = 0;
        while(k<min){
            if( s[k] < d[k] ){
		 return 1;
		}
            if( s[k] > d[k] ){
		 return -1;
		}
            k++;
        }
        if(m_iLength==size) return 0;
        if(m_iLength<size) return 1;
        return -1;
    }


#define DFW_STRING_PSTRCMP() (strncmp_s(from))
#define DFW_STRING_RSTRCMP() (strncmp_s(from))

    bool String::operator == (const String & from){
        return !(*this != from);
    }

    bool String::operator != (const String & from){
        if( !empty() && !from.empty()){
            return ((0 != DFW_STRING_RSTRCMP()) ? true : false);
        }
        if( empty() && from.empty()) return false;
        return true;
    }

    bool String::operator >  (const String & from){
        return !(*this <= from);
    }

    bool String::operator <  (const String & from){
        return !(*this >= from);
    }

    bool String::operator >= (const String & from){
        if( !empty() && !from.empty()){
            return ((0 < DFW_STRING_RSTRCMP()) ? false : true);
        }
        if( empty() && from.empty()) return true;
        if( empty() ) return false;
        return true;
    }

    bool String::operator <= (const String & from){
        if( !empty() && !from.empty()){
            return ((0 > DFW_STRING_RSTRCMP()) ? false : true);
        }
        if( empty() && from.empty()) return true;
        if( empty() ) return true;
        return false;
    }

};

using namespace dframework;


char* dfw_strdup_valist(dfw_size_t size, const char* fmt, va_list ap)
{
    if(size==0) return NULL;
    size += 2;

    char* buf = (char*)::malloc(size);
    if(!buf) return NULL;

    ::memset(buf, 0, size);
    ::vsnprintf(buf, size-1, fmt, ap);

    return buf;
}


#ifdef _WIN32
char* strcasestr(const char* s1, const char* s2)
{
        char *p1, *p2;
        if(*s2 == '\0') return((char*)s1);

        while(1) {
                for ( ; (*s1 != '\0') && (::tolower(*s1) != ::tolower(*s2)); s1++);
                if (*s1 == '\0')
                        return(NULL);
                /* found first character of s2, see if the rest matches */
                p1 = (char*)s1;
                p2 = (char*)s2;
                for (++p1, ++p2; ::tolower(*p1) == ::tolower(*p2); ++p1, ++p2) {
                        if (*p1 == '\0') {
                                /* both strings ended together */
                                return((char *)s1);
                        }
                }
                if (*p2 == '\0') {
                        /* second string ended, a match */
                        break;
                }
                /* didn't find a match here, try starting at next character in s1 */
                s1++;
        }
        return((char*)s1);
}
#endif

