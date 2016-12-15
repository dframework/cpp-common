#include <dframework/http/HttpConnection.h>
#include <dframework/http/IHttpReader.h>
#include <dframework/http/HttpRound.h>

#define DFW_HTTPSOCKET_MAX_SIZE  61440
#define DFW_HTTPSOCKET_MAX_SIZE2 61472

namespace dframework {

    IHttpReader::IHttpReader(){
    }

    IHttpReader::~IHttpReader(){
    }

    void IHttpReader::reset(sp<HttpConnection>& conn
                          , sp<HttpRound>& round){
        m_pConnection = conn;
        m_pSocket = conn->m_socket;
        m_pQuery = conn->m_pQuery;
        m_pRound = round;

        if(0==round->m_uRecvContentLength){
            round->m_uRecvContentLength = round->m_sRecvBuffer.length();
            round->m_sReaderBuffer = round->m_sRecvBuffer;
            round->m_sRecvBuffer = "";
        }
    }

    sp<Retval> IHttpReader::readBlocks(){
        sp<Retval> retval;
        size_t r_size = 0;
        size_t size = DFW_HTTPSOCKET_MAX_SIZE;
        char buffer[DFW_HTTPSOCKET_MAX_SIZE2];

        if( !DFW_RET(retval, readLastBlocks()) ){
            return NULL;
        }

        do{
            if( DFW_RET(retval, m_pSocket->wait_recv()) )
                return DFW_RETVAL_D(retval); // with DFW_E_TIMEOUT

            r_size = 0;
            if( DFW_RET(retval, m_pSocket->recv(buffer, &r_size, size)) ){
                int status = retval->value(); //DFW_RETVAL_V(retval);
                if(DFW_E_AGAIN!=status && DFW_E_DISCONNECT!=status){
                    if( DFW_RET(retval, readLastBlocks()) )
                        return DFW_RETVAL_D(retval);
                    return NULL;
                }
                if(r_size==0 && DFW_E_AGAIN==status){
                    if( !DFW_RET(retval, readLastBlocks()) )
                        return NULL;
                    if(DFW_E_AGAIN==status) continue;
                    return NULL;
                }
            }

            if(r_size>0){
                if( DFW_RET(retval, procBlocks(buffer, r_size, false)) )
                    return DFW_RETVAL_D(retval);
                if( !DFW_RET(retval, readLastBlocks()) )
                    return NULL;
            }

            if(DFW_E_DISCONNECT==DFW_RETVAL_V(retval)){
                return DFW_RETVAL_D(retval);
            }

            //usleep(1000);
        }while(true);
    }

    sp<Retval> IHttpReader::readLastBlocks(){
        sp<Retval> retval;
        if(m_pRound->m_uRecvContentLength == m_pRound->m_uContentLength){
            return DFW_RET_C(retval, procBlocks(NULL, 0, true));
        }
        return DFW_RETVAL_NEW(DFW_ERROR,0);
    }

    sp<Retval> IHttpReader::procBlocks(
            const char* buffer, size_t size, bool isended){
        sp<Retval> retval;
        size_t b_size = m_pQuery->getBlockSize();
        size_t offset = 0;
        m_pRound->m_uRecvContentLength += size;

        if(0==b_size){
            return DFW_RET_C(retval, m_pConnection->onResponse(buffer, size));
        }

        do{
            size_t s_size = b_size - m_pRound->m_sReaderBuffer.length();
            if(s_size>size){
                if(isended){
                    if( DFW_RET(retval, m_pConnection->onResponse(
                            m_pRound->m_sReaderBuffer.toChars()
                          , m_pRound->m_sReaderBuffer.length())) ){
                        return DFW_RETVAL_D(retval);
                    }
                    m_pRound->m_sReaderBuffer = "";
                }else{
                    m_pRound->m_sReaderBuffer.append(buffer+offset, size);
                }
                return NULL;
            }else if(s_size){
                m_pRound->m_sReaderBuffer.append(buffer+offset, s_size);
            }

            if( DFW_RET(retval, m_pConnection->onResponse(
                                                   m_pRound->m_sReaderBuffer.toChars()
                                                 , m_pRound->m_sReaderBuffer.length())) ){
                return DFW_RETVAL_D(retval);
            }

            m_pRound->m_sReaderBuffer = "";
            offset += s_size;
            size -= s_size;
        }while(true);
    }

    sp<Retval> IHttpReader::read(unsigned* out, char* b, unsigned s){
        sp<Retval> retval;

        *out = 0;
        size_t r_size = 0;
        size_t b_size = s;

        if(!m_pRound->m_sReaderBuffer.empty()){
            if(m_pRound->m_sReaderBuffer.length() >= s ){
                memcpy( b, m_pRound->m_sReaderBuffer.toChars(), s);
                *out = s;
                m_pRound->m_sReaderBuffer.shift(s);
                return NULL;
            }else{
                unsigned len = m_pRound->m_sReaderBuffer.length();
                memcpy( b, m_pRound->m_sReaderBuffer.toChars(), len);
                *out = len;
                b_size -= len;
                m_pRound->m_sReaderBuffer = "";
                if( m_pRound->m_uRecvContentLength == m_pRound->m_uContentLength ){
                    return NULL;
                }
            }
        }

        do{
            if( b_size == 0 ){
                if( m_pRound->m_uRecvContentLength == m_pRound->m_uContentLength ){
                    // FIXME: return complete value.
                }
                return NULL;
            }

            if( DFW_RET(retval, m_pSocket->wait_recv()) ){
                return DFW_RETVAL_D(retval); // with DFW_E_TIMEOUT
            }

            r_size = 0;
            if( DFW_RET(retval, m_pSocket->recv(b+(*out), &r_size, b_size)) ){
                if(0==r_size){
                    return DFW_RETVAL_D(retval);
                }
            }

            if(r_size>0){
                *out += r_size;
                b_size -= r_size;
                m_pRound->m_uRecvContentLength += r_size;
                continue;
            }

            if( *out == s ){
                return NULL;
            }else if( m_pRound->m_uRecvContentLength == m_pRound->m_uContentLength ){
                return NULL; // FIXME: return complete value.
            }else{
                return NULL;
            }
        }while(true);
    }

};

