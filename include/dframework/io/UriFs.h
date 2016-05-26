/*
 * UriFs
 * Supported scheme
 *     - file
 *     - http
 *     - https(x)
 *     - webdav
 *     - webdavs(x)
 */

#ifndef DFRAMEWORK_IO_URIFS_H
#define DFRAMEWORK_IO_URIFS_H

#include <dframework/base/Retval.h>
#include <dframework/net/URI.h>
#include <dframework/io/DirBox.h>

#ifdef __cplusplus
namespace dframework {

    class BaseFs : public Object
    {
    public:
        static const int ATTR_NOHEAD = 1;
        static const int ATTR_HEADER = 2;

    public:

        inline BaseFs() {}
        inline virtual ~BaseFs() {}

        virtual sp<Retval> ready(sp<URI>& uri) = 0;
        virtual sp<Retval> getattr(const char* path, struct stat* st) = 0;
        virtual sp<Retval> readdir(const char* path, sp<DirBox>& db) = 0;
        virtual sp<Retval> open(const char* path, int flag, int mode=0) = 0;
        virtual sp<Retval> read(const char* path
                              , unsigned *outsize
                              , char* buf, uint32_t size
                              , uint64_t offset) = 0;
        virtual sp<Retval> close(const char* path) = 0;
        virtual sp<Retval> getContentType(String& sContentType)=0;

        virtual sp<Retval> setAttribute(int type, int value) = 0;
        virtual sp<Retval> setAttribute(int type, const char* value) = 0;
        virtual sp<Retval> setAttribute(int type
                              , const char* name, const char* value) = 0;
    };

    class UriFs : public BaseFs
    {
    private:
        String     m_sUri;
        sp<URI>    m_oUri;
        sp<BaseFs> m_fs;

    public:
        UriFs();
        virtual ~UriFs();

        virtual sp<Retval> ready(const char* uri);
        virtual sp<Retval> onFindBaseFs(sp<URI>& uri, sp<BaseFs>& fs);

        virtual sp<Retval> ready(sp<URI>& uri);
        virtual sp<Retval> getattr(const char* path, struct stat* st);
        virtual sp<Retval> readdir(const char* path, sp<DirBox>& db);
        virtual sp<Retval> open(const char* path, int flag, int mode=0);
        virtual sp<Retval> read(const char* path
                              , unsigned *outsize
                              , char* buf, uint32_t size
                              , uint64_t offset);
        virtual sp<Retval> close(const char* path);
        virtual sp<Retval> getContentType(String& sContentType);

        virtual sp<Retval> setAttribute(int type, int value);
        virtual sp<Retval> setAttribute(int type, const char* value);
        virtual sp<Retval> setAttribute(int type
                              , const char* name, const char* value);

    };

};
#endif

#endif /* DFRAMEWORK_IO_URIFS_H */

