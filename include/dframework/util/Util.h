#ifndef DFRAMEWORK_UTIL_UTIL_H
#define DFRAMEWORK_UTIL_UTIL_H

#include <dframework/base/Retval.h>

namespace dframework {

  class Util : public Object
  {
  public:
      static String SizeToString(uint64_t size);
      inline static float SizeToString_l(uint64_t size, uint64_t l){
        return ((float)size / (float)l);
      }


  };

};


#endif /* DFRAMEWORK_UTIL_UTIL_H */

