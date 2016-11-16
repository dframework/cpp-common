#include <dframework/httpd/HttpdUtil.h>
#include <dframework/log/Logger.h>
#include <ctype.h>

namespace dframework {

    HttpdUtil::HttpdUtil(){
    }

    HttpdUtil::~HttpdUtil(){
    }

    DFW_STATIC
    int HttpdUtil::toLower(int c){
	    if (c >= 'A' && c <= 'Z')
		    c ^= 0x20;
	    return c;
    }

    DFW_STATIC
    sp<Retval> HttpdUtil::urldecode(String& out, const char *url){
        int s = 0, d = 0, url_len = 0;
        char c;
        char *dest = NULL;
	if (!url)
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Not found url parameter");
	url_len = ::strlen(url) + 1;
	dest = (char*)::malloc(url_len);
	if (!dest)
                return DFW_RETVAL_NEW(DFW_E_NOMEM, ENOMEM);
	while (s < url_len) {
		c = url[s++];
		if (c == '%' && s + 2 < url_len) {
			char c2 = url[s++];
			char c3 = url[s++];
			if (::isxdigit(c2) && ::isxdigit(c3)) {
				c2 = HttpdUtil::toLower(c2);
				c3 = HttpdUtil::toLower(c3);
				if (c2 <= '9')
					c2 = c2 - '0';
				else
					c2 = c2 - 'a' + 10;
				if (c3 <= '9')
					c3 = c3 - '0';
				else
					c3 = c3 - 'a' + 10;
				dest[d++] = 16 * c2 + c3;
			} else { /* %zz or something other invalid */
				dest[d++] = c;
				dest[d++] = c2;
				dest[d++] = c3;
			}
		} else if (c == '+') {
			dest[d++] = ' ';
		} else {
			dest[d++] = c;
		}
	}
        out = dest;
        DFW_FREE(dest);
	return NULL;
    }

#define DFW_HTTPD_ISSPACE(c)  (::isspace(((unsigned char)(c))))
#define DFW_HTTPD_ISUPPER(c)  (::isupper(((unsigned char)(c))))
#define DFW_HTTPD_ISLOWER(c)  (::islower(((unsigned char)(c))))
#define DFW_HTTPD_ISDIGIT(c)  (::isdigit(((unsigned char)(c))))
#define DFW_HTTPD_ISXDIGIT(c) (::isxdigit(((unsigned char)(c))))

    /*
     * Compare a string to a mask
     * Mask characters (arbitrary maximum is 256 characters, just in case):
     *   @ - uppercase letter
     *   $ - lowercase letter
     *   & - hex digit
     *   # - digit
     *   ~ - digit or space
     *   * - swallow remaining characters 
     *  <x> - exact match for any other character
     */
    DFW_STATIC
    bool HttpdUtil::isDateMask(const char *data, const char *mask){
        int i;
        char d;
        for(i = 0; i < 256; i++) {
            d = data[i];
            switch (mask[i]) {
            case '\0':
                return (d == '\0');
            case '*':
                return true;
            case '@':
                if (!DFW_HTTPD_ISUPPER(d)) return false;
                break;
            case '$':
                if (!DFW_HTTPD_ISLOWER(d)) return false;
                break;
            case '#':
                if (!DFW_HTTPD_ISDIGIT(d)) return false;
                break;
            case '&':
                if (!DFW_HTTPD_ISXDIGIT(d)) return false;
                break;
            case '~':
                if ((d != ' ') && !DFW_HTTPD_ISDIGIT(d)) return false;
                break;
            default:
                if (mask[i] != d) return false;
                break;
            }
        }
        /* We only get here if mask is corrupted (exceeds 256) */
        return false;
    }

#define DFW_HTTPD_TIME_SEC 1000000

    DFW_STATIC
    sp<Retval> HttpdUtil::expget(time_t *t, Time *xt){
        int year = xt->getYear();
        int days;
        static const int dayoffset[12] =
            {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};

        /* shift new year to 1st March in order to make leap year calc easy */
        if(xt->getMonth() < 2)
            year--;

        /* Find number of days since 1st March 1900
           (in the Gregorian calendar). */
        days = year * 365 + year / 4 - year / 100 + (year / 100 + 3) / 4;
        days += dayoffset[xt->getMonth()] + xt->getDay() - 1;
        days -= 25508; /* 1 jan 1970 is 25508 days since 1 mar 1900 */

        days = (((((days * 24) + xt->getHour() - xt->getGmtOffsetHour())*60)
               + xt->getMinute() - xt->getGmtOffsetMin())*60)
               + xt->getSecond();
        if(days<0){
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "days<0, days=%d", days);
        }
        *t = days;
        //*t = (days*DFW_HTTPD_TIME_SEC);// + xt->tm_usec;
        return NULL;
    }

    DFW_STATIC
    sp<Retval> HttpdUtil::parseToTime(time_t* out, const char* date){
        sp<Retval> retval;

        Time ds; //rahttp_time_exp_t ds;

        int mint, mon;
        const char *monstr, *timstr;
        static const int months[12] = {
            ('J' << 16) | ('a' << 8) | 'n', ('F' << 16) | ('e' << 8) | 'b',
            ('M' << 16) | ('a' << 8) | 'r', ('A' << 16) | ('p' << 8) | 'r',
            ('M' << 16) | ('a' << 8) | 'y', ('J' << 16) | ('u' << 8) | 'n',
            ('J' << 16) | ('u' << 8) | 'l', ('A' << 16) | ('u' << 8) | 'g',
            ('S' << 16) | ('e' << 8) | 'p', ('O' << 16) | ('c' << 8) | 't',
            ('N' << 16) | ('o' << 8) | 'v', ('D' << 16) | ('e' << 8) | 'c'};

        if(!date)
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "arg(date) is null.");

        /* Find first non-whitespace */
        while(*date && DFW_HTTPD_ISSPACE(*date)) ++date;
        if(*date == '\0')
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "Unknown arg(date=%s)", date);

        /* Find space after weekday */
        if((date = strchr(date, ' ')) == NULL)
            return DFW_RETVAL_NEW_MSG(DFW_E_INVAL, EINVAL
                       , "Unknown arg(date=%s)", date);
        ++date;

        /* start of the actual date information for all 4 formats. */
        if(isDateMask(date, "## @$$ #### ##:##:## *")) {
            /* RFC 1123 format with two days */
            ds.setYear((((date[7] -'0')*10) + (date[8]-'0')-19) * 100);
            if(ds.getYear() < 0)
                return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);
            ds.setYear(ds.getYear()+(((date[9]-'0')*10) + (date[10]-'0')));
            ds.setDay(((date[0] - '0') * 10) + (date[1] - '0'));
            monstr = date + 3;
            timstr = date + 12;
        }else if(isDateMask(date, "##-@$$-## ##:##:## *")) {
            /* RFC 850 format */
            ds.setYear(((date[7]-'0')*10) + (date[8]-'0'));
            if(ds.getYear() < 70)
                ds.setYear(ds.getYear()+100);
            ds.setDay(((date[0]-'0')*10) + (date[1]-'0'));
            monstr = date + 3;
            timstr = date + 10;
        }else if(isDateMask(date, "@$$ ~# ##:##:## ####*")) {
            /* asctime format */
            ds.setYear(((date[16]-'0')*10 + (date[17]-'0')-19)*100);
            if(ds.getYear() < 0)
                return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);
            ds.setYear(ds.getYear()+((date[18]-'0')*10) + (date[19]-'0'));
            if(date[4] == ' ')
                ds.setDay(0);
            else
                ds.setDay((date[4] - '0') * 10);
            ds.setDay(ds.getDay()+(date[5] - '0'));
            monstr = date;
            timstr = date + 7;
        }else if(isDateMask(date, "# @$$ #### ##:##:## *")) {
            /* RFC 1123 format with one day */
            ds.setYear(((date[6] - '0') * 10 + (date[7] - '0') - 19) * 100);
            if(ds.getYear() < 0)
                return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);
            ds.setYear(ds.getYear()+((date[8]-'0')*10) + (date[9]-'0'));
            ds.setDay(date[0] - '0');
            monstr = date + 2;
            timstr = date + 11;
        } else
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);

        if(ds.getDay() <= 0 || ds.getDay() > 31)
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);

        ds.setHour(((timstr[0] - '0') * 10) + (timstr[1] - '0'));
        ds.setMinute(((timstr[3] - '0') * 10) + (timstr[4] - '0'));
        ds.setSecond(((timstr[6] - '0') * 10) + (timstr[7] - '0'));

        if((ds.getHour()>23) || (ds.getMinute()>59) || (ds.getSecond()>61))
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);

        mint = (monstr[0] << 16) | (monstr[1] << 8) | monstr[2];
        for(mon = 0; mon < 12; mon++){
            if(mint == months[mon])
                break;
        }

        if(mon == 12)
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);
        if((ds.getDay()==31) && (mon==3 || mon==5 || mon==8 || mon==10))
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);

        /* February gets special check for leapyear */
        if ((mon == 1) &&
                    ((ds.getDay() > 29) ||
                    ((ds.getDay() == 29)
                    && ((ds.getYear() & 3)
                              || (((ds.getYear() % 100) == 0)
                              && (((ds.getYear() % 400) != 100))))))){
            return DFW_RETVAL_NEW(DFW_ERROR, EINVAL);
        }

        ds.setMonth(mon);

        /* ap_mplode_time uses tm_usec and tm_gmtoff fields, but they haven't 
         * been set yet. 
         * It should be safe to just zero out these values.
         * tm_usec is the number of microseconds into the second.  HTTP only
         * cares about second granularity.
         * tm_gmtoff is the number of seconds off of GMT the time is.  By
         * definition all times going through this function are in GMT, so this
         * is zero. 
         */
        //ds.tm_usec = 0;

        time_t result;
        if(DFW_RET(retval, expget(&result, &ds)))
            return DFW_RETVAL_D(retval);
        *out = result;
        return NULL;
    }

    sp<Retval> HttpdUtil::checkRangeBytes(
                  HttpRequest* request
                , String& sRange, String& sAcceptRange
                , String& sStart, String& sEnd, int* minus)
                //, char** pStart, char** pEnd, int* minus)
    {
        sp<Retval> retval;

        HttpHeader* oHeader = (HttpHeader*)request;
        sp<NamedValue> oIfRange = oHeader->getHeader("If-Range");
        sp<NamedValue> oRange = oHeader->getHeader("Range");
        sp<NamedValue> oAcceptRange = oHeader->getHeader("Accept-Range");

        if( !oRange.has() ) { 
            *minus = 0;
            return NULL; 
        }

        if( oIfRange.has() && !request->IsIfRange() ){
            // Modified 
            *minus = 0;
            return NULL;
        }

        sRange = oRange->m_sValue;

        if( oAcceptRange.has() )
            sAcceptRange = oAcceptRange->m_sValue;

        if( sRange.empty() ){
            *minus = 0;
            return NULL; 
        }

        if( !sAcceptRange.empty() ){
            if(!sAcceptRange.equals("bytes")&&!sAcceptRange.equals("none")){
                return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                           , "Not support Accept-Range: %s"
                           , sAcceptRange.toChars());
            }
        }

        //*pStart = *pEnd = NULL;
        const char* pRange = sRange.toChars();
        const char* p2 = NULL;
        const char* p = ::strcasestr(pRange, "bytes");
        if(p!=pRange)
            return DFW_RETVAL_NEW(DFW_ERROR, 0);

        p = pRange + ::strlen("bytes") + 1;
        if('\0'==*p){
            // bytes=
            return DFW_RETVAL_NEW(DFW_ERROR, 0);
        }
        if('-'==*p){
            // bytes=-
            *minus = 2;
            sEnd.set(p+1);
            //*pEnd = (char*)(p + 1);
        }else if(::strcmp(p, "0-0,-1")==0){
            // This is multipart.
            *minus = 3;
        }else{
            p2 = ::strstr(p+1,"-");
            if(NULL==p2){
                return DFW_RETVAL_NEW(DFW_ERROR, 0);
            }
            p2++;
            if( '\0'==*p2 ) {
                // bytes=num-
                *minus = 4;
                sStart.set(p, p2-p-1);
                //*pStart = (char*)p;
            }else{
                // bytes=num-num
                *minus = 1;
                sStart.set(p, p2-p-1);
                sEnd.set(p2);
                //*pStart = (char*)p;
                //*pEnd = (char*)p2;
            }
        }
        return NULL;
    }

};

