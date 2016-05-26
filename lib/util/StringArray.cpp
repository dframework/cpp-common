#include <dframework/util/StringArray.h>

namespace dframework {

    StringArray::StringArray(){
    }

    StringArray::~StringArray(){
    }

    sp<Retval> StringArray::split(const char* buffer, const char* str){
        sp<Retval> retval;
        const char* buf = buffer;
        dfw_size_t size;
        const char* p;

        while(buf){
            if( !(p = strstr(buf, str)) ){
                sp<String> ele = new String();
                ele->set(buf);
                if(ele->length()){
                    if( DFW_RET(retval, m_aList.insert(ele)) )
                        return DFW_RETVAL_D(retval);
                }
                break;
            }
            if(buf==p){
                buf++;
                continue;
            }
            size = p - buf;
            sp<String> ele = new String();
            ele->set(buf, size);
            if(ele->length()){
                if( DFW_RET(retval, m_aList.insert(ele)) )
                    return DFW_RETVAL_D(retval);
            }
            buf += size;
        }
        return NULL;
    }


};

