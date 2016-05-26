#include <dframework/http/HttpContentType.h>

namespace dframework {

    DFW_STATIC
    const char* HttpContentType::getContentType(const char* ext){
        if( !ext ) {
            return "text/plain";
        }

        int len = ::strlen(ext);
        if( 1==len )
            return getContentType_1(ext);
        else if( 2==len )
            return getContentType_2(ext);
        else if( 3==len )
            return getContentType_3(ext);
        else if( 4==len )
            return getContentType_4(ext);
        return "text/plain";
    }

    DFW_STATIC
    const char* HttpContentType::getContentType_1(const char* ext){
        char ch = ext[0];
        switch(ch){
        case 'c' :
            if( ::strstr(ext, "c")==ext ) return "text/x-c";
            break;

        case 'h' :
            if( ::strstr(ext, "h")==ext ) return "text/x-c";
            break;

        case 's' :
            if( ::strstr(ext, "s")==ext ) return "text/x-asm";
            break;

        case 'q' :
            break;

        case 'w' :
            break;
        }

        return "text/plain";
    }

    DFW_STATIC
    const char* HttpContentType::getContentType_2(const char* ext){
        char ch = ext[0];
        switch(ch){
        case 'c' :
            if( ::strstr(ext, "cc")==ext ) return "text/x-c";
            break;

        case 'h' :
            if( ::strstr(ext, "hh")==ext ) return "text/x-c";
            break;

        case 'i' :
            if( ::strstr(ext, "in")==ext ) return "text/plain";
            break;

        case 'q' :
            if( ::strstr(ext, "qt")==ext ) return "video/quicktime";
            break;

        case 'w' :
            break;
        }

        return "text/plain";
    }

    DFW_STATIC
    const char* HttpContentType::getContentType_3(const char* ext){
        char ch = ext[0];
        switch(ch){
        case 'a' :
            if( ::strstr(ext, "avi")==ext ) return "video/x-msvideo";
            if( ::strstr(ext, "asf")==ext ) return "video/x-ms-asf";
            if( ::strstr(ext, "asx")==ext ) return "video/x-ms-asf";
            if( ::strstr(ext, "asm")==ext ) return "text/x-asm";
            break;

        case 'c' :
            if( ::strstr(ext, "cpp")==ext ) return "text/x-c";
            if( ::strstr(ext, "cxx")==ext ) return "text/x-c";
            if( ::strstr(ext, "css")==ext ) return "text/css";
            if( ::strstr(ext, "csv")==ext ) return "text/csv";
            break;

        case 'd' :
            if( ::strstr(ext, "dic")==ext ) return "text/x-c";
            if( ::strstr(ext, "def")==ext ) return "text/plain";
            if( ::strstr(ext, "dtd")==ext ) return "application/xml-dtd";
            break;

        case 'g' :
            if( ::strstr(ext, "gif")==ext ) return "image/gif";
            break;

        case 'm' :
            if( ::strstr(ext, "mov")==ext ) return "video/quicktime";
            if( ::strstr(ext, "mp4")==ext ) return "video/mp4";
            if( ::strstr(ext, "mpg")==ext ) return "video/mpeg";
            if( ::strstr(ext, "mpe")==ext ) return "video/mpeg";
            if( ::strstr(ext, "m1v")==ext ) return "video/mpeg";
            if( ::strstr(ext, "m2v")==ext ) return "video/mpeg";
            if( ::strstr(ext, "m3u")==ext ) return "audio/x-mpegurl";
            if( ::strstr(ext, "mid")==ext ) return "audio/midi";
            break;

        case 'i' :
            if( ::strstr(ext, "ico")==ext ) return "image/x-icon";
            break;

        case 'j' :
            if( ::strstr(ext, "jpg")==ext ) return "image/jpeg";
            if( ::strstr(ext, "jpe")==ext ) return "image/jpeg";
            break;

        case 'l' :
            if( ::strstr(ext, "log")==ext ) return "text/plain";
            break;

        case 'h' :
            if( ::strstr(ext, "htm")==ext ) return "text/html";
            break;

        case 'p' :
            if( ::strstr(ext, "pcx")==ext ) return "image/x-pcx";
            if( ::strstr(ext, "png")==ext ) return "image/png";
            break;

        case 't' :
            if( ::strstr(ext, "txt")==ext ) return "text/plain";
            if( ::strstr(ext, "tif")==ext ) return "image/tiff";
            break;

        case 'w' :
            if( ::strstr(ext, "wmv")==ext ) return "video/x-ms-wmv";
            if( ::strstr(ext, "wav")==ext ) return "audio/x-wav";
            break;

        case 'x' :
            if( ::strstr(ext, "xml")==ext ) return "application/xml";
            if( ::strstr(ext, "xsl")==ext ) return "application/xml";
            break;

        case 'z' :
            if( ::strstr(ext, "zip")==ext ) return "application/zip";
            break;
        }

        return "text/plain";
    }

    DFW_STATIC
    const char* HttpContentType::getContentType_4(const char* ext){
        char ch = ext[0];
        switch(ch){
        case 'a' :
            break;

        case 'c' :
            if( ::strstr(ext, "conf")==ext ) return "text/plain";
            break;

        case 'l' :
            if( ::strstr(ext, "list")==ext ) return "text/plain";
            break;

        case 'm' :
            if( ::strstr(ext, "mp4v")==ext ) return "video/mp4";
            if( ::strstr(ext, "mpg4")==ext ) return "video/mp4";
            if( ::strstr(ext, "mpeg")==ext ) return "video/mpeg";
            if( ::strstr(ext, "midi")==ext ) return "audio/midi";
            break;

        case 'h' :
            if( ::strstr(ext, "h264")==ext ) return "video/h264";
            if( ::strstr(ext, "html")==ext ) return "text/html";
            break;

        case 'j' :
            if( ::strstr(ext, "jpev")==ext ) return "video/jpeg";
            if( ::strstr(ext, "jpeg")==ext ) return "image/jpeg";
            break;

        case 't' :
            if( ::strstr(ext, "text")==ext ) return "text/plain";
            if( ::strstr(ext, "tiff")==ext ) return "image/tiff";
            break;

        case 'w' :
            break;
        }

        return "text/plain";
    }

};

