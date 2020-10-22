#include <dframework/http/HttpQuery.h>
#include <dframework/http/HttpMakeClientBuffer.h>
#include <dframework/http/HttpUtils.h>
#include <dframework/io/Stat.h>
#include <dframework/log/Logger.h>

namespace dframework {


    HttpMakeClientBuffer::HttpMakeClientBuffer() :
            m_pQ(NULL),
            m_pR(NULL),
            m_uContentLength(0),
            m_iDepth(0),
            m_bDepth(false),
            m_bMultipart(false),
            m_bExpect100(false)
    {
    }

    HttpMakeClientBuffer::~HttpMakeClientBuffer(){
    }

    sp<Retval> HttpMakeClientBuffer::make(
            sp<HttpQuery>& q, sp<HttpRound>& round)
    {
        sp<Retval> retval;
        String sPath, sContents, sDepth;

        m_pQ = q;
        m_pR = round;

        retval = makeHost();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        retval = makeContents();
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        if( m_bMultipart ){
            m_bExpect100 = true;
        }
        if(m_uContentLength>0){
            sContents = String::format(
                  "Content-Type: %s\r\nContent-Length: %ld\r\n"
                , m_sContentType.toChars(), m_uContentLength
            );
        }
        if(m_bDepth){
            sDepth = String::format("Depth: %d\r\n", m_iDepth);
        }

        const char *pPath = m_pR->m_oUri.getPath().toChars();
        const char *pQuery = m_pR->m_oUri.getQuery().toChars();
        if(pQuery){
            sPath = String::format("%s?%s", pPath, pQuery);
        }else{
            sPath = String::format("%s", pPath);
        }
        sPath.replace(" ", "%20");

        m_pR->m_sHeadBuffer = String::format(
              "%s %s HTTP/1.1\r\n"
              "%s" /* Host: hostname:port */
              "Connection: Keep-Alive\r\n"
              "%s" /* Expect: ... */
              "%s" /* ContentsHeader ... */
              "%s" /* Depth */
              "User-Agent: %s\r\n"
            , m_pQ->m_sMethod.toChars(), sPath.toChars()
            , m_sHost.toChars()
            , (m_bExpect100 ? "Expect: 100-Continue\r\n":"")
            , (sContents.length()>0 ? sContents.toChars():"")
            , (sDepth.length()>0 ? sDepth.toChars():"")
            , m_pQ->m_sUserAgent.toChars()
        );

        if( DFW_RET(retval, appendHeaders(m_pQ->m_requestHeader)) )
            return DFW_RETVAL_D(retval);

        m_pR->m_sHeadBuffer.append("\r\n", 2);

        if(m_sContents.length()>0){
            m_pR->m_sHeadBuffer.append(
                    m_sContents.toChars(), m_sContents.length());
        }

        return NULL;
    }

    sp<Retval> HttpMakeClientBuffer::makeHost(){
        const char *host;
        int port;

        if(m_pQ->m_sVirtualHost.empty()){
            host = m_pR->m_oUri.getHost().toChars();
            port = m_pR->m_oUri.getPort();
        }else{
            host = m_pQ->m_sVirtualHost.toChars();
            port = m_pQ->m_iVirtualPort;
            if(0==port){
                port = m_pR->m_oUri.getPort();
            }
        }

        if(0==port || NULL==host || m_pQ->m_sMethod.empty()){
            return DFW_RETVAL_NEW(DFW_E_INVAL,0);
        }

        if(port==80){
            m_sHost = String::format("Host: %s:80\r\n", host);
        }else{
            m_sHost = String::format("Host: %s:%d\r\n", host, port);
        }

        return NULL;
    }

    sp<Retval> HttpMakeClientBuffer::makeContents()
    {
        bool bMultipart = false;
        int size = m_pQ->m_aFormDataList.size();
        int k;

        if(m_pQ->m_sMethod.equals("POST")){
            bMultipart = true;
        }else if(m_pQ->m_sMethod.equals("PROPFIND")){
            m_sContents = String::format(
                    "<?xml version=\"1.0\"?>"
                    "<D:propfind xmlns:D=\"DAV:\">\r\n"
                    "  <D:allprop>\r\n"
                    "    <D:namespaces>\r\n"
                    "      <D:namespace uri=\"DAV:\"/>\r\n"
                    "    </D:namespaces>\r\n"
                    "  </D:allprop>\r\n"
                    "</D:propfind>\r\n"
            );
            m_sContentType = "text/xml";
            m_uContentLength = m_sContents.length();
            m_iDepth = 1;
            m_bDepth = true;
            return NULL;
        }else{
            for(k=0; k<size; k++){
                sp<HttpFormData> data = m_pQ->m_aFormDataList.get(k);
                if(data.has() && data->m_bIsUri){
                    bMultipart = true;
                    break;
                }
            }
        }

        if(bMultipart){
            m_bMultipart = true;
            return makeMultipartContents();
        }

        return NULL;
    }

    sp<Retval> HttpMakeClientBuffer::makeMultipartContents(){
        sp<Retval> retval;
        dfw_size_t contLen = 0;
        int size = m_pQ->m_aFormDataList.size();
        int k;

        m_sBoundary = "----BOUNDARY"; // FIXME
        m_sContentType = String::format(
                "multipart/form-data; boundary=%s"
              , m_sBoundary.toChars()
        );

        for(k=0; k<size; k++){
            sp<HttpFormData> data = m_pQ->m_aFormDataList.get(k);
            if(!data.has()) continue;
            if(data->m_bIsUri){
                data->m_oUri = data->m_sUri;
                data->m_oFileInfo.set(data->m_oUri.getPath().toChars());
                if(data->m_sType.empty()){
                    const char *p = data->m_oFileInfo.m_sExtension.toChars();
                    data->m_sType = HttpUtils::contentType(p);
                }
#ifndef _WIN32
                if(data->m_oUri.getScheme().equals("File")){
                    Stat st;
                    retval =st.stat(data->m_oUri);
                    if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);
                    data->m_nContentLength = (dfw_size_t)st.m_stat.st_size;
                }
#endif
                data->m_sBuffer = String::format(
                        "%s\r\n" // boundary
                        "Content-Disposition: form-data; name=\"%s\"; "
                        "filename=\"%s\"\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %ld\r\n"
                        "\r\n"
                      , m_sBoundary.toChars()
                      , data->m_sName.toChars()
                      , data->m_oFileInfo.m_sFilename.toChars()
                      , data->m_sType.toChars()
                      , data->m_nContentLength
                );
            }else{
                data->m_nContentLength = data->m_sValue.length();
                data->m_sBuffer = String::format(
                        "%s\r\n" /* boundary */
                        "Content-Disposition: form-data; name=\"%s\"\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n"
                      , m_sBoundary.toChars()
                      , data->m_sName.toChars()
                      , data->m_sValue.length()
                );
            }
            contLen += (data->m_sBuffer.length()+data->m_nContentLength+2);
        }

        m_uContentLength = contLen + 2 /* 2 is end boundary "--" */;

        return NULL;
    }

    sp<Retval> HttpMakeClientBuffer::appendHeaders(sp<HttpHeader>& h){
        for(int k=0; k<h->m_aList.size(); k++){
            sp<NamedValue> nv = h->m_aList.get(k);
            if(nv.has() && !nv->m_sName.empty()){
                String temp = String::format("%s: %s\r\n"
                        , nv->m_sName.toChars()
                        , (nv->m_sValue.empty()?"":nv->m_sValue.toChars())
                );
                m_pR->m_sHeadBuffer.append(temp.toChars(), temp.length());
            }
        }
        return NULL;
    }

};

