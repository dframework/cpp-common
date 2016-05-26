#include <dframework/httpd/HttpdModules.h>

namespace dframework {

    HttpdMod::HttpdMod(const char* name, int flags, void* cb_init)
            : Object()
    {
        m_sName = name;
        m_flags = flags;
        m_handle = NULL;

        m_context = NULL;
        m_cb_init = (sp<Retval> (*)(struct httpd_oper_ex*))cb_init;
        ::memset(&m_oper, 0, sizeof(struct httpd_oper_ex));
    }

    HttpdMod::HttpdMod(const char* name, const char* path)
            : Object()
    {
        m_sName = name;
        m_sPath = path;
        m_flags = 0;
        m_handle = NULL;

        m_context = NULL;
        m_cb_init = NULL;
        ::memset(&m_oper, 0, sizeof(struct httpd_oper_ex));
    }

    HttpdMod::~HttpdMod(){
        close();
    }

    void HttpdMod::close(){
        AutoLock _l(this);
        sp<Retval> retval;
        if( m_flags == FLAG_NO_SOFILE){
            if( DFW_RET(retval, onDeleteMod()) ){
                // FIXME:
            }
        }else if(m_handle){
            if( DFW_RET(retval, onDeleteMod()) ){
                // FIXME:
            }
            ::dlclose(m_handle);
            m_handle = NULL;
        }
    }

    sp<Retval> HttpdMod::loadModule(const char* path){
        AutoLock _l(this);
        sp<Retval> retval;
      
        if( m_flags == FLAG_NO_SOFILE){
            if( !m_cb_init )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR,0
                       , "Not find init callback");
            if( DFW_RET(retval, m_cb_init(&m_oper)) )
                return DFW_RETVAL_D(retval);
        }else if( !m_handle ){
            String sFullpath = String::format("%s/%s"
                                            , path, m_sPath.toChars());
            void* handle = ::dlopen(sFullpath.toChars(), RTLD_NOW);
            if(!handle){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "Not loadModule: %s"
                           , dlerror());
            }
            dlerror(); /* Clear any existing error */
            if( DFW_RET(retval, loadFunctions(handle)) ){
                ::dlclose(handle);
                return DFW_RETVAL_D(retval);
            }
            if( DFW_RET(retval, onCreateMod()) ){
                ::dlclose(handle);
                return DFW_RETVAL_D(retval);
            }
            m_handle = handle;
        }
        return NULL;
    }

    sp<Retval> HttpdMod::loadFunctions(void* handle){
        AutoLock _l(this);
        sp<Retval> retval;
        String sInit = String::format("HTTPD_MOD_%s_init_ex"
                                    , m_sName.toChars());
        m_cb_init = (sp<Retval> (*)(struct httpd_oper_ex*))
                        ::dlsym(handle, sInit.toChars());
        if( !m_cb_init )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR,0
                       , "Not find symbol %s: %s"
                       , sInit.toChars(), dlerror());
        if( DFW_RET(retval, m_cb_init(&m_oper)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> HttpdMod::onCreateMod(){
        AutoLock _l(this);
        sp<Retval> retval;
        if( m_oper.mod_create
            && DFW_RET(retval, m_oper.mod_create(&m_context, 0,0,0))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpdMod::onDeleteMod(){
        AutoLock _l(this);
        sp<Retval> retval;
        if( m_oper.mod_delete
            && DFW_RET(retval, m_oper.mod_delete(m_context))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpdMod::request(sp<HttpdClient>& client
                                   , dfw_httpstatus_t* out)
    {
        sp<Retval> retval;
        if( m_oper.request && DFW_RET(retval
                , m_oper.request(client, m_context, out))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpdMod::open(sp<HttpdClient>& client, const char* path)
    {
        sp<Retval> retval;
        if( m_oper.open && DFW_RET(retval
                , m_oper.open(client, m_context, path))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }
 
    sp<Retval> HttpdMod::read(sp<HttpdClient>& client
                         , char* buf, uint32_t size
                         , uint64_t offset)
    {
        sp<Retval> retval;
        if( m_oper.read && DFW_RET(retval
                , m_oper.read(client, m_context, buf, size, offset))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpdMod::close(sp<HttpdClient>& client)
    {
        sp<Retval> retval;
        if( m_oper.close && DFW_RET(retval
                , m_oper.close(client, m_context))){
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    // --------------------------------------------------------------

    HttpdModules::HttpdModules(){
    }

    HttpdModules::~HttpdModules(){
    }

#if 0
    sp<Module> HttpdModules::createModule(const char* pName){
        return new HttpdMod(pName);
    }
#endif

};

