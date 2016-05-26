#include <dframework/lang/String.h>
#include <dframework/base/Retval.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace dframework {

    Retval::Stack::Stack(const char* file, int line, const char* func)
    {
        m_iLine = line;
        m_pNext = NULL;
        m_pFile = NULL;
        m_pFunc = NULL;
        if(file){
            int len = strlen(file);
            m_pFile = (char*)malloc(len+1);
            if(m_pFile){
                strncpy(m_pFile, file, len);
                m_pFile[len] = '\0';
            }
        }
        if(func){
            int len = strlen(func);
            m_pFunc = (char*)malloc(len+1);
            if(m_pFunc){
                strncpy(m_pFunc, func, len);
                m_pFunc[len] = '\0';
            }
        }
    }

    Retval::Stack::~Stack(){
        DFW_DELETE(m_pNext);
        DFW_FREE(m_pFile);
        DFW_FREE(m_pFunc);
    }

    // --------------------------------------------------------------

    DFW_DECLARATION_SAFE_COUNT(Retval);

    Retval::Retval(dfw_retno_t retno, dfw_errno_t errnum
                 , const char* file, int line, const char* func) 
            : Object()
    {
        DFW_SAFE_ADD(Retval, l);
        m_iValue = retno;
        m_iErrno = errnum;
        m_pContext = NULL;
        m_pStack = new Stack(file, line, func);
        m_pLastStack = m_pStack;
    }

    Retval::~Retval(){
        DFW_SAFE_REMOVE(Retval, l);
        DFW_DELETE(m_pStack);
    }

    sp<Retval> Retval::set(dfw_retno_t retno, dfw_errno_t errnum){
        m_iValue = retno;
        m_iErrno = errnum;
        return this;
    }

    sp<Retval> Retval::setValue(dfw_retno_t retno){
        m_iValue = retno;
        return this;
    }

    sp<Retval> Retval::setError(dfw_errno_t errnum){
        m_iErrno = errnum;
        return this;
    }

    String Retval::dump(){
        String val;
        if(m_sMessage.empty()){
            val = String::format(
                  "  @ Retval:: errcode=%d, errno=%d"
                , m_iValue, m_iErrno
            );
        }else{
            val = String::format(
                  "  @ Retval:: errcode=%d, errno=%d, %s"
                , m_iValue, m_iErrno, m_sMessage.toChars()
            );
        }

        Stack *stack = m_pStack;
        while(stack){
            String stack_v = String::format(
                  "\n    `-- in %s(..) at %s:%d"
                , stack->m_pFunc, stack->m_pFile, stack->m_iLine
            );
            val.append( stack_v );
            stack = stack->m_pNext;
        }

        return val;
    }

    sp<Retval> Retval::addStack(
          const char* file, int line, const char* func)
    {
        m_pLastStack->m_pNext = new Stack(file, line, func);
        m_pLastStack = m_pLastStack->m_pNext;
        return this;
    }

    // static
    dfw_retno_t Retval::retno(dfw_errno_t errnum){
        DFW_UNUSED(errnum);
        return DFW_ERROR;
    }

    // static
    sp<Retval> Retval::get(dfw_retno_t retno
            , dfw_errno_t errnum
            , const char *file
            , int line
            , const char *func
            , const char *msgfmt, ...)
    {
        String msg;
        sp<Retval> retval = get(retno, errnum, file, line, func);
        if( String::strlen(msgfmt)>0 ){
            char *buf = NULL;
            size_t size;
            va_list ap, ap2;
            DFW_VALIST_SIZE(size, temp, msgfmt, ap);
            DFW_VALIST_STRDUP(buf, size, msgfmt, ap2);
            msg = buf;
            DFW_FREE(buf);
        }
        retval->setMessage(msg);
        return retval;
    }

    // static
    const char* Retval::errno_short(
            dfw_retno_t *rno, int eno, const char* default_msg)
    {
        switch(eno){
        case EINVAL : 
            if(rno) *rno = DFW_E_INVAL;
            return "Invalid parametor";
        case EIO : 
            if(rno) *rno = DFW_E_IO;
            return "EIO";
        case EAGAIN : 
            if(rno) *rno = DFW_E_AGAIN;
            return "EAGAIN";
        case EDEADLK : 
            if(rno) *rno = DFW_E_DEADLK;
            return "EDEADLK";
        case EBADF : 
            if(rno) *rno = DFW_E_BADF;
            return "Bad file number";
        case EACCES : 
            if(rno) *rno = DFW_E_ACCES;
            return "Permission denied";
        case EEXIST :
            if(rno) *rno = DFW_E_EXIST;
            return "File exists";
        case EFAULT :
            if(rno) *rno = DFW_E_FAULT;
            return "Bad address";
        case EFBIG  :
            if(rno) *rno = DFW_E_FBIG;
            return "File too large";
        case EINTR  :
            if(rno) *rno = DFW_E_INTR;
            return "Interrupted";
        case EISDIR :
            if(rno) *rno = DFW_E_ISDIR;
            return "Is a directory";
        case ELOOP  :
            if(rno) *rno = DFW_E_LOOP;
            return "Too many links";
        case ENOLCK :
            if(rno) *rno = DFW_E_NOLCK;
            return "ENOLCK";
        case EMFILE :
            if(rno) *rno = DFW_E_MFILE;
            return "Too many open files";
        case ENAMETOOLONG :
            if(rno) *rno = DFW_E_NAMETOOLONG;
            return "File name too long";
        case ENFILE :
            if(rno) *rno = DFW_E_NFILE;
            return "Too many open files in system";
        case ENODEV :
            if(rno) *rno = DFW_E_NODEV;
            return "Operation not supported by device";
        case ENOENT :
            if(rno) *rno = DFW_E_NOENT;
            return "No such file or directory";
        case ENOMEM :
            if(rno) *rno = DFW_E_NOMEM;
            return "Cannot allocate memory";
        case ENOSPC :
            if(rno) *rno = DFW_E_NOSPC;
            return "No space left on device";
        case ENOTDIR:
            if(rno) *rno = DFW_E_NOTDIR;
            return "Not a directory";
        case ENXIO  :
            if(rno) *rno = DFW_E_NXIO;
            return "Device not configured";
        case EOVERFLOW:
            if(rno) *rno = DFW_E_OVERFLOW;
            return "Value too large to be stored in data type";
        case EPERM  :
            if(rno) *rno = DFW_E_PERM;
            return "Operation not permitted";
        case EROFS  :
            if(rno) *rno = DFW_E_ROFS;
            return "Read-only file system";
        case ENOTTY  :
            if(rno) *rno = DFW_E_NOTTY;
            return "No TTY";
        case EPIPE :
            if(rno) *rno = DFW_E_PIPE;
            return "Broken pipe";
        case ENOPROTOOPT :
            if(rno) *rno = DFW_E_NOPROTOOPT;
            return "No proto OPT";
        case ENOTSOCK :
            if(rno) *rno = DFW_E_NOTSOCK;
            return "Not Sock";
#if !defined(__WIN32__)
        case ETXTBSY:
            if(rno) *rno = DFW_E_TXTBSY;
            return "Text file busy";
#else
        case EWOULDBLOCK:
            if(rno) *rno = DFW_E_AGAIN;
            return "Resource temporarily unavailable";
#endif
        case EAFNOSUPPORT :
            if(rno) *rno = DFW_E_AFNOSUPPORT;
            return "EAFNOSUPPORT";
        case EPROTONOSUPPORT :
            if(rno) *rno = DFW_E_PROTONOSUPPORT;
            return "proto no support";
        case EADDRINUSE :
            if(rno) *rno = DFW_E_ADDRINUSE;
            return "Address in use";
        case EOPNOTSUPP :
            if(rno) *rno = DFW_E_OPNOTSUPP;
            return "OP NOT SUPP";
        case ECONNRESET :
            if(rno) *rno = DFW_E_CONNRESET;
            return "Connection reset by peer";
        default:
            if(rno) *rno = DFW_ERROR;
            return default_msg;
        }
    }


    DFW_STATIC
    const char* Retval::wsa2value(dfw_retno_t *rno, int wsa
                                , const char* default_msg)
    {
#ifndef _WIN32
        if(rno) *rno = DFW_ERROR;
        if(default_msg)
            return default_msg;
        return "WSA Error";
#else
        switch(wsa){
        case WSA_INVALID_HANDLE :
            if(rno) *rno = DFW_E_HANDLE;
            return "Invalid handle (WSA)";
        case WSA_NOT_ENOUGH_MEMORY :
            if(rno) *rno = DFW_E_NOMEM;
            return "Cannot allocate memory (WSA)";
        case WSA_INVALID_PARAMETER :
            if(rno) *rno = DFW_E_INVAL;
            return "Invalid argument (WSA)";
        case WSA_OPERATION_ABORTED :
            if(rno) *rno = DFW_ERROR;
            return "Operation aborted (WSA)";
        case WSA_IO_INCOMPLETE :
            if(rno) *rno = DFW_ERROR;
            return "Io complete (WSA)";
        case WSA_IO_PENDING :
            if(rno) *rno = DFW_ERROR;
            return "Io Pending (WSA)";
        case WSAEINTR :
            if(rno) *rno = DFW_E_INTR;
            return "Interrupted (WSA)";
        case WSAEBADF :
            if(rno) *rno = DFW_E_BADF;
            return "Bad file number (WSA)";
        case WSAEACCES :
            if(rno) *rno = DFW_E_ACCES;
            return "Permission denied (WSA)";
        case WSAEFAULT :
            if(rno) *rno = DFW_E_FAULT;
            return "Bad address (WSA)";
        case WSAEINVAL :
            if(rno) *rno = DFW_E_BIND;
            return "Wrong bind or address (WSA)";
        case WSAEMFILE :
            if(rno) *rno = DFW_E_MFILE;
            return "Too many open files (WSA)";
        case WSAEWOULDBLOCK :
            if(rno) *rno = DFW_E_AGAIN;
            return "Would block (WSA)";
        case WSAEINPROGRESS :
            if(rno) *rno = DFW_E_INPROGRESS;
            return "In progress (WSA)";
        case WSAEALREADY :
            if(rno) *rno = DFW_E_EALREADY;
            return "Eal ready (WSA)";
        case WSAENOTSOCK :
            if(rno) *rno = DFW_E_NOTSOCK;
            return "Not Sock (WSA)";
        case WSAEDESTADDRREQ :
            if(rno) *rno = DFW_E_DESTADDRREQ;
            return "No dest address req (WSA)";
        case WSAEMSGSIZE :
            if(rno) *rno = DFW_E_MSGSIZE;
            return "Clip message size (WSA)";
        case WSAEPROTOTYPE :
            if(rno) *rno = DFW_ERROR;
            return "No Proto type (WSA)";
        case WSAENOPROTOOPT :
            if(rno) *rno = DFW_E_NOPROTOOPT;
            return "No proto OPT (WSA)";
        case WSAEPROTONOSUPPORT :
            if(rno) *rno = DFW_E_PROTONOSUPPORT;
            return "proto no support (WSA)";
        case WSAESOCKTNOSUPPORT :
            if(rno) *rno = DFW_ERROR;
            return "No supported address in socket (WSA)";
        case WSAEOPNOTSUPP :
            if(rno) *rno = DFW_E_OPNOTSUPP;
            return "OP NOT SUPP (WSA)";
        case WSAEPFNOSUPPORT :
            if(rno) *rno = DFW_ERROR;
            return "WSAEPFNOSUPPORT (WSA)";
        case WSAEAFNOSUPPORT :
            if(rno) *rno = DFW_E_AFNOSUPPORT;
            return "WSAEAFNOSUPPORT (WSA)";
        case WSAEADDRINUSE :
            if(rno) *rno = DFW_E_ADDRINUSE;
            return "Address in use (WSA)";
        case WSAEADDRNOTAVAIL :
            if(rno) *rno = DFW_ERROR;
            return "Address not available (WSA)";
        case WSAENETDOWN :
            if(rno) *rno = DFW_ERROR;
            return "Down network (WSA)";
        case WSAENETUNREACH :
            if(rno) *rno = DFW_ERROR;
            return "WSAENETUNREACH (WSA)";
        case WSAENETRESET :
            if(rno) *rno = DFW_ERROR;
            return "WSAENETRESET (WSA)";
        case WSAECONNABORTED :
            if(rno) *rno = DFW_ERROR;
            return "WSAECONNABORTED (WSA)";
        case WSAECONNRESET :
            if(rno) *rno = DFW_E_CONNRESET;
            return "Connection reset by peer (WSA)";
        case WSAENOBUFS :
            if(rno) *rno = DFW_ERROR;
            return "WSAENOBUFS (WSA)";
        case WSAEISCONN :
            if(rno) *rno = DFW_E_ISCONN;
            return "WSAEISCONN (WSA)";
        case WSAENOTCONN :
            if(rno) *rno = DFW_E_NOTCONN;
            return "WSAENOTCONN (WSA)";
        case WSAESHUTDOWN :
            if(rno) *rno = DFW_ERROR;
            return "WSAESHUTDOWN (WSA)";
        case WSAETOOMANYREFS :
            if(rno) *rno = DFW_ERROR;
            return "WSAETOOMANYREFS (WSA)";
        case WSAETIMEDOUT :
            if(rno) *rno = DFW_E_TIMEOUT;
            return "Timeout (WSA)";
        case WSAECONNREFUSED : 
            if(rno) *rno = DFW_E_CONNREFUSED;
            return "WSAECONNREFUSED (WSA)";
        case WSAELOOP :
            if(rno) *rno = DFW_E_LOOP;
            return "Too many symbolic link (WSA)";
        case WSAENAMETOOLONG :
            if(rno) *rno = DFW_E_NAMETOOLONG;
            return "Name too long (WSA)";
        case WSAEHOSTDOWN :
            if(rno) *rno = DFW_ERROR;
            return "Host down (WSA)";
        case WSAEHOSTUNREACH :
            if(rno) *rno = DFW_ERROR;
            return "WSAEHOSTUNREACH (WSA)";
        case WSAENOTEMPTY :
            if(rno) *rno = DFW_ERROR;
            return "Not ready network by WSAStartup (WSA)";
        case WSAEPROCLIM :
            if(rno) *rno = DFW_ERROR;
            return "Too many process (WSA)";
        case WSAEUSERS :
            if(rno) *rno = DFW_ERROR;
            return "Too many users (WSA)";
        case WSAEDQUOT :
            if(rno) *rno = DFW_ERROR;
            return "WSAEDQUOT (WSA)";
        case WSAESTALE :
            if(rno) *rno = DFW_ERROR;
            return "Stale NFS file handle (WSA)";
        case WSAEREMOTE :
            if(rno) *rno = DFW_ERROR;
            return "Too many levels of remote in path (WSA)";
        case WSAEDISCON :
            if(rno) *rno = DFW_ERROR;
            return "Network is exiting (WSA)";
        case WSASYSNOTREADY :
            if(rno) *rno = DFW_ERROR;
            return "Not ready network (WSA)";
        case WSAVERNOTSUPPORTED :
            if(rno) *rno = DFW_ERROR;
            return "Not supported version (WSA)";
        case WSANOTINITIALISED :
            if(rno) *rno = DFW_ERROR;
            return "Not initialised (WSAStartup) (WSA)";
        case WSAHOST_NOT_FOUND :
            if(rno) *rno = DFW_E_HOST_NOT_FOUND;
            return "Not found host (WSA)";
        case WSATRY_AGAIN :
            if(rno) *rno = DFW_ERROR;
            return "Not found request host (WSA)";
        case WSANO_RECOVERY :
            if(rno) *rno = DFW_E_NO_RECOVERY;
            return "No recovery (WSA)";
        case WSANO_DATA :
            if(rno) *rno = DFW_E_NODATA;
            return "No data request name (WSA)";
        default :
            if(rno) *rno = DFW_ERROR;
            if(default_msg)
                return default_msg;
            return "WSA Error";
        }
#endif /* _WIN32 */
    }


}; // end namespace dframework

