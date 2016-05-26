#ifndef DFRAMEWORK_HTTPD_TYPES_H
#define DFRAMEWORK_HTTPD_TYPES_H

#include <dframework/http/HttpStatus.h>

enum dfw_httpd_errno {

    HTTPD_STATUS_500 = 50000,

};

enum dfw_httpd_client_status {

    HTTPD_CSTATUS_READY  = 0,
    HTTPD_CSTATUS_HEADER = 1,
    HTTPD_CSTATUS_BODY   = 2,

};

#endif /* DFRAMEWORK_HTTPD_TYPES_H */

