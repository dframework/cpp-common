#ifndef DFRAMEWORK_BASE_SYSTEM_H
#define DFRAMEWORK_BASE_SYSTEM_H

#include <dframework/base/Retval.h>

#ifdef __cplusplus
namespace dframework {

    class System : public Object
    {
    public:
        System();
        virtual ~System();

        static const char* encoding();

#if !defined(_WIN32)
        static sp<Retval> fork(int* pid);
#endif
        static sp<Retval> getpname(String& outname, int pid);

    };

}; // end namespae dframework
#endif // end if __cplusplus

#endif /* DFRAMEWORK_BASE_SYSTEM_H */

