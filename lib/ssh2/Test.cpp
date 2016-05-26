#include "SSH2Session.h"
#include <dframework/io/File.h>

using namespace dframework;

int error(sp<Retval>& retval){
    printf("%s\n", retval->dump().toChars());
    return retval->value();
}

int main(void){
    sp<Retval> retval;

    printf("hello\n");

    URI uri;
    uri.parse("sftp://chk.imgtech.co.kr:22");
    String sUser = "hyun";
    String sPass = "img0701!#";

    sp<SSH2Session> ssh2 = new SSH2Session();
    if( DFW_RET(retval, ssh2->ready(uri, sUser, sPass)) )
        return error(retval);

    sp<DirBox> db = new DirBox();
    if( DFW_RET(retval, ssh2->ftp_readdir("/opt", db)) )
        return error(retval);

printf("end readdir\n");

    struct stat st;
    if( DFW_RET(retval, ssh2->ftp_stat("/opt/test", &st)) )
        return error(retval);

    printf("dir: %d, file: %d\n", st.st_mode & S_IFDIR, st.st_mode & S_IFREG);
    printf("size: %lu\n", st.st_size);

printf("end stat\n");

    if( DFW_RET(retval, ssh2->ftp_open("/opt/zonevd.virtual", O_RDONLY)) )
        return error(retval);

    unsigned outsize = 0;
    unsigned size = 2048;
    char buf[size];
memset(buf, 0, size);
    if( DFW_RET(retval, ssh2->ftp_read(&outsize, buf, size, 0)) ){
        return error(retval);
     }

    if( DFW_RET(retval, ssh2->ftp_close()) )
        return error(retval);

printf("outsize: %d\n", outsize);
printf("%s\n", buf);

    return 0;
}


