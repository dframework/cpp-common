#ifndef DFRAMEWORK_FSCORE_FSCONFIG_H
#define DFRAMEWORK_FSCORE_FSCONFIG_H

#include <dframework/base/Retval.h>

#define D_MIN_FSNODE_RECENTLY_COUNT 10

#ifdef __cplusplus
namespace dframework {

    class FsConfig : public Object
    {
    private:
        static    Object SAFE;
        static       int MAX_RECENTLY_COUNT;
        static const int MIN_RECENTLY_COUNT = D_MIN_FSNODE_RECENTLY_COUNT;

    public:
        static bool useRecently();
        static void setRecentlyMax(int max);
        static int  getRecentlyMax();

    };

};
#endif

#endif /* DFRAMEWORK_FSCORE_FSCONFIG_H */

