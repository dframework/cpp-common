#include <dframework/http/HttpRound.h>
#include <dframework/util/Time.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>

namespace dframework {


    HttpRound::HttpRound(int round)
            : Object()
    {
        m_iRound = round;
        m_oUri = NULL;
        m_iMajorVersion = 0;
        m_iMinorVersion = 0;
        m_iStatus = 0;
        m_uRecvContentLength = 0;
        m_bChunked = false;
        m_tLastModified = 0;
        m_tDate = 0;
        m_tExpires = 0;
        m_bKeepAlive = false;
        m_uContentLength = 0;
        m_iTimeout = 0;
        m_iTimeoutMax = 0;

        m_bUseSpecialStatusCode = true;
        m_bOnRecvContents = true;
        m_responseHeader = new HttpHeader();
    }

    HttpRound::~HttpRound(){
    }

    sp<Retval> HttpRound::ready(const char *uri){
        m_sUri   = uri;
        m_oUri   = uri;
        return parseUri();
    }

    sp<Retval> HttpRound::ready(const URI& uri){
        m_sUri   = uri.toString();
        m_oUri   = uri;
        return parseUri();
    }

    sp<Retval> HttpRound::parseUri(){
        sp<Retval> retval;
        if(m_oUri.getHost().empty()){
            return DFW_RETVAL_NEW(DFW_E_HOSTNAME,0);
        }
        if(m_oUri.getScheme().empty()){
            m_oUri.setScheme("http");
        }
        if(m_oUri.getPort()==0){
            m_oUri.setPort(80);
        }
        if(m_oUri.getPath().empty()){
            m_oUri.setPath("/");
        }
        if( DFW_RET(retval, m_oHostname.get(m_oUri)) ){
            return DFW_RETVAL_D(retval);
        }
        if( m_oHostname.size()<=0 ){
            return DFW_RETVAL_NEW(DFW_E_HOST_NOT_FOUND,0);
        }
        return NULL;
    }

    sp<Retval> HttpRound::readLine(){
        m_sLineBuffer = "";
        if(m_sRecvBuffer.empty()){
            return DFW_RETVAL_NEW(DFW_ERROR,0);
        }

        const char *buffer = m_sRecvBuffer.toChars();
        const char *test = strstr(buffer, "\r\n");
        if(!test){
            return DFW_RETVAL_NEW(DFW_ERROR,0);
        }

        dfw_size_t size = test-buffer;
        m_sLineBuffer.append(buffer, size);
        m_sRecvBuffer.shift(size+2);

        return NULL;
    }

    sp<Retval> HttpRound::parseStatusLine(){
        sp<Retval> retval;
        String r_maj, r_min, r_status;
        const char *line = m_sLineBuffer.toChars();

        Regexp exp("^HTTP/([0-9]+)\\.([0-9]+) ([0-9]+)");
        retval = exp.regexp(line);
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        r_maj.set(exp.getMatch(1), exp.getMatchLength(1));
        r_min.set(exp.getMatch(2), exp.getMatchLength(2));
        r_status.set(exp.getMatch(3), exp.getMatchLength(3));

        m_iMajorVersion = Integer::parseInt(r_maj);
        m_iMinorVersion = Integer::parseInt(r_min);
        m_iStatus = Integer::parseInt(r_status);
        m_sStatusMsg = line + exp.getMatchLength(0);

        return NULL;
    }

    sp<Retval> HttpRound::parseHeaderLine(){
        String name, value;

        const char *p = m_sLineBuffer.toChars();
        const char *test = strstr(p, ":");
        if(!test){
            return DFW_RETVAL_NEW(DFW_ERROR,0);
        }

        dfw_size_t depth = test-p;
        name.set(p, depth);
        if(m_sLineBuffer.length()>(depth+1)){
            value.set(test+2);
        }

        patchHeader(name, value);

        m_responseHeader->appendHeader(name, value);

        return NULL;
    }

    sp<Retval> HttpRound::patchHeader(String& name, String& value)
    {
        sp<Retval> retval;
        if(name.equals("Transfer-Encoding")){
            if(value.equals("chunked")){
                 m_bChunked = true;   
            }
            return NULL;
        }else if(name.equals("Content-Type")){
            parseContentType(value.toChars());
        }else if(name.equals("Last-Modified") ){
            Time::parse(&m_tLastModified, value.toChars());
        }else if(name.equals("Date") ){
            Time::parse(&m_tDate, value.toChars());
        }else if(name.equals("Expires") ){
            Time::parse(&m_tExpires, value.toChars());
        }else if(name.equals("Connection") ){
            if(value.equals("Keep-Alive")){
                m_bKeepAlive = true;
            }else{
                m_bKeepAlive = false;
            }
        }else if(name.equals("Keep-Alive") ){
            /* FIXME: */
        }else if(name.equals("Content-Length") ){
            m_uContentLength = Long::parseLong(value);
        }else if(name.equals("Location") ){
            m_sLocation = value;
        }else if(name.equals("Server") ){
            m_sServer = value;
        }else if(name.equals("Authentication-Info") ){
            /* FIXME: */
        }
        return NULL;
    }

    sp<Retval> HttpRound::parseContentType(const char *str){
        sp<Retval> retval;
        bool isFind = false;
        String ctype;

        m_sContentTypeLine = str;

        if(!isFind) {
            Regexp exp("^[\\s]*([a-zA-Z0-9]*)[\\s]*/[\\s]*([a-zA-Z0-9]*)[\\s]*;[\\s]*([a-zA-Z0-9]*)[\\s]*=([a-zA-Z0-9-_\"\'\\s]*)");
            retval = exp.regexp(str);
            if(!DFW_RETVAL_H(retval)){
                m_sContentType.set(
                        exp.getMatch(1), exp.getMatchLength(1));
                m_sContentSubType.set(
                        exp.getMatch(2), exp.getMatchLength(2));
                ctype.set(exp.getMatch(3), exp.getMatchLength(3));
                //FIXME: unused: const char *charset = exp.getMatch(4);
                if(ctype.equals("charset")){
                    m_sCharset.set(
                            exp.getMatch(4), exp.getMatchLength(4));
                }else{
                    m_sCharset.set(
                            exp.getMatch(4), exp.getMatchLength(4));
                }
                isFind = true;
            }
        }
        if(!isFind) {
            Regexp exp("^[\\s]*([a-zA-Z0-9]*)/([a-zA-Z0-9]*)[\\s]*");
            retval = exp.regexp(str);
            if(!DFW_RETVAL_H(retval)){
                m_sContentType.set(
                        exp.getMatch(1), exp.getMatchLength(1));
                m_sContentSubType.set(
                        exp.getMatch(2), exp.getMatchLength(2));
                isFind = true;
            }
        }
        return NULL;
    }

};

