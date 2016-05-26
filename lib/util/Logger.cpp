//
//  Log.cpp
//  dframework.lib
//
//  Created by Choi HyunKyu on 2015. 6. 8..
//  Copyright (c) 2015년 Choi HyunKyu. All rights reserved.
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <dframework/util/Logger.h>
#include <dframework/io/File.h>
#include <dframework/util/Time.h>

namespace dframework {
    
    Logger::Logger(ISock *source) :
        mSource(source)
    {
    }
    
    Logger::~Logger(){
        {
            Mutex::AutoLock _l(this);
            DFW_DELETE(mSource);
        }
    }
    
    dfw_status_t Logger::open(){
        Mutex::AutoLock _l(this);
        if(mSource)
            return mSource->open();
        return DFW_E_EINVAL;
    }
    
    dfw_status_t Logger::close(){
        Mutex::AutoLock _l(this);
        if(mSource)
            return mSource->close();
        return DFW_E_EINVAL;
    }
    
    void Logger::onOpen(){
    }
    
    void Logger::onClose(){
        
    }
    
    void Logger::onError(int errnum){
    }
    
    dfw_status_t Logger::send(const dfw_byte_t *data, size_t size, dfw_ulong_t timeout_msec){
        Mutex::AutoLock _l(this);
        if(mSource)
            return mSource->send(data, size, timeout_msec);
        return DFW_E_EINVAL;
    }
    
    dfw_status_t Logger::recv(dfw_byte_t* o_data, size_t *o_size, size_t size, dfw_ulong_t timeout_msec){
        Mutex::AutoLock _l(this);
        if(mSource)
            return mSource->recv(o_data, o_size, size, timeout_msec);
        return DFW_E_EINVAL;
    }
    
    String Logger::makeLog(dfw_logtype_t type, const char *logdata){
        Time time;
        String timeFmt = time.format("%Y-%m-%d %H:%M:%S");
        String currentTime = String::format("%s +%02d:%02d", timeFmt.toBytes(), time.getGmtOffsetHour(), time.getGmtOffsetMin());
        String sData;
        
        size_t size;
        if(logdata)
            size = strlen(logdata);
        else
            size = 0;
        
        char chtype = 'N';
        switch(type){
            case DFW_LOGTYPE_ERROR:
                chtype = 'E';
                break;
            case DFW_LOGTYPE_INFO :
                chtype = 'I';
                break;
            case DFW_LOGTYPE_LOG :
                chtype = 'L';
                break;
            case DFW_LOGTYPE_WARRING :
                chtype = 'W';
                break;
        }
        
        const char *crlf;
#if defined(_WIN32) || defined(_WIN64)
        crlf = "\r";
#else
        crlf = "\r\n";
#endif
        
        if(0==size){
            sData = String::format("%s [%c]%s", currentTime.toBytes(), chtype, crlf);
        }else{
            sData = String::format("%s [%c] %s%s", currentTime.toBytes(), chtype, logdata, crlf);
        }
        
        return sData;
    }
    
    dfw_status_t Logger::write(dfw_logtype_t type, const char *logdata){
        String sData = makeLog(type, logdata);
        if(sData.length())
            return send(sData.toBytes(), sData.length());
        return DFW_E_NODATA;
    }
    
    dfw_status_t Logger::write(const char *data){
        if(!data){
            return DFW_E_EINVAL;
        }
        return send((dfw_byte_t*)data, strlen(data));
    }
    
    dfw_status_t Logger::write(const String &data){
        if(!data.length()){
            return DFW_E_EINVAL;
        }
        return send(data.toBytes(), data.length());
    }
    
    dfw_status_t Logger::log(dfw_logtype_t type, const char *fmt, ...){
        Mutex::AutoLock _l(this);
        
        char *value = NULL;
        if(!fmt || (fmt&&strlen(fmt)==0)){
            return DFW_E_EINVAL;
        }
        
        DFW_VALIST_BEGIN_DECLARE(fmt);
        DFW_VALIST_GET_VALUE(value);
        DFW_VALIST_END_DECLARE();
        
        dfw_status_t status = write(type, value);
        DFW_FREE(value);
        return status;
    }
    
};

