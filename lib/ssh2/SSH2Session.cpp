#include <dframework/ssh2/SSH2Session.h>

#include <dframework/io/File.h>

#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#define SESSION()      ((LIBSSH2_SESSION*)m_session)
#define SFTP_SESSION() ((LIBSSH2_SFTP*)m_sftp)
#define SFTP_HANDLE()  ((LIBSSH2_SFTP_HANDLE*)m_sftp_handle)

namespace dframework {

    DFW_DECLARATION_SAFE_COUNT(SSH2Session);

    SSH2Session::SSH2Session(){
        DFW_SAFE_ADD(SSH2Session, l);
        m_session = NULL;
        m_sftp = NULL;
        m_sftp_handle = NULL;
        m_sftp_offset = 0;
    }

    SSH2Session::~SSH2Session(){
        if( m_sftp_handle ){
            ::libssh2_sftp_close(SFTP_HANDLE());
            m_sftp_handle = NULL;
        }
        if( m_sftp ){
            ::libssh2_sftp_shutdown(SFTP_SESSION());
            m_sftp = NULL;
        }
        if(m_session){
            ::libssh2_session_disconnect(SESSION(), "Shutdown SSH2Session.");
            ::libssh2_session_free(SESSION());
            m_session=NULL;
        }
        DFW_SAFE_REMOVE(SSH2Session, l);
    }

    sp<Retval> SSH2Session::connect(URI& uri
                                , String& sUser, String& sPass
                                , int authType)
    {
        sp<Retval> retval;
        int rc;

        m_Uri = uri;
        m_sUser = sUser;
        m_sPass = sPass;

        if( DFW_RET(retval, SSH2Global::instance(m_global)) )
            return DFW_RETVAL_D(retval);

        m_sock = new Socket();
        if( DFW_RET(retval, m_sock->connect(m_Uri)) )
            return DFW_RETVAL_D(retval);

        m_session = (void*)::libssh2_session_init();
        if( !m_session )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not libssh2_session_init.");

        if((rc=::libssh2_session_handshake(SESSION(), m_sock->getHandle())))
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Not libssh2_session_handshake. "
                         "rc=%d, sock=%d", rc, m_sock->getHandle());

        m_sFingerPrint = ::libssh2_hostkey_hash(SESSION()
                                            , LIBSSH2_HOSTKEY_HASH_SHA1);

        if( DFW_RET(retval, password(authType)) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    sp<Retval> SSH2Session::connect(URI& uri, int authType){
        sp<Retval> retval;
        String sUser = uri.getUser();
        String sPass = uri.getPass();
        if( DFW_RET(retval, connect(uri, sUser, sPass, authType)) )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Session::password(int authType){
        int auth_pw = 0;
        m_sUserAuthList = ::libssh2_userauth_list(SESSION()
                                              , m_sUser.toChars()
                                              , m_sUser.length());

        if( m_sUserAuthList.indexOf("password")!=0 )
            auth_pw |= AUTH_TYPE_PASS;
        if( m_sUserAuthList.indexOf("keyboard-interactive")!=0 )
            auth_pw |= AUTH_TYPE_KEYBOARD;
        if( m_sUserAuthList.indexOf("publickey")!=0 )
            auth_pw |= AUTH_TYPE_PUBLICKEY;

        if((auth_pw & AUTH_TYPE_PASS) && (AUTH_TYPE_PASS==authType))
            auth_pw = AUTH_TYPE_PASS;
        if((auth_pw & AUTH_TYPE_KEYBOARD) && (AUTH_TYPE_KEYBOARD==authType))
            auth_pw = AUTH_TYPE_KEYBOARD;
        if((auth_pw & AUTH_TYPE_PUBLICKEY) && (AUTH_TYPE_PUBLICKEY==authType))
            auth_pw = AUTH_TYPE_PUBLICKEY;

        if( auth_pw & AUTH_TYPE_PASS ){
            if( ::libssh2_userauth_password(SESSION()
                                        , m_sUser.toChars()
                                        , m_sPass.toChars()) )
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Authentication by password failed.");
        }else if( auth_pw & AUTH_TYPE_KEYBOARD ){
#if 0
            if( ::libssh2_userauth_keyboard_interactive(SESSION()
                                        , m_sUser.toChars()
                                        , &SSH2Session_keyboard_callback))
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Authentication by keyboard-interactive failed.");
#endif
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support Authentication by keyboard-interactive.");
        }else if( auth_pw & AUTH_TYPE_PUBLICKEY ){
#if 0
            if( ::libssh2_userauth_publickey_fromfile(SESSION()
                                        , m_sUser.toChars()
                                        , keyfile1, keyfile2
                                        , m_sPass.toChars() ))
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Authentication by public key failed");
#endif
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not support Authentication by public key.");
        }else{
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "No supported authentication methods found.");
        }
        return NULL;
    }

    sp<Retval> SSH2Session::attributes2stat(void* in, struct stat* st){
        LIBSSH2_SFTP_ATTRIBUTES* attr = (LIBSSH2_SFTP_ATTRIBUTES*)in;
        if( !in )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                   , "in attributes parametor is null.");
        if( !st )
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                   , "out stat parametor is null.");

        ::memset(st, 0, sizeof(struct stat));

        if( attr->flags & LIBSSH2_SFTP_ATTR_SIZE )
            st->st_size = attr->filesize;

        if( attr->flags & LIBSSH2_SFTP_ATTR_PERMISSIONS )
            st->st_mode = attr->permissions;

        if( attr->flags & LIBSSH2_SFTP_ATTR_ACMODTIME ){
            st->st_atime = attr->atime;   
            st->st_mtime = attr->mtime;   
        }

        if( attr->flags & LIBSSH2_SFTP_ATTR_UIDGID ){
            st->st_uid = attr->uid;
            st->st_gid = attr->gid;
        }

        return NULL;
    }

    sp<Retval> SSH2Session::lastError(const char* func){
        String sMsg;
        char* errmsg = NULL;
        int err_len = 0;
        int rc = ::libssh2_session_last_error(SESSION(),&errmsg,&err_len,0);
        sMsg.set(errmsg, err_len);
        //DFW_FREE(errmsg);

        switch( rc ){
        case LIBSSH2_ERROR_ALLOC:
            return DFW_RETVAL_NEW_MSG(DFW_E_NOMEM, rc
                   , "NOMEM: %s at %s", sMsg.toChars(), func);
        case LIBSSH2_ERROR_SOCKET_SEND:
            return DFW_RETVAL_NEW_MSG(DFW_E_WRITE, rc
                   , "NO SOCKET SEND: %s at %s", sMsg.toChars(), func);
        case LIBSSH2_ERROR_SOCKET_TIMEOUT:
            return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, rc
                   , "SOCKET TIMEOUT: %s at %s", sMsg.toChars(), func);
        case LIBSSH2_ERROR_SFTP_PROTOCOL:
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, rc
                   , "SFTP PROTOCOL: %s at %s", sMsg.toChars(), func);
        }
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, rc
                   , "UNKNOWN: %s, res=%d at %s", sMsg.toChars(), rc, func);
    }

    sp<Retval> SSH2Session::lastError(int result, const char* func){
        switch( result ){
        case LIBSSH2_ERROR_ALLOC:
            return DFW_RETVAL_NEW_MSG(DFW_E_NOMEM, result
                   , "ERROR: NOMEM at %s", func);
        case LIBSSH2_ERROR_SOCKET_SEND:
            return DFW_RETVAL_NEW_MSG(DFW_E_WRITE, result
                   , "ERROR: NO SOCKET SEND at %s", func);
        case LIBSSH2_ERROR_SOCKET_TIMEOUT:
            return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, result
                   , "ERROR: SOCKET TIMEOUT at %s", func);
        case LIBSSH2_ERROR_SFTP_PROTOCOL:
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, result
                   , "ERROR: SFTP PROTOCOL at %s", func);
        }
        return DFW_RETVAL_NEW_MSG(DFW_ERROR, result, "ERROR: at %s", func);
    }

    sp<Retval> SSH2Session::wait(){
        sp<Retval> retval;

        struct timeval timeout;
        int rc;
        fd_set fd;
        fd_set *writefd = NULL;
        fd_set *readfd = NULL;
        int dir;
 
        timeout.tv_sec  = 5;
        timeout.tv_usec = 0;
 
        FD_ZERO(&fd);
        FD_SET(m_sock->getHandle(), &fd);
 
        /* now make sure we wait in the correct direction */ 
        dir = ::libssh2_session_block_directions(SESSION());
        if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
            readfd = &fd;
        if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
            writefd = &fd;
 
        rc = ::select(m_sock->getHandle()+1, readfd, writefd, NULL, &timeout);
        switch( rc ){
        case -1 :
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, errno, "Not select.");
        case 0 :
            return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0, "Timeout select.");
        }
        return NULL;
    }

    sp<Retval> SSH2Session::wait_read(){
        sp<Retval> retval;

        struct timeval timeout;
        int rc;
        fd_set fd;
 
        timeout.tv_sec  = 5;
        timeout.tv_usec = 0;
 
        FD_ZERO(&fd);
        FD_SET(m_sock->getHandle(), &fd);
 
        rc = ::select(m_sock->getHandle()+1, &fd, &fd, NULL, &timeout);
        switch( rc ){
        case -1 :
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, errno, "Not select.");
        case 0 :
            return DFW_RETVAL_NEW_MSG(DFW_E_TIMEOUT, 0, "Timeout select.");
        }
        return NULL;
    }

    // --------------------------------------------------------------
    //    S F T P
    // --------------------------------------------------------------

    sp<Retval> SSH2Session::sftp_init(){
        if(!m_session)
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                   , "Has not session or ready.");
        if( !(m_sftp = ::libssh2_sftp_init(SESSION())) )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Unable to init SFTP session");
        return NULL;
    }

    sp<Retval> SSH2Session::ftp_readdir(const char* path, sp<DirBox>& db){
        sp<Retval> retval;
        int rc;
        LIBSSH2_SFTP_HANDLE* handle;

        if( !m_sftp && DFW_RET(retval, sftp_init()) )
            return DFW_RETVAL_D(retval);

        if( !(handle = ::libssh2_sftp_opendir(SFTP_SESSION(), path)) )
            return DFW_RET_C(retval, lastError("libssh2_sftp_opendir"));

        do{
            char mem[1024];
            char longentry[1024];
            LIBSSH2_SFTP_ATTRIBUTES attrs;
            ::memset(&attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));

            /* loop until we fail */
            if(!(rc = ::libssh2_sftp_readdir_ex(handle, mem, sizeof(mem)
                                   , longentry, sizeof(longentry), &attrs))){
                /* rc is the length of the file name in the mem buffer */
                break;
            }

            sp<Stat> st = new Stat();
            st->setName(mem);
            if( DFW_RET(retval, attributes2stat(&attrs, &st->m_stat)) ){
                DFW_RETVAL_D(retval);
                break;
            }
            if( DFW_RET(retval, db->insert(st)) ){
                DFW_RETVAL_D(retval);
                break;
            }
        }while(1);

        ::libssh2_sftp_closedir(handle);

        if( retval.has() )
            return DFW_RETVAL_D(retval);
        return NULL;
    }

    sp<Retval> SSH2Session::ftp_stat(const char* path, struct stat* st){
        sp<Retval> retval;
        int rc;
        LIBSSH2_SFTP_ATTRIBUTES attrs;

        ::memset(&attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));

        if( !m_sftp && DFW_RET(retval, sftp_init()) )
            return DFW_RETVAL_D(retval);

        if( (rc = ::libssh2_sftp_stat(SFTP_SESSION(), path, &attrs)) )
            return DFW_RET_C(retval, lastError("libssh2_sftp_stat"));

        if( DFW_RET(retval, attributes2stat(&attrs, st)) )
            return DFW_RETVAL_D(retval);

        return NULL;
    }

    sp<Retval> SSH2Session::ftp_open(const char* path, int flags, int mode){
        sp<Retval> retval;

        if( !m_sftp && DFW_RET(retval, sftp_init()) )
            return DFW_RETVAL_D(retval);

        long flags_l = 0;
        long mode_l = mode;

        if( flags & O_RDONLY ) flags_l |= LIBSSH2_FXF_READ;
        if( flags & O_WRONLY ) flags_l |= LIBSSH2_FXF_WRITE;
        if( flags & O_RDWR )   flags_l |= (LIBSSH2_FXF_WRITE|LIBSSH2_FXF_READ);
        if( flags & O_APPEND ) flags_l |= LIBSSH2_FXF_APPEND;
        if( flags & O_CREAT )  flags_l |= LIBSSH2_FXF_CREAT;
        if( flags & O_TRUNC )  flags_l |= LIBSSH2_FXF_TRUNC;
        if( flags & O_EXCL )   flags_l |= LIBSSH2_FXF_EXCL;

        ftp_close();
        m_sftp_offset = 0;

        do{
            if( !(m_sftp_handle = ::libssh2_sftp_open(SFTP_SESSION()
                                           , path, flags_l, mode_l)) ){
                retval = lastError("libssh2_sftp_open");
                if( retval->error() != LIBSSH2_ERROR_EAGAIN )
                    return DFW_RETVAL_D(retval);
                if( DFW_RET(retval, wait()) )
                    return DFW_RETVAL_D(retval);
            }
        }while(!m_sftp_handle);

        return NULL;
    }

    sp<Retval> SSH2Session::ftp_close(){
        if( m_sftp_handle ){
            ::libssh2_sftp_close(SFTP_HANDLE());
            m_sftp_handle = NULL;
        }
        m_sftp_offset = 0;
        return NULL;
    }

    sp<Retval> SSH2Session::ftp_seek(uint64_t offset){
        ::libssh2_sftp_seek(SFTP_HANDLE(), (size_t)offset);
        m_sftp_offset = offset;
        return NULL;
    }
 
    sp<Retval> SSH2Session::ftp_read(unsigned *outsize, char* buf
                                   , unsigned size, uint64_t offset)
    {
        sp<Retval> retval;
        unsigned n_off = 0;

        if(m_sftp_offset != offset){
            ftp_seek(offset);
        }

        do{
            int rsize = libssh2_sftp_read(SFTP_HANDLE(), buf+n_off, size-n_off);
            if( rsize < 0 ){
                if(rsize != LIBSSH2_ERROR_EAGAIN)
                    return DFW_RET_C(retval, lastError(rsize, "libssh2_sftp_read"));
                if( DFW_RET(retval, wait_read()) )
                    return DFW_RETVAL_D(retval);
                continue;
            }else if(rsize == 0 ){
                return NULL;
            }else{
                n_off += rsize;
                *outsize = n_off;
                if( n_off == size )
                    return NULL;
            }
        }while(1);
    }

};

