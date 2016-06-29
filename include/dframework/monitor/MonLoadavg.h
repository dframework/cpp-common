#ifndef DFRAMEWORK_MONITOR_MONLOADAVG_H
#define DFRAMEWORK_MONITOR_MONLOADAVG_H

#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonLoadavg : public MonBase
  {
  public:
      uint64_t m_1;
      uint64_t m_5;
      uint64_t m_15;

  private:
      sp<Retval> parseLine_loadavg(String& sLine);

  public:
      MonLoadavg(uint64_t sec);
      virtual ~MonLoadavg();

      virtual sp<MonBase> create(uint64_t sec);
      virtual const char* source_path();
      virtual const char* savename();
      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual bool        getRawString(String& s, sp<MonBase>& b);
      virtual sp<Retval>  draw(int num, sp<info>&, sp<MonBase>&
                              , const char* path);


  };

};

#endif /* DFRAMEWORK_MONITOR_MONLOADAVG_H */

