//
//  Log.h
//  dframework.lib.headers
//
//  Created by Choi HyunKyu on 2015. 6. 8..
//
//

#ifndef dframework_lib_headers_Log_h
#define dframework_lib_headers_Log_h

#include <dframework/Type.h>
#include <dframework/Header.h>
#include <dframework/lang/String.h>
#include <dframework/net/URI.h>
#include <dframework/net/ISock.h>
#include <dframework/core/ref.h>



enum dfw_logtype_t {
    DFW_LOGTYPE_LOG = 0,
    DFW_LOGTYPE_INFO = 1,
    DFW_LOGTYPE_WARRING = 2,
    DFW_LOGTYPE_ERROR = 3,
};


#ifdef __cplusplus
namespace dframework {
    
    class Logger : public ref, public Able, public ISock
    {
    public:
        Logger(ISock *source);
        virtual ~Logger();
                
    private:
        ISock *mSource;

    protected:
        void onOpen();
        void onClose();
        void onError(int errnum);
        
    public:
        virtual dfw_status_t open();
        virtual dfw_status_t close();
        virtual dfw_status_t send(const dfw_byte_t *data, size_t size, dfw_ulong_t timeout_msec=0);
        virtual dfw_status_t recv(dfw_byte_t* o_data, size_t *o_size, size_t size, dfw_ulong_t timeout_msec=0);
        
        virtual dfw_status_t write(const char *data);
        virtual dfw_status_t write(const String &data);
        virtual dfw_status_t write(dfw_logtype_t type, const char *data);
        virtual dfw_status_t log(dfw_logtype_t type, const char *fmt, ...);
      
        static String makeLog(dfw_logtype_t type, const char *data);
    };

};
#endif


#endif
