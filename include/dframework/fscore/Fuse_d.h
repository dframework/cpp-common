#ifndef DFRAMEWORK_FSCORE_FUSE_D_H
#define DFRAMEWORK_FSCORE_FUSE_D_H

#ifndef _WIN32
#include <dframework/base/Retval.h>
#include <stdint.h>
#include <sys/stat.h>

extern "C" {
#include <dframework/fscore/fuse_kernel.h>
};

#define FUSE_UNKNOWN_INO 0xffffffff

/**
 * Capability bits for 'fuse_conn_info.capable' and 'fuse_conn_info.want'
 *
 * FUSE_CAP_ASYNC_READ: filesystem supports asynchronous read requests
 * FUSE_CAP_POSIX_LOCKS: filesystem supports "remote" locking
 * FUSE_CAP_ATOMIC_O_TRUNC: filesystem handles the O_TRUNC open flag
 * FUSE_CAP_EXPORT_SUPPORT: filesystem handles lookups of "." and ".."
 * FUSE_CAP_BIG_WRITES: filesystem can handle write size larger than 4kB
 * FUSE_CAP_DONT_MASK: don't apply umask to file mode on create operations
 * FUSE_CAP_SPLICE_WRITE: ability to use splice() to write to the fuse device
 * FUSE_CAP_SPLICE_MOVE: ability to move data to the fuse device with splice()
 * FUSE_CAP_SPLICE_READ: ability to use splice() to read from the fuse device
 * FUSE_CAP_IOCTL_DIR: ioctl support on directories
 */
#define FUSE_CAP_ASYNC_READ     (1 << 0)
#define FUSE_CAP_POSIX_LOCKS    (1 << 1)
#define FUSE_CAP_ATOMIC_O_TRUNC (1 << 3)
#define FUSE_CAP_EXPORT_SUPPORT (1 << 4)
#define FUSE_CAP_BIG_WRITES     (1 << 5)
#define FUSE_CAP_DONT_MASK      (1 << 6)
#define FUSE_CAP_SPLICE_WRITE   (1 << 7)
#define FUSE_CAP_SPLICE_MOVE    (1 << 8)
#define FUSE_CAP_SPLICE_READ    (1 << 9)
#define FUSE_CAP_FLOCK_LOCKS    (1 << 10)
#define FUSE_CAP_IOCTL_DIR      (1 << 11)

        /**
        This is struct fuse_conn_info at fuse_common.h in fuse-2.9.4
        */
        struct conn_info {
            unsigned proto_major;
            unsigned proto_minor;
            unsigned async_read;
            unsigned max_write;
            unsigned max_readahead;
            unsigned capable;
            unsigned want;
            unsigned max_background;
            unsigned congestion_threshold;
            unsigned reserved[23];
        };

    /**
     * Information about open files
     *
     * Changed in version 2.5
     */
    struct fuse_file_info {
        /** Open flags.  Available in open() and release() */
        int flags;

        /** Old file handle, don't use */
        unsigned long fh_old;

        /** In case of a write operation indicates if this was caused by a
            writepage */
        int writepage;

        /** Can be filled in by open, to use direct I/O on this file.
            Introduced in version 2.4 */
        unsigned int direct_io : 1;

        /** Can be filled in by open, to indicate, that cached file data
            need not be invalidated.  Introduced in version 2.4 */
        unsigned int keep_cache : 1;

        /** Indicates a flush operation.  Set in flush operation, also
            maybe set in highlevel lock operation and lowlevel release
            operation.  Introduced in version 2.6 */
        unsigned int flush : 1;

        /** Can be filled in by open, to indicate that the file is not
            seekable.  Introduced in version 2.8 */
        unsigned int nonseekable : 1;

        /* Indicates that flock locks for this file should be
           released.  If set, lock_owner shall contain a valid value.
           May only be set in ->release().  Introduced in version
           2.9 */
        unsigned int flock_release : 1;

        /** Padding.  Do not use*/
        unsigned int padding : 27;

        /** File handle.  May be filled in by filesystem in open().
            Available in all other file operations */
        uint64_t fh;

        /** Lock owner id.  Available in locking operations and flush */
        uint64_t lock_owner;
    };

    typedef unsigned long fuse_ino_t;

    struct fuse_entry_param {
        /** Unique inode number
         *
         * In lookup, zero means negative entry (from version 2.5)
         * Returning ENOENT also means negative entry, but by setting zero
         * ino the kernel may cache negative entries for entry_timeout
         * seconds.
         */
        fuse_ino_t ino;

        /** Generation number for this entry.
         *
         * If the file system will be exported over NFS, the
         * ino/generation pairs need to be unique over the file
         * system's lifetime (rather than just the mount time). So if
         * the file system reuses an inode after it has been deleted,
         * it must assign a new, previously unused generation number
         * to the inode at the same time.
         *
         * The generation must be non-zero, otherwise FUSE will treat
         * it as an error.
         *
         */
        unsigned long generation;

        /** Inode attributes.
         *
         * Even if attr_timeout == 0, attr must be correct. For example,
         * for open(), FUSE uses attr.st_size from lookup() to determine
         * how many bytes to request. If this value is not correct,
         * incorrect data will be returned.
         */
        struct stat attr;

        /** Validity timeout (in seconds) for the attributes */
        double attr_timeout;

        /** Validity timeout (in seconds) for the name */
        double entry_timeout;
    };


#ifdef __cplusplus
namespace dframework {

    class FUSE {
    public:
        static const uint32_t OP_MAX           = 4096;

        static const uint32_t OP_LOOKUP        = 1;
        static const uint32_t OP_FORGET        = 2;  /* no reply */
        static const uint32_t OP_GETATTR       = 3;
        static const uint32_t OP_SETATTR       = 4;
        static const uint32_t OP_READLINK      = 5;
        static const uint32_t OP_SYMLINK       = 6;
        static const uint32_t OP_MKNOD         = 8;
        static const uint32_t OP_MKDIR         = 9;
        static const uint32_t OP_UNLINK        = 10;
        static const uint32_t OP_RMDIR         = 11;
        static const uint32_t OP_RENAME        = 12;
        static const uint32_t OP_LINK          = 13;
        static const uint32_t OP_OPEN          = 14;
        static const uint32_t OP_READ          = 15;
        static const uint32_t OP_WRITE         = 16;
        static const uint32_t OP_STATFS        = 17;
        static const uint32_t OP_RELEASE       = 18;
        static const uint32_t OP_FSYNC         = 20;
        static const uint32_t OP_SETXATTR      = 21;
        static const uint32_t OP_GETXATTR      = 22;
        static const uint32_t OP_LISTXATTR     = 23;
        static const uint32_t OP_REMOVEXATTR   = 24;
        static const uint32_t OP_FLUSH         = 25;
        static const uint32_t OP_INIT          = 26;
        static const uint32_t OP_OPENDIR       = 27;
        static const uint32_t OP_READDIR       = 28;
        static const uint32_t OP_RELEASEDIR    = 29;
        static const uint32_t OP_FSYNCDIR      = 30;
        static const uint32_t OP_GETLK         = 31;
        static const uint32_t OP_SETLK         = 32;
        static const uint32_t OP_SETLKW        = 33;
        static const uint32_t OP_ACCESS        = 34;
        static const uint32_t OP_CREATE        = 35;
        static const uint32_t OP_INTERRUPT     = 36;
        static const uint32_t OP_BMAP          = 37;
        static const uint32_t OP_DESTROY       = 38;
        static const uint32_t OP_IOCTL         = 39;
        static const uint32_t OP_POLL          = 40;
        static const uint32_t OP_NOTIFY_REPLY  = 41;
        static const uint32_t OP_BATCH_FORGET  = 42;
        static const uint32_t OP_FALLOCATE     = 43;

        /* CUSE specific operations */
        static const uint32_t OP_CUSE_INIT     = 4096;

    public:
        static const char* getOpcodeName(uint32_t opcode);
    };


}; // end namespace dframework
#endif // end if cplusplus

#endif

#endif /* DFRAMEWORK_FSCORE_FUSE_D_H */

