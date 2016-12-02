#ifndef DFRAMEWORK_IO_SSH2FS_H
#define DFRAMEWORK_IO_SSH2FS_H

#include <dframework/io/UriFs.h>
#include <dframework/ssh2/SSH2Session.h>

#ifdef __cplusplus
namespace dframework {

    class SSH2Fs : public BaseFs
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(SSH2Fs);

    private:
        sp<SSH2Session> m_session;
        String  m_sUri;
        sp<URI> m_oUri;
        String  m_sPath;
        unsigned long m_uTimeout;

    public:
        SSH2Fs();
        virtual ~SSH2Fs();

        virtual sp<Retval> ready(sp<URI>& uri);
        virtual void setTimeout(unsigned long value);
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

#endif /* DFRAMEWORK_IO_SSH2FS_H */

