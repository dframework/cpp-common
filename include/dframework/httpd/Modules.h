#ifndef DFRAMEWORK_UTIL_MODULES_H
#define DFRAMEWORK_UTIL_MODULES_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#ifdef _WIN32
#include <dframework/mingw/dlfcn.h>
#else
#include <dlfcn.h>
#endif


#ifdef __cplusplus
namespace dframework {

    class Module : public Object
    {
    protected:
        String m_sAliasName;
        String m_sName;
        ArraySorted<String> m_aSites;

        void* m_handle;

    public:
        Module(const char* name);
        virtual ~Module();

        virtual sp<Retval> loadFunctions(void* handle)=0;
        virtual sp<Retval> onCreateMod()=0;
        virtual sp<Retval> onDeleteMod()=0;

        virtual sp<Retval> insertSitename(const char* insite);
        virtual bool hasSite(const char* insite);
        virtual void close();

        virtual sp<Retval> loadModule(const char* path);
        virtual inline sp<Retval> loadModule(const String& path){
            return loadModule(path.toChars());
        }

        DFW_OPERATOR_EX_DECLARATION(Module, m_sName);

    };

    class Modules : public Object
    {
    protected:
        String m_sPath;
        ArraySorted<Module> m_aMods;

    public:
        Modules();
        virtual ~Modules();

        virtual sp<Module> createModule(const char* pName)=0;

        inline void setPath(String path) { m_sPath = path; }
        inline String& getPath() { return m_sPath; }

        int sizeModule();
        sp<Module> getModule(int position);
        sp<Module> getModule(const char* name);
        sp<Retval> insertModule(const char* site, const char* name);

    };

};
#endif

#endif /* DFRAMEWORK_UTIL_MODULES_H */

