#ifndef DFRAMEWORK_UTIL_BASE64_H
#define DFRAMEWORK_UTIL_BASE64_H


#include <dframework/base/type.h>
#include <dframework/lang/String.h>


#ifdef __cplusplus
namespace dframework {

    class Base64
    {
    private:
        static const dfw_byte_t pr2six[256];
        static const char alphabet[];

        int decodeLen(const char *bufcoded);
        int decodeBinary(unsigned char *bufplain, const char *bufcoded);

    public:
        Base64();
        virtual ~Base64();

        String decode(const char *buf);
        String decode(const dfw_byte_t *buf);
        String encode(const char *text, int inlen);
        String encode(const dfw_byte_t *text, int inlen);

    };

};
#endif

#endif /* DFRAMEWORK_UTIL_BASE64_H */

