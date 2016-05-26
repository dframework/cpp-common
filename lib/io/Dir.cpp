#include <dframework/io/Dir.h>
#include <dirent.h>

namespace dframework {

    Dir::Dir(){
        ___init();
    }

    Dir::Dir(const char* dir)
            : Object()
    {
        ___init();
        open(dir);
    }

    Dir::Dir(const String& dir)
            : Object()
    {
        ___init();
        open(dir);
    }

    Dir::Dir(const URI& dir)
            : Object()
    {
        ___init();
        open(dir);
    }

    Dir::~Dir(){
        close();
    }

    void Dir::___init(){
        m_hSource = NULL;
        m_hEnt = NULL;
    }

    void Dir::close(){
        if(m_hSource){
            ::closedir((DIR*)m_hSource);
            m_hSource = NULL;
        }
        m_hEnt = NULL;
    }

    sp<Retval> Dir::open(const char* dir){
        m_Uri = dir;
        return (m_LastRetval = open());
    }

    sp<Retval> Dir::open(const String& dir){
        m_Uri = dir;
        return (m_LastRetval = open());
    }

    sp<Retval> Dir::open(const URI& dir){
        m_Uri = dir;
        return (m_LastRetval = open());
    }

    sp<Retval> Dir::open(){
        String path = m_Uri.getPath();
        if(path.empty())
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, 0, "path=null");

#if defined(DFW_OS_WINDOWS)
        //path.append("/*", 2); // FIXME:
#endif
        if( NULL==(m_hSource = ::opendir(path.toChars())) ){
            int eno = errno;
            switch(eno){
            case EACCES:
                return DFW_RETVAL_NEW(DFW_E_ACCES,eno);
            case EBADF:
                return DFW_RETVAL_NEW(DFW_E_BADF,eno);
            case EMFILE:
                return DFW_RETVAL_NEW(DFW_E_MFILE,eno);
            case ENOENT:
                return DFW_RETVAL_NEW(DFW_E_NOENT,eno);
            case ENOMEM:
                return DFW_RETVAL_NEW(DFW_E_NOMEM,eno);
            case ENOTDIR:
                return DFW_RETVAL_NEW(DFW_E_NOTDIR,eno);
            }
            return DFW_RETVAL_NEW(DFW_E_OPENDIR,eno);
        }

        return NULL;
    }

#define DFW_DIR_DIRENT(x) ((struct dirent*)x)

    sp<Retval> Dir::read(DFW_OUT String& name){
        if( !m_hSource ) {
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);
        }
        if( NULL==(m_hEnt = ::readdir((DIR*)m_hSource)) ){
            return DFW_RETVAL_NEW(DFW_T_COMPLETE, 0);
        }
        name = DFW_DIR_DIRENT(m_hEnt)->d_name;
        return NULL;
    }

};

