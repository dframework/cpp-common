#include <dframework/http/HttpUtils.h>
#include <dframework/net/URI.h>

namespace dframework {


    HttpUtils::HttpUtils(){
    }

    HttpUtils::~HttpUtils(){
    }

    /* static */
    const char *HttpUtils::contentType(const char *fm){
        URI::FileInfo fi;
        fi.set(fm);
        if( fi.m_sExtension.equals("html") ){
            return "text/html";
        }else if( fi.m_sExtension.equals("txt") ){
            return "text/plain";
        }else if( fi.m_sExtension.equals("xml") ){
            return "text/xml";
        }else if( fi.m_sExtension.equals("jpg") ){
            return "image/jpeg";
        }else if( fi.m_sExtension.equals("jpeg") ){
            return "image/jpeg";
        }else if( fi.m_sExtension.equals("gif") ){
            return "image/gif";
        }else if( fi.m_sExtension.equals("png") ){
            return "image/png";
        }else if( fi.m_sExtension.equals("tif") ){
            return "image/tiff";
        }else if( fi.m_sExtension.equals("tiff") ){
            return "image/tiff";
        }else{
            return "application/octet-stream";
        }
    }

};

