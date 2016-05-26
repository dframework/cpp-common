#ifndef DFRAMEWORK_FSCORE_FUSEMOUNT_H
#define DFRAMEWORK_FSCORE_FUSEMOUNT_H

#include <dframework/base/Retval.h>
#include <dframework/fscore/Mount.h>
#include <dframework/fscore/Fuse_d.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/uio.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseMount : public Mount
    {
    public:
        static const char*  DEVICE_NAME;
        static const size_t MIN_BUFFER_SIZE = 0x21000;
        static const size_t S_PAGE_SIZE;
        static const size_t BUFFER_SIZE;
        static Object       SAFE_LOCK;

    private:
        bool       m_bGetInit;
        uint64_t   m_currentUnique;
        int        m_lastOpcode;

    public:
        struct conn_info m_conn;

    private:
        size_t iov_length(const struct iovec *iov, size_t count);
        virtual sp<Retval> send(const dfw_point_t, size_t);
        void init();

    protected:

    public:
        FuseMount();
        FuseMount(const char* mountpoint);
        virtual ~FuseMount();

        void setUnique(uint64_t unique, int opcode);
        uint64_t getUnique();
        int getLastOpcode();

        inline void setInit(bool isget){ m_bGetInit = isget; }
        inline bool isInit() { return m_bGetInit; }

        virtual sp<Retval> mount(const char* mountpoint);
        virtual sp<Retval> mount(const String& mountpoint);
        virtual sp<Retval> mount() {
            return Mount::mount();
        }

        virtual sp<Retval> send(uint64_t unique, int error
                              , const dfw_point_t, size_t size);
        virtual sp<Retval> read(size_t *out, char* buf, size_t size);

        inline unsigned proto_major() { return m_conn.proto_major; }
        inline unsigned proto_minor() { return m_conn.proto_minor; }
        inline unsigned async_read() { return m_conn.async_read; }
        inline unsigned max_write() { return m_conn.max_write; }
        inline unsigned max_readahead() { return m_conn.max_readahead; }
        inline unsigned capable() { return m_conn.capable; }
        inline unsigned want() { return m_conn.want; }
        inline unsigned max_background() { return m_conn.max_background; }
        inline unsigned congestion_threshold() { 
            return m_conn.congestion_threshold; 
        }
    };

}; // end namespace dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEMOUNT_H */

