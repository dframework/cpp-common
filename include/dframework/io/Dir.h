#ifndef DFRAMEWORK_IO_DIR_H
#define DFRAMEWORK_IO_DIR_H
#define DFRAMEWORK_IO_DIR_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/net/URI.h>

typedef dfw_point_t dfw_DIR_t;
typedef dfw_point_t dfw_dirent_t;


#ifdef __cplusplus
namespace dframework {

    class Dir : public Object
    {
    private:
        sp<Retval>   m_LastRetval;
        dfw_DIR_t    m_hSource;
        dfw_dirent_t m_hEnt;
        URI          m_Uri;

    public:
        Dir();
        Dir(const char* dir);
        Dir(const String& dir);
        Dir(const URI& dir);
        virtual ~Dir();

        sp<Retval> open(const char *dir);
        sp<Retval> open(const String& dir);
        sp<Retval> open(const URI& dir);

        void close();

        sp<Retval> read(DFW_OUT String& name);

        inline sp<Retval> lastRetval() const { return m_LastRetval; }

        DFW_OPERATOR_EX_DECLARATION(Dir, m_Uri);

    private:
        void ___init();
        sp<Retval> open();
    };

};
#endif

#endif /* DFRAMEWORK_IO_DIR_H */


