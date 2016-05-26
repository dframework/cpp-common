#ifndef DFRAMEWORK_LIB_SSH2SESSION_H
#define DFRAMEWORK_LIB_SSH2SESSION_H

#include <dframework/base/Retval.h>
#include <dframework/net/Socket.h>
#include <dframework/net/URI.h>
#include <dframework/io/DirBox.h>
#include <dframework/ssh2/SSH2Global.h>

#ifdef __cplusplus
namespace dframework {

    class SSH2Session : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(SSH2Session);

        static const int AUTH_TYPE_PASS = 1;
        static const int AUTH_TYPE_KEYBOARD = 2;
        static const int AUTH_TYPE_PUBLICKEY = 4;

    private:
        sp<SSH2Global> m_global;
        void*          m_session;     // LIBSSH2_SESSION
        void*          m_sftp;        // LIBSSH2_SFTP
        void*          m_sftp_handle; // LIBSSH2_SFTP_HANDLE
        uint64_t       m_sftp_offset;
        sp<Socket>     m_sock;

        String m_sFingerPrint;
        String m_sUserAuthList;

        URI    m_Uri;
        String m_sUser;
        String m_sPass;

    public:
        SSH2Session();
        virtual ~SSH2Session();

        sp<Retval> connect(URI& uri
                       , String& sUser, String& sPass
                       , int authType=AUTH_TYPE_PASS);
        sp<Retval> connect(URI& uri, int authType=AUTH_TYPE_PASS);
        inline sp<Retval> ready(URI& uri
                       , String& sUser, String& sPass
                       , int authType=AUTH_TYPE_PASS){
            return connect(uri, sUser, sPass, authType);
        }
        inline sp<Retval> ready(URI& uri, int authType=AUTH_TYPE_PASS){
            return connect(uri, authType);
        }


        sp<Retval> attributes2stat(void* in_attrs, struct stat* out_stat);
        //                               in_attrs: LIBSSH2_SFTP_ATTRIBUTES

        // ----------------------------------------------------------
        // S F T P
        sp<Retval> ftp_readdir(const char* path, sp<DirBox>& db);
        inline sp<Retval> ftp_readdir(const String& path, sp<DirBox>& db){
            return ftp_readdir(path.toChars(), db);
        }
        sp<Retval> ftp_stat(const char* path, struct stat* st);
        inline sp<Retval> ftp_stat(const String& path, struct stat* st){
            return ftp_stat(path.toChars(), st);
        }
        sp<Retval> ftp_open(const char* path, int flags, int mode=0);
        inline sp<Retval> ftp_open(const String& path, int flags, int mode=0){
            return ftp_open(path.toChars(), flags, mode);
        }
        sp<Retval> ftp_close();
        sp<Retval> ftp_read(unsigned *outsize, char* buf
                          , unsigned size, uint64_t offset);
        sp<Retval> ftp_seek(uint64_t offset);


    private:
        sp<Retval> password(int authType);
        sp<Retval> sftp_init();
        sp<Retval> lastError(const char* func);
        sp<Retval> lastError(int result, const char* func);
        sp<Retval> wait();
        sp<Retval> wait_read();

    };

};
#endif


#endif /* DFRAMEWORK_LIB_SSH2SESSION_H */

