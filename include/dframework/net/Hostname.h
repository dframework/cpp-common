#ifndef DFRAMEWORK_NET_HOSTNAME_H
#define DFRAMEWORK_NET_HOSTNAME_H

#include <dframework/base/Retval.h>
#include <dframework/net/URI.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {

    class Hostname : public Object 
    {
    public:
        class Result : public Object 
        {
        public:
            String m_sIp;
            int    m_iAddrType;
            void*  m_SockAddr; // struct sockaddr_in(6)
            int    m_iSockAddrLen;

        public:
            Result(const String& ip, int type, void* ai_addr, int ai_addr_len);
            virtual ~Result();

            DFW_OPERATOR_EX_DECLARATION(Result, m_sIp);
        };

        // ----------------------------------------------------------

        class CacheData: public Object 
        {
        public:
            dfw_ulong_t m_uCacheTime;
            String m_sHost;
            Array<Result> m_aResultList;

        public:
            CacheData(const String& host);
            CacheData(const String& host, const Hostname& hn);
            inline virtual ~CacheData() {}

            DFW_OPERATOR_EX_DECLARATION(CacheData, m_sHost);
        };

        // ----------------------------------------------------------

        class Cache : public Object 
        {
        private:
            ArraySorted<CacheData> m_aCacheList;

        public:
            inline Cache() {}
            inline virtual ~Cache() {}

            sp<Retval> append(const String& host, Hostname& hn);
            sp<Retval> search(const String& host, Hostname& hn);

            int size();
            sp<CacheData> get(int position);
        };

    private:
        URI           m_Uri;
        dfw_ulong_t   m_uCacheTime;
        Array<Result> m_aResultList;
        Array<Result> m_aCacheList;
        sp<Retval>    m_LastRetval;

    public:
        static Cache m_Cache;
        static int CACHE_TIME_SEC;

    public:
        Hostname();
        Hostname(String& out, const char* uri, bool bCache=true);
        Hostname(String& out, const String& uri, bool bCache=true);
        Hostname(String& out, const URI& uri, bool bCache=true);
        virtual ~Hostname();

        sp<Retval> get(const char* uri, bool bCache=true);
        sp<Retval> get(const String& uri, bool bCache=true);
        sp<Retval> get(const URI& uri, bool bCache=true);

        sp<Retval> get(String& out, const char* uri, bool bCache=true);
        sp<Retval> get(String& out, const String& uri, bool bCache=true);
        sp<Retval> get(String& out, const URI& uri, bool bCache=true);

        String     getIp(const char* uri, bool bCache=true);
        String     getIp(const String& uri, bool bCache=true);
        String     getIp(const URI& uri, bool bCache=true);

        void       clear();
        int        size() const;
        sp<Result> getResult(int position) const;
        String     ip(int position) const;

        sp<Retval> addCache(sp<Hostname::Result> result);
        sp<Retval> setCacheData(sp<Hostname::CacheData> cd);

    private:
        sp<Retval> parse_start(bool bCache);
        sp<Retval> parse_start_l(bool bCache);
        //sp<Retval> parse_addrtype(const String& hostip);
        sp<Retval> parse_hostbyname(const String& hostip);
        sp<Retval> inet_ntop4(const dfw_byte_t* src, char *dst, long size);
        sp<Retval> inet_ntop6(const dfw_byte_t* src, char *dst, long size);

    };

};
#endif

#endif /* DFRAMEWORK_NET_HOSTNAME_H */

