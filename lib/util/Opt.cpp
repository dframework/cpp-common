#include <dframework/util/Opt.h>

namespace dframework {

    sp<Retval> Opt::getopt_long(int argc, char** argv
                        , const char* opt_string
                        , const struct option* opt_option)
    {
        sp<Retval> retval;
        int param;

        while( true )
        {
            int option_index = 0;
            if( -1==(param = ::getopt_long(argc, argv, opt_string
                              , opt_option, &option_index)) ){
                return NULL;
            }
            if( DFW_RET(retval, on_opt(option_index, param, optarg)) )
                return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

};

