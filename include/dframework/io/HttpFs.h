#ifndef DFRAMEWORK_IO_HTTPFS_H
#define DFRAMEWORK_IO_HTTPFS_H

#include <dframework/io/UriFs.h>
#include <dframework/http/HttpQuery.h>

#ifdef __cplusplus
namespace dframework {

    class HttpFs : public BaseFs
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(HttpFs);

        class Listener : public HttpQuery::OnHttpListener
        {
        private:
            String m_sBuffer;

        public:
            Listener();
            virtual ~Listener();

            virtual sp<Retval> onStatus(sp<HttpConnection>& conn, int status);
            virtual sp<Retval> onResponse(sp<HttpConnection>& conn
                                        , const char* buf,  dfw_size_t size);

            friend class HttpFs;
    };


    private:
        String  m_sUri;
        sp<URI> m_oUri;

        sp<HttpQuery> m_http;
        sp<Listener>  m_http_listener;
        uint64_t      m_size;
        uint64_t      m_offset;
        int           m_step;
        unsigned long m_uTimeout;

        bool          m_bUseHead; // default true.
        String        m_sUserAgent;

    public:
        HttpFs();
        virtual ~HttpFs();

        virtual sp<Retval> ready(sp<URI>& uri);
        virtual void setTimeout(unsigned long value);
        virtual sp<Retval> getattr(const char* path, struct stat* st);
        virtual sp<Retval> readdir(const char* path, sp<DirBox>& db);
        virtual sp<Retval> open(const char* path, int flag, int mode=0);
        virtual sp<Retval> read(const char* path
                              , unsigned* outsize
                              , char* buf, uint32_t size
                              , uint64_t offset);
        virtual sp<Retval> close(const char* path);
        virtual sp<Retval> getContentType(String& sContentType);

        virtual sp<Retval> setAttribute(int type, int value);
        virtual sp<Retval> setAttribute(int type, const char* value);
        virtual sp<Retval> setAttribute(int type
                              , const char* name, const char* value);

    private:
        sp<Retval> getattr_l2(sp<HttpQuery>& http, URI& uri, bool* isclosed);
        sp<Retval> getattr_l(sp<HttpQuery>& http
                           , const char* path, struct stat* st
                           , bool* isclosed);

        sp<Retval> open_l(const char*path);
        sp<Retval> request_l(const char*path
                           , unsigned* outsize
                           , char* buf, uint32_t size
                           , uint64_t offset);

    };

};
#endif

#endif /* DFRAMEWORK_IO_HTTPFS_H */

