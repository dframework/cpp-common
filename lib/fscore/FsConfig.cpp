#include <dframework/fscore/FsConfig.h>

namespace dframework {

    Object FsConfig::SAFE;
    int    FsConfig::MAX_RECENTLY_COUNT = D_MIN_FSNODE_RECENTLY_COUNT;

    bool FsConfig::useRecently(){
        AutoLock _l(&SAFE);
        return (MAX_RECENTLY_COUNT != 0 ? true : false);
    }

    void FsConfig::setRecentlyMax(int max){
        AutoLock _l(&SAFE);
        if( max!=0 ){
            if(max < MIN_RECENTLY_COUNT)
                max = MIN_RECENTLY_COUNT;
        }
        MAX_RECENTLY_COUNT = max;
    }

    int FsConfig::getRecentlyMax(){
        AutoLock _l(&SAFE);
        return MAX_RECENTLY_COUNT;
    }

};

