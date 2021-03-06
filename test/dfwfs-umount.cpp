#include <dframework/fscore/FuseMount.h>

using namespace dframework;

int main(void)
{
    sp<Retval> retval;

    sp<FuseMount>  mount  = new FuseMount();

    if( DFW_RET(retval, mount->umount("/opt/fuse-mount")) ){
        printf("%s\n", retval->dump().toChars() );
        return 1;
    }

    return 0;
};


