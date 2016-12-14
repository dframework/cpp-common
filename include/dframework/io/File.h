#ifndef DFRAMEWORK_IO_FILE_H
#define DFRAMEWORK_IO_FILE_H

#include <dframework/base/Retval.h>
#include <fcntl.h>


#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

extern ssize_t pread (int, void *, size_t, off_t);

#ifdef __cplusplus
};
#endif
#endif



#ifdef __cplusplus
namespace dframework {

    class File : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(File);

    private:
        int      m_fd;
        uint64_t m_offset;
        unsigned long m_uTimeout;
        String   m_sPath;

    public:
        File();
        virtual ~File();

        void close();

        void setTimeout(unsigned long value);

        sp<Retval> open(const char* path, int flag=O_RDONLY);
        inline sp<Retval> open(const String& path, int flag=O_RDONLY){
            return open(path.toChars(), flag);
        }
        sp<Retval> open(const char* path, int flag, int mode);
        inline sp<Retval> open(const String& path, int flag, int mode){
            return open(path.toChars(), flag, mode);
        }

        inline sp<Retval> read(unsigned *out_size
                             , char* buf, uint32_t size){
            return File::read(m_fd, out_size, buf, size);
        }
        sp<Retval> read(unsigned *out_size, char* buf, unsigned size
                      , uint64_t offset);

        inline sp<Retval> write(const char* buf, uint32_t size){
            return File::write(m_fd, buf, size);
        }
        sp<Retval> write(const char* buf, unsigned size, uint64_t offset);

        sp<Retval> seek(uint64_t offset);
        sp<Retval> lastSeek(uint64_t offset);

    private:
        void close_l();

    public:
        static bool isFile(const char* path);
        static inline bool isFile(const String& path) {
            return isFile(path.toChars());
        }

        static bool isAccess(const char* path);
        static inline bool isAccess(const String& path) {
            return isAccess(path.toChars());
        }

        static bool isDirectory(const char* path);
        static inline bool isDirectory(const String& path) {
            return isDirectory(path.toChars());
        }

        // ----------------------------------------------------------

        static sp<Retval> makeDirectory(const char* path, int mode);
        static inline sp<Retval> makeDirectory(const String& path, int mode){
            return makeDirectory(path.toChars(), mode);
        }
        static inline sp<Retval> makeDir(const char* path){
            return makeDirectory(path, 0755);
        }
        static inline sp<Retval> makeDir(const String& path){
            return makeDirectory(path.toChars(), 0755);
        }

        // ----------------------------------------------------------

        static sp<Retval> makeDir(const char* basedir
                                       , const char* path, int mode);
        static inline sp<Retval> makeDir(const String& basedir
                                       , const char* path, int mode){
            return makeDir(basedir.toChars(), path, mode);
        }
        static inline sp<Retval> makeDir(const String& basedir
                                       , const String& path, int mode){
            return makeDir(basedir.toChars(), path.toChars(), mode);
        }
        static inline sp<Retval> makeDir(const char* basedir
                                        ,const char* path){
            return makeDir(basedir, path, 0755);
        }
        static inline sp<Retval> makeDir(const String& basedir
                                        ,const char* path){
            return makeDir(basedir.toChars(), path, 0755);
        }
        static inline sp<Retval> makeDir(const String& basedir
                                       , const String& path){
            return makeDir(basedir.toChars(), path.toChars(), 0755);
        }

        // ----------------------------------------------------------

        static sp<Retval> open(int* out_fd, const char* path, int flag);
        static inline sp<Retval> open(int* out_fd,const String& path,int flag){
            return open(out_fd, path.toChars(), flag);
        }
        static sp<Retval> open(int* out_fd, const char* path, int flag
                             , int mode);
        static inline sp<Retval> open(int* out_fd
                                    , const String& path, int flag, int mode){
            return open(out_fd, path.toChars(), flag, mode);
        }

        // ----------------------------------------------------------

        static sp<Retval> truncate(const char* path, uint64_t size);
        static inline sp<Retval> truncate(const String& path, uint64_t size){
            return truncate(path.toChars(), size);
        }
        static sp<Retval> truncate(int fd, uint64_t size
                                 , const char* path=NULL);
        static inline sp<Retval> truncate(int fd, uint64_t size
                                 , const String& path){
            return truncate(fd, size, path.toChars());
        }

        // ----------------------------------------------------------

        static sp<Retval> read(int fd, const char* path
                             , unsigned *out_size
                             , char* buf, uint32_t size, uint64_t offset);
        static inline sp<Retval> read(int fd, String& path
                             , unsigned *out_size
                             , char* buf, uint32_t size, uint64_t offset){
            return read(fd, path.toChars(), out_size, buf, size, offset);
        }

        static sp<Retval> read(int fd, unsigned *out_size
                             , char* buf, uint32_t size);
        static sp<Retval> read(int fd, unsigned *out_size
                             , char* buf, uint32_t size, uint64_t offset);

        // ----------------------------------------------------------

        static sp<Retval> contents(String& contents, const char* path);

        // ----------------------------------------------------------

        static sp<Retval> write(int fd, const char* contents, unsigned size);
        static inline sp<Retval> write(int fd, const String& contents){
            return write(fd, contents.toChars(), (unsigned)contents.length());
        }
        static inline sp<Retval> write(int fd, const String& contents
                                     , unsigned size){
            return write(fd, contents.toChars(), size);
        }
        static sp<Retval> write(int fd, const char* contents, unsigned size
                              , uint64_t offset);
        static inline sp<Retval> write(int fd, const String& contents
                              , uint64_t offset){
            return write(fd, contents.toChars(), (unsigned)contents.length()
                       , offset);
        }
        static inline sp<Retval> write(int fd, const String& contents
                                     , unsigned size, uint64_t offset){
            return write(fd, contents.toChars(), size, offset);
        }

        // ----------------------------------------------------------

        static sp<Retval> save(const char* buf, unsigned size, const char* path);
        inline static sp<Retval> save(const char* buf, unsigned size
                                    , const String& sPath){
            return save(buf, size, sPath.toChars());
        }
        inline static sp<Retval> save(const String& buf, const char* path){
            return save(buf.toChars(), (unsigned)buf.length(), path);
        }

        // ----------------------------------------------------------

        static sp<Retval> seek(int fd, uint64_t offset);
        static sp<Retval> lastSeek(int fd, uint64_t offset);

        // ----------------------------------------------------------

        static sp<Retval> remove(const char* path);
        static inline sp<Retval> remove(const String& path){
            return remove(path.toChars());
        }

        static sp<Retval> removePath(const char* base, const char* path);
        static inline sp<Retval> removePath(const char* base
                                          , const String& path){
            return removePath(base, path.toChars());
        }
        static inline sp<Retval> removePath(const String& base
                                          , const char* path){
            return removePath(base.toChars(), path);
        }
        static inline sp<Retval> removePath(const String& base
                                          , const String& path){
            return removePath(base.toChars(), path.toChars());
        }

        static sp<Retval> removeAll(const char* path);
        static inline sp<Retval> removeAll(const String& path){
            return removeAll(path.toChars());
        }

        // ----------------------------------------------------------

        static sp<Retval> rename(const char* oldname, const char* newname);
        static inline sp<Retval> rename(const String& old, const String& newn){
            return rename(old.toChars(), newn.toChars());
        }

        // ----------------------------------------------------------

        static sp<Retval> mtime(const char* path, uint64_t time);
        static inline sp<Retval> mtime(const String& path, uint64_t time){
            return mtime(path.toChars(), time);
        }

        static sp<Retval> setNonBlockSocket(int fd, bool is=true);
        static sp<Retval> isReadable(int fd, unsigned long timeout);

    };

};
#endif

#endif /* DFRAMEWORK_IO_FILE_H */

