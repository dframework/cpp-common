#include <dframework/http/HttpConnection.h>
#include <dframework/http/HttpChunked.h>
#include <dframework/http/HttpStatus.h>

namespace dframework {

    HttpChunked::HttpChunked(){
    }

    HttpChunked::~HttpChunked(){
    }

    sp<Retval> HttpChunked::procChunkedBlocks(
        const char *buffer, size_t size, bool isended)
    {
        sp<Retval> retval;
        size_t offset = 0;
        m_pRound->m_uRecvContentLength += size;

        if(0==m_pQuery->getBlockSize() && !isended){
            return m_pConnection->onResponse(buffer, size);
        }

        do{
            size_t s_size = m_pQuery->getBlockSize() - m_sChunkedBuffer.length();
            if(s_size>size){
                if(isended){
                    if( DFW_RET(retval, m_pConnection->onResponse(
                            m_sChunkedBuffer.toChars()
                          , m_sChunkedBuffer.length())) ){
                        return DFW_RETVAL_D(retval);
                    }

                    if( DFW_RET(retval, m_pConnection->onResponse(NULL, 0)) ){
                        return DFW_RETVAL_D(retval);
                    }

                    m_sChunkedBuffer = "";
                }else{
                    m_sChunkedBuffer.append(buffer+offset, size);
                }
                return NULL;
            }else if(s_size){
                m_sChunkedBuffer.append(buffer+offset, s_size);
            }

            if( DFW_RET(retval, m_pConnection->onResponse(
                    m_sChunkedBuffer.toChars()
                  , m_sChunkedBuffer.length())) ){
                return DFW_RETVAL_D(retval);
            }

            m_sChunkedBuffer = "";
            offset += s_size;
            size -= s_size;
        }while(true);
    }

    sp<Retval> HttpChunked::parseChunkedBlocks(
            size_t *o_size, char *buf, size_t t_size)
    {
        sp<Retval> retval;
        char *org = buf;
        char *pnt;
        //FIXME: unused: size_t offset = 0;
        size_t r_size = 0;
        size_t l_size = t_size;
        *o_size = 0;

        do{
            if(0==l_size){
                goto HttpChunked_parseChunkedBlocks_END;
            }

            if( !(pnt = ::strstr(buf, "\r\n")) ){
                goto HttpChunked_parseChunkedBlocks_END;
            }

            size_t val = ::strtol(buf, NULL, 16);
            if( val==0 && buf[0]=='0'){
                if( DFW_RET(retval, procChunkedBlocks(NULL, 0, true)) )
                    return DFW_RETVAL_D(retval);
                return DFW_RETVAL_NEW(DFW_T_CHUNKED_END, 0);
            }

            size_t len = (pnt - buf) + 2;
            if(len>6)
                return DFW_RETVAL_NEW(DFW_E_HTTP_CHUNKED, 0);

            size_t savelen = len+val+2;
            if( savelen > l_size ){
                goto HttpChunked_parseChunkedBlocks_END;
            }

            if( DFW_RET(retval, procChunkedBlocks(buf+len, val, false)) )
                return DFW_RETVAL_D(retval);

            buf += savelen;
            r_size += savelen;
            l_size -= savelen;
        }while(true);

  HttpChunked_parseChunkedBlocks_END:
        ::memmove((void*)org, (void*)buf, l_size);
        *o_size = r_size;
        return NULL;
    }

    sp<Retval> HttpChunked::readBlocks(){
#define DFW_HTTPSOCKET_MAX_SIZE  61440
#define DFW_HTTPSOCKET_MAX_SIZE2 61472
        sp<Retval> retval;
        size_t r_size = 0;
        size_t o_size = 0;
        size_t offset = 0;
        size_t size = DFW_HTTPSOCKET_MAX_SIZE;
        char buffer[DFW_HTTPSOCKET_MAX_SIZE2];
        bool last = false;
        int status;

	// FIXME
        while(!m_pRound->m_sReaderBuffer.empty()){
            const char *p = m_pRound->m_sReaderBuffer.toChars();
            size_t p_size = m_pRound->m_sReaderBuffer.length();
            if(p_size>size){
                r_size = size;
            }else{
                r_size = p_size;
                last = true;
            }
            ::memmove(buffer+offset, p, r_size);
            m_pRound->m_sReaderBuffer.shift(r_size);
            offset += r_size;
            o_size = 0;
            if( DFW_RET(retval, parseChunkedBlocks(&o_size, buffer, offset)) )
                return DFW_RETVAL_D(retval);

            offset -= o_size;
            size = DFW_HTTPSOCKET_MAX_SIZE - offset;
            if(last)
                break;
        } // end while()

        do{
            if( DFW_RET(retval, m_pSocket->wait_recv()) )
                return DFW_RETVAL_D(retval);

            retval = NULL;
            r_size = 0;
            status = 0;
            if( size>0 && DFW_RET(retval, m_pSocket->recv(buffer+offset
                                                        , &r_size, size)) ){
                DFW_RETVAL_D(retval);
                if( (status = retval->value()) ){
                    if( DFW_E_AGAIN!=status && DFW_E_DISCONNECT!=status )
                        return DFW_RETVAL_D(retval);
                    if(r_size==0 && DFW_E_AGAIN==status)
                        continue;
                }
            }

            if(r_size>0){
                offset += r_size;
                size -= r_size;
            }

            o_size = 0;
            sp<Retval> ps = parseChunkedBlocks(&o_size, buffer, offset);
            if(ps.has()){
                return DFW_RETVAL_D(ps);
            }

            offset -= o_size;
            size = DFW_HTTPSOCKET_MAX_SIZE - o_size;

            if(DFW_E_DISCONNECT==status){
                return DFW_RETVAL_D(retval);
            }
            continue;
        }while(true);
    }


    sp<Retval> HttpChunked::read(unsigned* out, char* b, unsigned s){
        sp<Retval> retval;
        if( DFW_RET(retval, readSub(out, b, s)) ){
            if(retval->value() == DFW_T_CHUNKED_END){ 
                return NULL;
            }
            return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> HttpChunked::readSub(unsigned* out, char* b, unsigned s){
        sp<Retval> retval;
        sp<Retval> ps;

#define MAX_SIZE 102400
        unsigned max_s = MAX_SIZE;
        if( max_s < s ) max_s = s;

        char buffer[max_s];
        unsigned buffer_len = 0;

        *out = 0;
        if(!m_pRound->m_sReaderBuffer.empty()){
            size_t o_size = 0;
            if( DFW_RET(retval, parseChunkedRead(out, b, s, &o_size
                              , m_pRound->m_sReaderBuffer.toChars()
                              , m_pRound->m_sReaderBuffer.length())) )
            {
                m_pRound->m_sReaderBuffer.shift(o_size);
                return DFW_RETVAL_D(retval);
            }

            m_pRound->m_sReaderBuffer.shift(o_size);
            if( *out == s )
                return NULL;

            if( m_pRound->m_sReaderBuffer.length() < max_s )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not enouth buffer(1).");

            if( m_pRound->m_sReaderBuffer.length() ){
                memcpy( buffer, m_pRound->m_sReaderBuffer.toChars(), m_pRound->m_sReaderBuffer.length() );
                buffer_len = m_pRound->m_sReaderBuffer.length();
            }
        }

        do{
            if( buffer_len >= max_s ){
                retval = DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not enouth buffer(2).");
                goto HttpChunked_readSub_END;
            }

            if( DFW_RET(retval, m_pSocket->wait_recv()) ){
                goto HttpChunked_readSub_END; // with DFW_E_TIMEOUT
            }

            size_t r_size = 0;
            if( DFW_RET(retval, m_pSocket->recv(buffer+buffer_len, &r_size, max_s-buffer_len)) ){
                if(r_size == 0){
                    goto HttpChunked_readSub_END;
                }
            }
            buffer_len += r_size;

            unsigned test_out = 0;
            size_t o_size = 0;
            ps = parseChunkedRead(&test_out, b+(*out), s-(*out), &o_size, buffer, buffer_len);

            if( test_out ){
                *out += test_out;
            }

            if(o_size){
                buffer_len -= o_size;
                ::memmove((void*)buffer, (void*)(buffer+o_size), buffer_len);
            }

            if( ps.has() ){
                retval = ps;
                goto HttpChunked_readSub_END;
            }else if( retval.has() ){
                goto HttpChunked_readSub_END;
            }else if( *out == s ){
                goto HttpChunked_readSub_END;
            }
        }while(true);

  HttpChunked_readSub_END:
        if( buffer_len )
            m_pRound->m_sReaderBuffer.set(buffer, buffer_len);
        if( retval.has() )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    /*
        retval: NULL, DFW_T_CHUNKED_END, DFW_E_HTTP_CHUNKED
    */
    sp<Retval> HttpChunked::parseChunkedRead(unsigned* out, char* b, unsigned size
                     , size_t* o_isize, const char* ibuf, size_t isize)
    {
        sp<Retval> retval;
        char *pnt;
        char *buf = (char*)ibuf;
        size_t l_size = isize;
        unsigned offset = 0;
        *o_isize = 0;

        do{
            if(0==l_size){
                goto HttpChunked_parseChunkedRead_END;
            }

            if( !(pnt = ::strstr(buf, "\r\n")) ){
                goto HttpChunked_parseChunkedRead_END;
            }

            size_t val = ::strtol(buf, NULL, 16);
            if( val==0 && buf[0]=='0'){
                retval = DFW_RETVAL_NEW(DFW_T_CHUNKED_END, 0);
                goto HttpChunked_parseChunkedRead_END;
            }

            size_t len = (pnt - buf) + 2;
            if(len>6){
                retval = DFW_RETVAL_NEW(DFW_E_HTTP_CHUNKED, 0);
                goto HttpChunked_parseChunkedRead_END;
            }

            size_t savelen = len+val+2;
            if( savelen > l_size ){
                goto HttpChunked_parseChunkedRead_END;
            }

            unsigned test_osize = 0;
            if( DFW_RET(retval, procChunkedRead(&test_osize, b+offset, size-offset
                                              , buf+len, val)) ){
                goto HttpChunked_parseChunkedRead_END;
            }

            *out += test_osize;
            offset += test_osize;

            buf += savelen;
            l_size -= savelen;
        }while(true);

  HttpChunked_parseChunkedRead_END :
        *o_isize = isize - l_size;
        if( l_size > 0 ){
            ::memmove((void*)ibuf, (void*)buf, l_size);
        }
        if( retval.has() )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    /*
        retval: NULL
    */
    sp<Retval> HttpChunked::procChunkedRead(unsigned* out, char* b, unsigned size
                                          , const char *ibuffer, size_t isize)
    {
        sp<Retval> retval;

        if( isize == 0 ) {
            *out = 0;
            return NULL;
        }

        m_pRound->m_uRecvContentLength += isize;
        m_pRound->m_sChunkedBuffer.append(ibuffer, isize);
 
        const char* cb_buf  = m_pRound->m_sChunkedBuffer.toChars();
             size_t cb_size = m_pRound->m_sChunkedBuffer.length();
        if( size >= cb_size ){
            *out = cb_size;
            memcpy(b, cb_buf, *out);
            m_pRound->m_sChunkedBuffer = NULL;
        }else{
            *out = size;
            memcpy(b, cb_buf, *out);
            m_pRound->m_sChunkedBuffer.shift(size);
        }

        return NULL;
    }

};

