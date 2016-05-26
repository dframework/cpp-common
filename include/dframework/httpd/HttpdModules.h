#ifndef DFRAMEWORK_HTTPD_HTTPDMODULES_H
#define DFRAMEWORK_HTTPD_HTTPDMODULES_H

#include <dframework/base/Retval.h>
#include <dframework/http/HttpStatus.h>
#include <dframework/net/ServerAccept.h>
#include <dframework/httpd/HttpdClient.h>

#ifdef _WIN32
#include <dframework/mingw/dlfcn.h>
#else
#include <dlfcn.h>
#endif


enum zonehttpd_module_t {
    ZONEHTTPD_MOD_OK    = 9000,
    ZONEHTTPD_MOD_AGAIN = 9001,
    ZONEHTTPD_MOD_ERROR = 9002,
};

#ifdef __cplusplus
namespace dframework {

    struct httpd_oper_ex {
    sp<Retval> (*mod_create)  (void** ctx, int, int, int);
    sp<Retval> (*mod_delete)  (void* ctx);

    sp<Retval> (*accept)  (sp<ClientSocket>& sock, void* ctx);
    sp<Retval> (*request) (sp<HttpdClient>& client, void* ctx
                         , dfw_httpstatus_t* out);
    sp<Retval> (*open)  (sp<HttpdClient>& client, void* ctx
                         , const char* path);
    sp<Retval> (*read)  (sp<HttpdClient>& client, void* ctx
                         , char* buf, uint32_t size
                         , uint64_t offset);
    sp<Retval> (*close)   (sp<HttpdClient>& client, void* ctx);
    };

    class HttpdMod : public Object
    {
    public:
        static const int FLAG_NO_SOFILE = 1;

    private:
        String m_sName;
        String m_sPath;
        int   m_flags;
        void* m_handle;

        void* m_context;
        sp<Retval> (*m_cb_init) (struct httpd_oper_ex* out);
        struct httpd_oper_ex m_oper;

    public:
        // flags is FLAG_NO_SOFILE
        HttpdMod(const char* name, int flags, void* cb_init);
        HttpdMod(const char* name, const char* path);
        virtual ~HttpdMod();
        void close();

        sp<Retval> loadModule(const char* path);
        inline sp<Retval> loadModule(String& sPath){
            return loadModule(sPath.toChars());
        }

        sp<Retval> loadFunctions(void* handle);
        sp<Retval> onCreateMod();
        sp<Retval> onDeleteMod();

        sp<Retval> request(sp<HttpdClient>& client, dfw_httpstatus_t* out);
        sp<Retval> open(sp<HttpdClient>& client, const char* path);
        sp<Retval> read(sp<HttpdClient>& client, char* buf, uint32_t size
                         , uint64_t offset);
        sp<Retval> close(sp<HttpdClient>& client);

        DFW_OPERATOR_EX_DECLARATION(HttpdMod, m_sName);

    };

    class HttpdModules : public Object
    {
    public:
        HttpdModules();
        virtual ~HttpdModules();

        //virtual sp<Module> createModule(const char* pName);

    };

};
#endif

#endif /* DFRAMEWORK_HTTPD_HTTPDMODULES_H */

