#ifndef DFRAMEWORK_MONITOR_MONITOR_H
#define DFRAMEWORK_MONITOR_MONITOR_H

#include <dframework/base/Thread.h>
#include <dframework/util/Array.h>
#include <dframework/monitor/MonBase.h>

namespace dframework {

  class Monitor : public Thread
  {
  public:
      const static unsigned MAX_ROWS  = 400;

  private:
      static unsigned SECONDS[];

  private:
      Array<MonBase::group> m_aDevices;

  private:
      bool run_s_l(sp<MonBase::group>&, uint64_t, int, sp<MonBase>&);
      void run_s(uint64_t sec);

  public:
      Monitor();
      virtual ~Monitor();

      virtual void run();

      sp<Retval> addDevice(sp<MonBase>& base);

  };

};

#endif /* DFRAMEWORK_MONITOR_MONITOR_H */

