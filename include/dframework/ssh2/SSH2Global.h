#ifndef DFRAMEWORK_LIB_SSH2GLOBAL_H
#define DFRAMEWORK_LIB_SSH2GLOBAL_H

#include <dframework/base/Retval.h>

#ifdef __cplusplus
namespace dframework {

    class SSH2Global : public Object
    {
    private:
        static Object         m_ssh2_safe;
        static sp<SSH2Global> m_ssh2_global;

        SSH2Global();
        sp<Retval> ready();

    public:
        virtual ~SSH2Global();

        static sp<Retval> instance(sp<SSH2Global>& out);

    };

};
#endif

#endif /* DFRAMEWORK_LIB_SSH2GLOBAL_H */

