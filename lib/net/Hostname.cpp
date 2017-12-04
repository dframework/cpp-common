#include <dframework/net/Hostname.h>
#include <dframework/util/Regexp.h>
#include <time.h>
#include <sys/types.h>

#ifdef __ANDROID__
#include <netinet/in.h>
#endif

#ifndef _WIN32
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/nameser.h>
#else
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
#endif

#if defined(_WIN32) || defined(__ANDROID__)
# ifndef AF_INET6
#  define AF_INET6     28
# endif
# ifndef NS_IN6ADDRSZ
# define NS_IN6ADDRSZ  16      /* IPv6 T_AAAA */
# endif
# ifndef NS_INT16SZ
# define NS_INT16SZ    2       /* #/bytes of data in a u_int16_t */
# endif
#endif

namespace dframework {
    
    Hostname::Result::Result(const String& ip, int type, void* ai_addr, int ai_addr_len)
            : Object()
    {
        if(ai_addr_len){
            m_SockAddr = (void*)malloc(ai_addr_len);
            memcpy(m_SockAddr, ai_addr, ai_addr_len);
        }else{
            m_SockAddr = NULL;
        }
        
        m_iSockAddrLen = ai_addr_len;
        m_sIp = ip;
        m_iAddrType = type;
    }
    
    Hostname::Result::~Result(){
        if(m_SockAddr){
            ::free(m_SockAddr);
            m_SockAddr = NULL;
        }
    }

    // --------------------------------------------------------------

    Hostname::CacheData::CacheData(const String& host) 
            : Object()
    {
        m_sHost = host;
        m_uCacheTime = time(NULL);
    }

    Hostname::CacheData::CacheData(const String& host, const Hostname& hn)
            : Object()
    {
        m_sHost = host;
        m_uCacheTime = time(NULL);

        int size = hn.size();
        for(int k=0; k<size; k++){
            sp<Hostname::Result> result = hn.getResult(k);
            if(result.has()){
                sp<Hostname::Result> nr
                    = new Hostname::Result(result->m_sIp, result->m_iAddrType, result->m_SockAddr, result->m_iSockAddrLen);
                m_aResultList.insert(nr);
            }
        }
    }

    // --------------------------------------------------------------

    sp<Retval> Hostname::Cache::append(const String& host, Hostname& hn){
        AutoLock _l(this);
        if(hn.size()==0)
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        sp<Hostname::CacheData> cd = new Hostname::CacheData(host, hn);
        sp<Hostname::CacheData> rm = m_aCacheList.remove(cd);
        return m_aCacheList.insert(cd);
    }

    sp<Retval> Hostname::Cache::search(const String& host, Hostname& hn){
        AutoLock _l(this);
        sp<Hostname::CacheData> cd = new Hostname::CacheData(host);
        sp<Hostname::CacheData> find = m_aCacheList.get(cd);
        if(find.has())
            return hn.setCacheData(find);
        return DFW_RETVAL_NEW(DFW_E_NOT_CACHE_HOST, 0);
    }

    int Hostname::Cache::size(){
        AutoLock _l(this);
        return m_aCacheList.size();
    }

    sp<Hostname::CacheData> Hostname::Cache::get(int position){
        AutoLock _l(this);
        return m_aCacheList.get(position);
    }

    // --------------------------------------------------------------

    /* static */
    Hostname::Cache Hostname::m_Cache;

    /* static */
    int Hostname::CACHE_TIME_SEC = 60;

    Hostname::Hostname(){
    }

    Hostname::Hostname(String& out, const char* uri, bool bCache)
            : Object()
    {
        get(out, uri, bCache);
    }

    Hostname::Hostname(String& out, const String& uri, bool bCache)
            : Object()
    {
        get(out, uri, bCache);
    }

    Hostname::Hostname(String& out, const URI& uri, bool bCache)
            : Object()
    {
        get(out, uri, bCache);
    }

    Hostname::~Hostname(){
        clear();
    }

    sp<Retval> Hostname::get(const char* uri, bool bCache){
        clear();
        m_Uri = uri;
        return (m_LastRetval = parse_start(bCache));
    }

    sp<Retval> Hostname::get(const String& uri, bool bCache){
        clear();
        m_Uri = uri;
        return (m_LastRetval = parse_start(bCache));
    }

    sp<Retval> Hostname::get(const URI& uri, bool bCache){
        clear();
        m_Uri = uri;
        return (m_LastRetval = parse_start(bCache));
    }

    sp<Retval> Hostname::get(String& out, const char* uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            out = ip(0);
        return retval;
    }

    sp<Retval> Hostname::get(String& out, const String& uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            out = ip(0);
        return retval;
    }

    sp<Retval> Hostname::get(String& out, const URI& uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            out = ip(0);
        return retval;
    }

    String Hostname::getIp(const char* uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            return ip(0);
        return NULL;
    }

    String Hostname::getIp(const String& uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            return ip(0);
        return NULL;
    }

    String Hostname::getIp(const URI& uri, bool bCache){
        sp<Retval> retval = get(uri, bCache);
        if(!retval.has())
            return ip(0);
        return NULL;
    }

    void Hostname::clear(){
        m_uCacheTime = 0;
        m_aResultList.clear();
        m_aCacheList.clear();
    }

    int Hostname::size() const{
        if(m_uCacheTime>0){
            return m_aCacheList.size();
        }
        return m_aResultList.size();
    }

    sp<Hostname::Result> Hostname::getResult(int position) const{
        if(m_uCacheTime>0){
            return m_aCacheList.get(position);
        }
        return m_aResultList.get(position);
    }

    String Hostname::ip(int position) const{
        sp<Result> result = getResult(position);
        if(result.has())
            return result->m_sIp;
        return NULL;
    }

    sp<Retval> Hostname::addCache(sp<Hostname::Result> result){
        sp<Hostname::Result> ns 
              = new Hostname::Result(result->m_sIp, result->m_iAddrType, result->m_SockAddr, result->m_iAddrType);
        sp<Retval> retval = m_aCacheList.insert(ns);
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Hostname::setCacheData(sp<Hostname::CacheData> cd){
        if(!cd.has())
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        int size = cd->m_aResultList.size();
        if( size <= 0 ) 
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        m_uCacheTime = cd->m_uCacheTime;
        for(int k=0; k<size; k++){
            sp<Hostname::Result> result = cd->m_aResultList.get(k);
            if(result.has()){
                sp<Retval> retval = addCache(result);
                if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
            }
        }

        return NULL;
    }

    sp<Retval> Hostname::parse_start(bool bCache){
        sp<Retval> retval;
        if( DFW_RET(retval, parse_start_l(bCache)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> Hostname::parse_start_l(bool bCache){
        sp<Retval> retval;

        String hostip = m_Uri.getHost();
        hostip.trim();
        if(hostip.empty())
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        //if( !DFW_RET(retval, parse_addrtype(hostip)) )
        //    return NULL;

        dfw_ulong_t cacheTime = 0;
        if(bCache && (!DFW_RET(retval, m_Cache.search(hostip, *this))) ){
            if( m_uCacheTime>((dfw_ulong_t)time(NULL)-CACHE_TIME_SEC) ){
                return NULL;
            }
        }

        cacheTime = m_uCacheTime;
        m_uCacheTime = 0;

        if( !DFW_RET(retval, parse_hostbyname(hostip)) ){
            m_Cache.append(hostip, *this);
            return NULL;
        }

        if(cacheTime>0){
            m_uCacheTime = cacheTime;
            return NULL;
        }

        return DFW_RETVAL_D(retval);
    }

    /*
    sp<Retval> Hostname::parse_addrtype(const String& hostip)
    {
        sp<Retval> retval;

        Regexp regexp("(^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$)");
        retval = regexp.regexp(hostip);
        if( !retval.has() ){
            sp<Result> result = new Result(hostip, AF_INET);
            return m_aResultList.insert(result);
        }

        Regexp regexp2("(^[0-9a-fA-F]{0,4}:[0-9a-fA-F]{0,4}:[0-9a-fA-F]{0,4}:[0-9a-fA-F]{0,4}:[0-9a-fA-F]{0,4}:[0-9a-fA-F]{0,4})");
        retval = regexp2.regexp(hostip);
        if( !retval.has() ){
            sp<Result> result = new Result(hostip, AF_INET6);
            return m_aResultList.insert(result);
        }

        Regexp regexp3("(^::1$)");
        retval = regexp3.regexp(hostip);
        if( !retval.has() ){
            sp<Result> result = new Result(hostip, AF_INET6);
            return m_aResultList.insert(result);
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR,0, "input=%s", hostip.toChars());
    }
    */

    sp<Retval> Hostname::parse_hostbyname(const String& hostip)
    {
        sp<Retval> retval;
        //FIXME: unused: int eno;
        int retry = 0;
        char addrstr[128];

#define USE_GETADDRINFO 1
#ifdef USE_GETADDRINFO
        /* FILES: /etc/gai.conf */
        struct addrinfo *hptr = NULL;
        struct addrinfo hints;
        ::memset (&hints, 0, sizeof (hints));
#ifdef USE_GAI_ADDRCONFIG /* added in the RFC3493 API */
        hints.ai_flags = AI_ADDRCONFIG;
#endif
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        do{
            hptr = NULL;
            int eno = ::getaddrinfo(hostip.toChars(), NULL, &hints, &hptr);
            if(eno){
                switch(eno){
                case EAI_AGAIN:
                    if(retry>10)
                        return DFW_RETVAL_NEW(DFW_E_AGAIN, eno);
                    usleep(1000);
                    retry++;
                    continue;
                case EAI_BADFLAGS :
                    return DFW_RETVAL_NEW(DFW_E_BADFLAGS, eno);
                case EAI_FAIL :
                    return DFW_RETVAL_NEW(DFW_E_FAIL, eno);
                case EAI_FAMILY :
                    return DFW_RETVAL_NEW(DFW_E_FAMILY, eno);
                case EAI_MEMORY:
                    return DFW_RETVAL_NEW(DFW_E_NOMEM, eno);
                case EAI_NODATA:
                    return DFW_RETVAL_NEW(DFW_E_NODATA, eno);
                case EAI_NONAME:
                    return DFW_RETVAL_NEW(DFW_E_NONAME, eno);
                case EAI_SERVICE:
                    return DFW_RETVAL_NEW(DFW_E_SERVICE, eno);
                case EAI_SOCKTYPE:
                    return DFW_RETVAL_NEW(DFW_E_SOCKTYPE, eno);
#ifndef _WIN32
                case EAI_ADDRFAMILY :
                    return DFW_RETVAL_NEW(DFW_E_ADDRFAMILY, eno);
                case EAI_SYSTEM:
                    return DFW_RETVAL_NEW(DFW_E_SYSTEM, eno);
#endif
                }
                return DFW_RETVAL_NEW(DFW_E_GETADDRINFO, eno);
            }
            break;
        }while(true);

        void *ptr;
        while(hptr){
            ::memset( addrstr , 0 , 128);
            switch(hptr->ai_family){
                case AF_INET:
                    ptr = &((struct sockaddr_in *) hptr->ai_addr)->sin_addr;
                    if(DFW_RET(retval, inet_ntop4((const unsigned char *)ptr, addrstr, 112)))
                        return DFW_RETVAL_D(retval);
                    break;
                case AF_INET6:
                    ptr = &((struct sockaddr_in6 *) hptr->ai_addr)->sin6_addr;
                    if(DFW_RET(retval, inet_ntop6((const unsigned char *)ptr, addrstr, 112)))
                        return DFW_RETVAL_D(retval);
                    break;
                default :
                    return DFW_RETVAL_NEW(DFW_E_ADDRTYPE, 0);
            }
            
            String hostip = addrstr;
            sp<Result> result = new Result(hostip, hptr->ai_family, hptr->ai_addr, hptr->ai_addrlen);
            if( !m_aResultList.get(result).has() )
                m_aResultList.insert(result);

            hptr = hptr->ai_next;
        }

#ifndef __ANDROID__
        ::freeaddrinfo(hptr);
#endif
        return NULL;
#else
        unsigned char **pptr;
        struct hostent *hptr;

        do{
#if defined(__MINGW32__) || defined(__MSYS__)
            if( (hptr=d7c_mingw_gethostbyname(hostip.toChars()))==NULL )
#else
            if( (hptr = ::gethostbyname(hostip.toChars()))==NULL )
#endif
            {
                int eno = h_errno;
                switch(eno){
                    case TRY_AGAIN:
                        if(retry>10)
                            return DFW_RETVAL_NEW(DFW_E_AGAIN, eno);
                        usleep(1000);
                        retry++;
                        continue;
                    case HOST_NOT_FOUND:
                        return DFW_RETVAL_NEW(DFW_E_HOST_NOT_FOUND, eno);
//                  case NO_ADDRESS:
//                  case NO_DATA:
                    case NO_RECOVERY:
                        return DFW_RETVAL_NEW(DFW_E_NO_RECOVERY, eno);
                }
                return DFW_RETVAL_NEW(DFW_E_GETHOSTBYNAME, eno);
            }
            break;
        }while(true);

        int addrtype = hptr->h_addrtype;

        pptr = (unsigned char **)hptr->h_addr_list;
        while(*pptr){
            ::memset( addrstr , 0 , 128);
            switch(addrtype){
                case AF_INET:
                    retval = inet_ntop4(*pptr, addrstr, 112);
                    break;
                case AF_INET6:
                    retval = inet_ntop6(*pptr, addrstr, 112);
                    break;
                default :
                    return DFW_RETVAL_NEW(DFW_E_ADDRTYPE, eno);
            }
            if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

            String hostip = addrstr;
            sp<Result> result = new Result(hostip, addrtype);
            if( !m_aResultList.get(result).has() )
                m_aResultList.insert(result);
            pptr++;
        }
        return NULL;
#endif
    }

#define SPRINTF(x) (sprintf x)

    sp<Retval> Hostname::inet_ntop4(const dfw_byte_t* src, char *dst, long size)
    {
        static const char fmt[] = "%u.%u.%u.%u";
        char tmp[sizeof "255.255.255.255"];

        if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) >= size)
            return DFW_RETVAL_NEW(DFW_E_NOSPC,0);

        ::strcpy(dst, tmp);

        return NULL;
    }

    sp<Retval> Hostname::inet_ntop6(const dfw_byte_t* src, char *dst, long size)
    {
        /*
         * Note that int32_t and int16_t need only be "at least" large enough
         * to contain a value of the specified size.  On some systems, like
         * Crays, there is no such thing as an integer variable with 16 bits.
         * Keep this in mind if you think this function should have been coded
         * to use pointer overlays.  All the world's not a VAX.
         */
        sp<Retval> retval;
        char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
        struct { int base, len; } best, cur;
        int i;
#ifndef _WIN32
        u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
#else
        unsigned words[NS_IN6ADDRSZ / NS_INT16SZ];
#endif
        /*
         * Preprocess:
         *      Copy the input (bytewise) array into a wordwise array.
         *      Find the longest run of 0x00's in src[] for :: shorthanding.
         */
        ::memset(words, '\0', sizeof words);
        for (i = 0; i < NS_IN6ADDRSZ; i++)
            words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
        best.base = -1;
        best.len = 0;
        cur.base = -1;
        cur.len = 0;
        for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
            if (words[i] == 0) {
                if (cur.base == -1)
                    cur.base = i, cur.len = 1;
                else
                    cur.len++;
            } else {
                if (cur.base != -1) {
                    if (best.base == -1 || cur.len > best.len)
                        best = cur;
                    cur.base = -1;
                }
            }
        }
        if (cur.base != -1) {
            if (best.base == -1 || cur.len > best.len)
                best = cur;
        }
        if (best.base != -1 && best.len < 2)
            best.base = -1;

        /*
         * Format the result.
         */
        tp = tmp;
        for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
            /* Are we inside the best run of 0x00's? */
            if (best.base != -1 && i >= best.base &&
                i < (best.base + best.len)) {
                if (i == best.base)
                    *tp++ = ':';
                continue;
            }
            /* Are we following an initial run of 0x00s or any real hex? */
            if (i != 0)
                *tp++ = ':';
            /* Is this address an encapsulated IPv4? */
            if (i == 6 && best.base == 0 && (best.len == 6 ||
                                             (best.len == 7 && words[7] != 0x0001) ||
                                             (best.len == 5 && words[5] == 0xffff))) {
                retval = inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp));
                if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
                tp += strlen(tp);
                break;
            }
            tp += SPRINTF((tp, "%x", words[i]));
        }
        /* Was it a trailing run of 0x00's? */
        if (best.base != -1 && (best.base + best.len) ==
            (NS_IN6ADDRSZ / NS_INT16SZ))
            *tp++ = ':';
        *tp++ = '\0';

        /*
         * Check for overflow, copy, and we're done.
         */
        if ((tp - tmp) > size)
            return DFW_RETVAL_NEW(DFW_E_NOSPC,0);

        ::strcpy(dst, tmp);

        return NULL;
    }
#undef SPRINTF

};

