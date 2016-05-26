#ifndef _WIN32
#include <dframework/fscore/Fuse_d.h>

namespace dframework {

    const char* FUSE::getOpcodeName(uint32_t opcode){
        switch(opcode){
        case OP_LOOKUP       : return "LOOKUP";
        case OP_FORGET       : return "FORGET";
        case OP_GETATTR      : return "GETATTR";
        case OP_SETATTR      : return "SETATTR";
        case OP_READLINK     : return "READLINK";
        case OP_SYMLINK      : return "SYMLINK";
        case OP_MKNOD        : return "MKNOD";
        case OP_MKDIR        : return "MKDIR";
        case OP_UNLINK       : return "UNLINK";
        case OP_RMDIR        : return "RMDIR";
        case OP_RENAME       : return "RENAME";
        case OP_LINK         : return "LINK";
        case OP_OPEN         : return "OPEN";
        case OP_READ         : return "READ";
        case OP_WRITE        : return "WRITE";
        case OP_STATFS       : return "STATFS";
        case OP_RELEASE      : return "RELEASE";
        case OP_FSYNC        : return "FSYNC";
        case OP_SETXATTR     : return "SETXATTR";
        case OP_GETXATTR     : return "GETXATTR";
        case OP_LISTXATTR    : return "LISTXATTR";
        case OP_REMOVEXATTR  : return "REMOVEXATTR";
        case OP_FLUSH        : return "FLUSH";
        case OP_INIT         : return "INIT";
        case OP_OPENDIR      : return "OPENDIR";
        case OP_READDIR      : return "READDIR";
        case OP_RELEASEDIR   : return "RELEASEDIR";
        case OP_FSYNCDIR     : return "FSYNCDIR";
        case OP_GETLK        : return "GETLK";
        case OP_SETLK        : return "SETLK";
        case OP_SETLKW       : return "SETLKW";
        case OP_ACCESS       : return "ACCESS";
        case OP_CREATE       : return "CREATE";
        case OP_INTERRUPT    : return "INTERRUPT";
        case OP_BMAP         : return "BMAP";
        case OP_DESTROY      : return "DESTROY";
        case OP_IOCTL        : return "IOCTL";
        case OP_POLL         : return "POLL";
        case OP_NOTIFY_REPLY : return "NOTIFY_REPLY";
        case OP_BATCH_FORGET : return "BATCH_FORGET";
        case OP_FALLOCATE    : return "FALLOCATE";
        case OP_CUSE_INIT    : return "CUSE_INIT";
        }
        return NULL;
    }

};
#endif

