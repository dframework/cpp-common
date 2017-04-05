#ifndef DFRAMEWORK_MONITOR_MONITOR_H
#define DFRAMEWORK_MONITOR_MONITOR_H

#include <dframework/base/Thread.h>
#include <dframework/util/Array.h>
#include <dframework/monitor/MonBase.h>

namespace dframework {

  class Monitor : public Thread
  {
  public:
      static const unsigned SECONDS_NUM = 5;

  private:
      unsigned MAX_ROWS;
      unsigned SECONDS[SECONDS_NUM+1];
      String   m_sPath;
      Array<MonBase::group> m_aDevices;

  private:
      sp<Retval> run_s_l(sp<MonBase::group>&, uint64_t, int, sp<MonBase>&);
      void run_s(uint64_t sec);

  public:
      Monitor(const char* savepath, unsigned max);
      virtual ~Monitor();

      virtual sp<Retval> ready();
      virtual void run();

      void setSecondsNum(unsigned pos, unsigned seconds);
      unsigned getSecondsNum(unsigned pos){
          return (unsigned)((pos<SECONDS_NUM) ? SECONDS[pos] : 0);
      }

      void setPath(const char* path) { m_sPath = path; }
      String getPath() { return m_sPath; }

      sp<Retval> addDevice(sp<MonBase>& base);

  };

};

#endif /* DFRAMEWORK_MONITOR_MONITOR_H */

