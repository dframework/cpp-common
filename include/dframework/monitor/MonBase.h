#ifndef DFRAMEWORK_MONITOR_MONBASE_H
#define DFRAMEWORK_MONITOR_MONBASE_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>

namespace dframework {

  class MonBase : public Object
  {
  public:
      class info : public Object
      {
      public:
          uint64_t    m_sec;
          sp<MonBase> m_last;
          sp<MonBase> m_total;
          ArraySorted<MonBase> m_aLists;

      public:
          info();
          virtual ~info();

          DFW_OPERATOR_EX_DECLARATION(info, m_sec);
      };

      class group : public Object
      {
      public:
          sp<MonBase> m_base;
          Array<info> m_infos;

      public:
          group();
          virtual ~group();
      };

  public:
      uint64_t m_sec;

  public:
      MonBase(uint64_t sec);
      virtual ~MonBase();

      virtual sp<MonBase> create(uint64_t sec) = 0;
      virtual sp<Retval>  readData() = 0;
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old) = 0;
      virtual void        plus(sp<MonBase>& old) = 0;
      virtual void        draw(int num, sp<info>&, sp<MonBase>&) = 0;

      DFW_OPERATOR_EX_DECLARATION(MonBase, m_sec);
  };

};

#endif /* DFRAMEWORK_MONITOR_MONBASE_H */

