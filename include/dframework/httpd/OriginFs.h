#ifndef DFRAMEWORK_HTTPD_ORIGINFS_H
#define DFRAMEWORK_HTTPD_ORIGINFS_H

#include <dframework/base/Retval.h>
#include <dframework/io/UriFs.h>

#ifdef __cplusplus
namespace dframework {

    class OriginFs : public dframework::Object
    {
    private:
        bool       m_bInit;
        sp<Object> m_host;
        sp<UriFs>  m_fs;
        int        m_poff;

    public:
        OriginFs();
        virtual ~OriginFs();

        virtual sp<Retval> ready(sp<Object>& host);

        virtual sp<Retval> getattr_l(const char* path, struct stat* st);
        virtual sp<Retval> getattr(const char* path, struct stat* st);
        virtual sp<Retval> readdir(const char* path, sp<DirBox>& db);
        virtual sp<Retval> open(const char* path, int flag, int mode=0);
        virtual sp<Retval> read(const char* path, unsigned *outsize
                              , char* buf, uint32_t size, uint64_t offset);
        virtual sp<Retval> close(const char* path);
        virtual sp<Retval> getContentType(String& contentType);

    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_ORIGINFS_H */

