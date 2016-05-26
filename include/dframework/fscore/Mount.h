#ifndef DFRAMEWORK_FSCORE_MOUNT_H
#define DFRAMEWORK_FSCORE_MOUNT_H
#define DFRAMEWORK_FSCORE_MOUNT_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/io/Stat.h>

#ifndef _WIN32

typedef struct dfw_mount_opts dfw_mount_opts;

#ifdef __cplusplus
namespace dframework {

    class Mount : public Object
    {
    private:
        int     m_iHandle;
        bool    m_bMounted;
        bool    m_bExited;

        String  m_sDeviceName;
        String  m_sMountPoint;

        bool    m_bAllowOther;
        bool    m_bAllowRoot;
        bool    m_bNonEmpty;
        bool    m_bAutoUnmount; // FIXME: Not used.
        bool    m_bBlkDev;
        int     m_iFlags;
        String  m_sFsname;
        String  m_sSubtype;
        String  m_sSubtypeOpt;
        String  m_sMTabOpts;
        String  m_sFUserMountOpts;
        String  m_sKernelOpts;

    protected:
        Object m_read_lock;
        Object m_send_lock;

        virtual void init();
        virtual void addDefaultFlags();
        virtual sp<Retval> mount_real(const Stat& st);
        virtual sp<Retval> mount_open();
        virtual sp<Retval> checkEmptyMountPoint(const Stat& st);
        virtual int check_fuseblk(void);

    public:
        Mount();
        virtual ~Mount();

        virtual sp<Retval> mount(const char *devicename
                               , const char* mountpoint);
        virtual sp<Retval> mount(const String& devicename
                               , const String& mountpoint);
        virtual sp<Retval> mount();
        virtual sp<Retval> umount(void);
        virtual sp<Retval> umount(const char* mountpoint);
        virtual sp<Retval> umount(const String& mountpoint);
        virtual sp<Retval> read(size_t* outsize, char *buf, size_t size);

        inline int getHandle() { return m_iHandle; }
        inline bool isMounted() { return m_bMounted; }

        inline void setExited() { m_bExited = true; }
        inline bool isExited() { return m_bExited; }

        inline void setDeviceName(const char* devicename){
            m_sDeviceName = devicename;
        }
        inline void setDeviceName(const String& devicename){
            m_sDeviceName = devicename;
        }
        inline String& getDeviceName() { return m_sDeviceName; }

        inline void setMountPoint(const char* mountpoint) {
            m_sMountPoint = mountpoint; 
        }
        inline void setMountPoint(const String& mountpoint) {
            m_sMountPoint = mountpoint; 
        }
        inline String& getMountPoint() { return m_sMountPoint; }

        inline void setAllowOther(bool allow) { m_bAllowOther = allow; }
        inline bool getAllowOther() { return m_bAllowOther; }

        inline void setAllowRoot(bool allow) { m_bAllowRoot = allow; }
        inline bool getAllowRoot() { return m_bAllowRoot; }

        inline void setNonEmpty(bool nonempty) { m_bNonEmpty = nonempty; }
        inline bool getNonEmpty() { return m_bNonEmpty; }

        inline void setAutoUnmount(bool allow) { m_bAutoUnmount = allow; }
        inline bool getAutoUnmount() { return m_bAutoUnmount; }

        inline void setBlkDev(bool blkdev) { m_bBlkDev = blkdev; }
        inline bool getBlkDev() { return m_bBlkDev; }

        inline void setFlags(int flags) { m_iFlags = flags; }
        inline void addFlag(int flag) { m_iFlags |= flag; }
        inline void removeFlag(int flag) { m_iFlags &= (~flag); }
        inline int  getFlags() { return m_iFlags; }

        inline void setFsname(const char* fsname) { m_sFsname = fsname; }
        inline void setFsname(const String& fsname) { m_sFsname = fsname; }
        inline String& getFsname() { return m_sFsname; }

        inline void setSubtype(const char* subtype) { m_sSubtype = subtype; }
        inline void setSubtype(const String& subtype) { m_sSubtype=subtype; }
        inline String& getSubtype() { return m_sSubtype; }

        inline void setSubtypeOpt(const char* opt) { m_sSubtypeOpt = opt; }
        inline void setSubtypeOpt(const String& opt) { m_sSubtypeOpt =opt; }
        inline String& getSubtypeOpt() { return m_sSubtypeOpt; }

        inline void setMTabOpts(const char* opts) { m_sMTabOpts = opts; }
        inline void setMTabOpts(const String& opts) { m_sMTabOpts =opts; }
        inline String& getMTabOpts() { return m_sMTabOpts; }

        inline void setFUserMountOpts(const char* opts) { 
            m_sFUserMountOpts = opts; 
        }
        inline void setFUserMountOpts(const String& opts) { 
            m_sFUserMountOpts =opts; 
        }
        inline String& getFUserMountOpts() { return m_sFUserMountOpts; }

        inline void setKernelOpts(const char* opts) { m_sKernelOpts = opts; }
        inline void setKernelOpts(const String& opts) { m_sKernelOpts =opts; }
        inline String& getKernelOpts() { return m_sKernelOpts; }

    };

}; // end namespace dframework
#endif // end if cplusplus

#endif

#endif /* DFRAMEWORK_FSCORE_MOUNT_H */

