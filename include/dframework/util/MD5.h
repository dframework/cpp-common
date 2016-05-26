#ifndef DFRAMEWORK_UTIL_MD5_H
#define DFRAMEWORK_UTIL_MD5_H


#include <dframework/base/type.h>
#include <dframework/lang/String.h>


#ifdef __cplusplus
namespace dframework {


    class MD5
    {
    private:
        static const int MD5_SIZE; /* 16 */
        static const dfw_byte_t PADDING[64];
        struct dfw_md5_ctx {
            dfw_uint_t state[4];  /** state (ABCD) */
            dfw_uint_t count[4];  /** number of bits, modulo 2^64 (lsb first) */
            dfw_byte_t buffer[64]; /** input buffer */

        };
        typedef struct dfw_md5_ctx dfw_md5_ctx;

        void encode(unsigned char *output, const dfw_uint_t *input,
                dfw_uint_t len);
        void decode(dfw_uint_t *output, const dfw_byte_t *input,
                dfw_uint_t len);
        void transform(dfw_uint_t state[4], const dfw_byte_t block[64]);

    public:
        MD5();
        virtual ~MD5();

        dfw_md5_ctx m_ctx;

        void init();
        void update(const void *vinput, size_t inputLen);
        void finalization(unsigned char *buffer);
        //String binary(const dfw_byte_t *buf, int length);
    };


};
#endif

#endif /* DFRAMEWORK_UTIL_MD5_H */


