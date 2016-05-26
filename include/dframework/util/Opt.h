#ifndef DFRAMEWORK_UTIL_OPT_H
#define DFRAMEWORK_UTIL_OPT_H

#include <dframework/base/Retval.h>
#include <unistd.h>
#include <getopt.h>

#ifdef __cplusplus
namespace dframework {

    class Opt : public Object
    {
    public:
        inline Opt() {}
        inline virtual ~Opt(){}

        virtual sp<Retval> on_opt(int index, int param, const char* opt_arg)=0;

        sp<Retval> getopt_long(int argc, char** argv
                              , const char* opt_string
                              , const struct option* opt_option);

    };

};
#endif

#endif /* DFRAMEWORK_UTIL_OPT_H */

