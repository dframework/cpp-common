#ifndef DFRAMEWORK_IO_STAT_H
#define DFRAMEWORK_IO_STAT_H
#define DFRAMEWORK_IO_STAT_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/net/URI.h>
#include <sys/stat.h>

#ifdef __ANDROID__
# define S_IREAD        S_IRUSR
# define S_IWRITE       S_IWUSR
# define S_IEXEC        S_IXUSR
#endif

#define DFW_S_MODE(v,m) (((unsigned)((v) & 0170000)) == ((unsigned)m))

#ifdef __cplusplus
namespace dframework {

    class Stat : public Object
    {
    public:
#if defined(DFW_OS_WINDOWS)
        static const int D_IFMT   = _S_IFMT;
        static const int D_IFDIR  = _S_IFDIR;
        static const int D_IFCHR  = _S_IFCHR;
        static const int D_IFIFO  = _S_IFIFO;
        static const int D_IFREG  = _S_IFREG;
        static const int D_IREAD  = _S_IREAD;
        static const int D_IWRITE = _S_IWRITE;
        static const int D_IEXEC  = _S_IEXEC;
        static const int D_IFBLK  = 0060000;
        static const int D_IFLNK  = 0120000;
        static const int D_IFSOCK = 0140000;
#else
        static const int D_IFMT   = S_IFMT;
        static const int D_IFDIR  = S_IFDIR;
        static const int D_IFCHR  = S_IFCHR;
        static const int D_IFIFO  = S_IFIFO;
        static const int D_IFREG  = S_IFREG;
        static const int D_IREAD  = S_IREAD;
        static const int D_IWRITE = S_IWRITE;
        static const int D_IEXEC  = S_IEXEC;
        static const int D_IFBLK  = S_IFBLK;
        static const int D_IFLNK  = S_IFLNK;
        static const int D_IFSOCK = S_IFSOCK;
#endif

    private:
        sp<Retval> m_LastRetval;

    public:
        String      m_name;
        URI         m_Uri;
        struct stat m_stat;

    private:
        sp<Retval> stat_real();
        sp<Retval> stat_file_real();

    public:
        Stat();
        Stat(const char* uri);
        Stat(const String& uri);
        Stat(const URI& uri);
        virtual ~Stat();

        void clear();

        sp<Retval> stat(const char* uri);
        sp<Retval> stat(const String& uri);
        sp<Retval> stat(const URI& uri);

        inline void setName(const char* name) { m_name = name; }
        inline void setName(String& name) { m_name = name; }
        inline sp<Retval> lastRetval() const { return m_LastRetval; }
        inline const URI& getUri() const { return m_Uri; }

        inline bool isBlk()  const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFBLK); }
        inline bool isLnk()  const {
            return DFW_S_MODE(m_stat.st_mode, D_IFLNK); }
        inline bool isSock() const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFSOCK); }
        inline bool isMt()   const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFMT); }
        inline bool isDir()  const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFDIR); }
        inline bool isChr()  const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFCHR); }
        inline bool isFifo() const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFIFO); }
        inline bool isReg()  const { 
            return DFW_S_MODE(m_stat.st_mode, D_IFREG); }
        inline bool isRead() const { 
            return DFW_S_MODE(m_stat.st_mode, D_IREAD); }
        inline bool isWrite()const { 
            return DFW_S_MODE(m_stat.st_mode, D_IWRITE); }
        inline bool isExec() const { 
            return DFW_S_MODE(m_stat.st_mode, D_IEXEC); }

        DFW_OPERATOR_EXP_DECLARATION(Stat);
    };

};
#endif

#endif /* DFRAMEWORK_IO_STAT_H */

