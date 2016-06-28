#ifndef DFRAMEWORK_MONITOR_MONDISKSPACE_H
#define DFRAMEWORK_MONITOR_MONDISKSPACE_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonDiskspace : public MonBase
  {
  public:
      static const char* PATH;

  public:
      class Data : public Object
      {
      public:
          String   m_sDev;
          String   m_sPath;
          String   m_sOptions;

          uint64_t m_blocks;
          uint64_t m_avail;
          uint64_t m_free;
          uint64_t m_used;

      public:
          Data();
          virtual ~Data();

          DFW_OPERATOR_EX_DECLARATION(Data, m_sDev);
      };

  public:
      ArraySorted<Data> m_aLists;

  private:
      sp<Retval> parseLine(String& sLine);

  public:
      MonDiskspace(uint64_t sec);
      virtual ~MonDiskspace();

      inline virtual sp<MonBase> create(uint64_t sec){
          return new MonDiskspace(sec);
      }
      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual void        draw(int num, sp<info>&, sp<MonBase>&);


      inline sp<Data> getData(int pos) { return m_aLists.get(pos); }
  };

};

#endif /* DFRAMEWORK_MONITOR_MONDISKSPACE_H */

