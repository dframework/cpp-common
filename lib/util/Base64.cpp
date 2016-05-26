#include <dframework/util/Base64.h>

namespace dframework {

    const dfw_byte_t Base64::pr2six[256] = {
        /* ASCII table */
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };


    /* static */
    const char Base64::alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/=";

    Base64::Base64(){
    }

    Base64::~Base64(){
    }

    int Base64::decodeLen(const char *bufcoded){
        int nbytesdecoded;
#ifdef __APPLE__
        const unsigned char *bufin;
        size_t nprbytes;
#else
        register const unsigned char *bufin;
        register size_t nprbytes;
#endif

	bufin = (const unsigned char *) bufcoded;
	while (pr2six[*(bufin++)] <= 63);

	nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
	nbytesdecoded = (((int)nprbytes + 3) / 4) * 3;

	return nbytesdecoded + 1;
    }


    int Base64::decodeBinary(unsigned char *bufplain, const char *bufcoded)
    {
        int nbytesdecoded;
#ifdef __APPLE__
        const unsigned char *bufin;
        unsigned char *bufout;
        size_t nprbytes;
#else
        register const unsigned char *bufin;
        register unsigned char *bufout;
        register size_t nprbytes;
#endif
        
        bufin = (const dfw_byte_t*) bufcoded;
        while (pr2six[*(bufin++)] <= 63);
        nprbytes = (bufin - (const dfw_byte_t*) bufcoded) - 1;
        nbytesdecoded = (((int)nprbytes + 3) / 4) * 3;

        bufout = (dfw_byte_t*) bufplain;
        bufin = (const dfw_byte_t*) bufcoded;

        while (nprbytes > 4) {
            *(bufout++) =
                (dfw_byte_t)(pr2six[*bufin]<<2 | pr2six[bufin[1]]>>4);
            *(bufout++) =
                (dfw_byte_t)(pr2six[bufin[1]]<<4|pr2six[bufin[2]]>>2);
            *(bufout++) =
                (dfw_byte_t)(pr2six[bufin[2]]<<6|pr2six[bufin[3]]);
            bufin += 4;
            nprbytes -= 4;
	}

        /*Note: (nprbytes==1) would be an error, so just ingore that case*/
        if (nprbytes > 1) {
            *(bufout++) =
                (dfw_byte_t)(pr2six[*bufin]<<2|pr2six[bufin[1]]>>4);
        }
        if (nprbytes > 2) {
            *(bufout++) =
                (dfw_byte_t)(pr2six[bufin[1]]<<4|pr2six[bufin[2]]>>2);
        }
        if (nprbytes > 3) {
            *(bufout++) =
                (dfw_byte_t)(pr2six[bufin[2]]<<6|pr2six[bufin[3]]);
        }

        nbytesdecoded -= (4 - (int)nprbytes) & 3;
        return nbytesdecoded;
    }

    String Base64::decode(const dfw_byte_t *buf)
    {
        String ret;
	int len = decodeLen((char*)buf); 
        if(len>0){
            dfw_byte_t *result = (dfw_byte_t *)malloc(len);
            DFW_ABORT(!result);
	    len = decodeBinary(result, (char*)buf);
            ret.set((char *)result, len);
            DFW_FREE(result);
        }
	return ret;
    }

    String Base64::decode(const char *buf)
    {
        return decode((dfw_byte_t*)buf);
    }

    String Base64::encode(const dfw_byte_t *text, int inlen)
    {
        /* The tricky thing about this is doing the padding at the end,
         * doing the bit manipulation requires a bit of concentration only */
        char *buffer, *point;
        int outlen;

        /* Use 'buffer' to store the output. Work out how big it should be...
         * This must be a multiple of 4 bytes */

        outlen = (inlen*4)/3;
        if ((inlen % 3) > 0) /* got to pad */
            outlen += 4 - (inlen % 3);

        buffer = (char *)malloc(outlen + 1); /* +1 for the \0 */

        /* now do the main stage of conversion, 3 bytes at a time,
         * leave the trailing bytes (if there are any) for later */

        for(point=buffer; inlen>=3; inlen-=3, text+=3) {
            *(point++) = alphabet[ (*text)>>2 ];
            *(point++) = alphabet[ ((*text)<<4 & 0x30) | (*(text+1))>>4 ];
            *(point++) = alphabet[ ((*(text+1))<<2 & 0x3c) | (*(text+2))>>6 ];
            *(point++) = alphabet[ (*(text+2)) & 0x3f ];
        }

        /* Now deal with the trailing bytes */
        if (inlen > 0) {
            /* We always have one trailing byte */
            *(point++) = alphabet[ (*text)>>2 ];
            *(point++) = alphabet[ (((*text)<<4 & 0x30) |
                (inlen==2?(*(text+1))>>4:0)) ];
            *(point++) = (inlen==1?'=':alphabet[ (*(text+1))<<2 & 0x3c ]);
            *(point++) = '=';
        }

        /* Null-terminate */
        *point = '\0';
        String ret = buffer;
        return ret;
    }

    String Base64::encode(const char *text, int inlen){
        return encode((const dfw_byte_t *)text, inlen);
    }

};

