#ifndef DFRAMEWORK_MONITOR_MONDISKSTATS_H
#define DFRAMEWORK_MONITOR_MONDISKSTATS_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonDiskstats : public MonBase
  {
  public:
      static const char* SAVE_FILENM;

  public:
      class Data : public Object
      {
      public:
          int      m_type;
          int      m_devtype;
          String   m_sName;
          uint64_t m_rcount;
          uint64_t m_rmerged;
          uint64_t m_rsector;
          uint64_t m_rtime;

          uint64_t m_wcount;
          uint64_t m_wmerged;
          uint64_t m_wsector;
          uint64_t m_wtime;

          uint64_t m_iocount;
          uint64_t m_iotime;
          uint64_t m_iowtime;

      public:
          Data();
          virtual ~Data();

          DFW_OPERATOR_EX_DECLARATION(Data, m_sName);
      };

  public:
      ArraySorted<Data> m_aLists;
      sp<Data> m_all;

  private:
      sp<Retval> parseLine(String& sLine);

  public:
      MonDiskstats(uint64_t sec);
      virtual ~MonDiskstats();

      virtual sp<MonBase> create(uint64_t sec);
      virtual const char* source_path();
      virtual const char* savename();
      virtual const char* rawname();
      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual void        avg(int count);
      virtual sp<Retval>  draw(int num, sp<info>&, sp<MonBase>&
                              , const char* path);

      virtual bool        getRawString(String& s, sp<MonBase>& b);
      virtual sp<MonBase> createBlank(uint64_t, sp<MonBase>&);
      virtual sp<Retval>  loadData(sp<MonBase>& out, String&);

      void depth_l(int no, sp<Data>& d, sp<Data>& c, sp<Data>& o);

      inline sp<Data> getData(int pos) { return m_aLists.get(pos); }

      inline uint64_t type()   {return (m_all.has() ? m_all->m_type : 0); }
      inline uint64_t devtype(){return (m_all.has() ? m_all->m_devtype : 0); }

      inline uint64_t rcount() {return (m_all.has() ? m_all->m_rcount:0); }
      inline uint64_t rmerged(){return (m_all.has() ? m_all->m_rmerged:0); }
      inline uint64_t rsector(){return (m_all.has() ? m_all->m_rsector:0); }
      inline uint64_t rtime()  {return (m_all.has() ? m_all->m_rtime:0); }

      inline uint64_t wcount() {return (m_all.has() ? m_all->m_wcount:0); }
      inline uint64_t wmerged(){return (m_all.has() ? m_all->m_wmerged:0); }
      inline uint64_t wsector(){return (m_all.has() ? m_all->m_wsector:0); }
      inline uint64_t wtime()  {return (m_all.has() ? m_all->m_wtime:0); }

      inline uint64_t iocount() {return (m_all.has() ? m_all->m_iocount:0); }
      inline uint64_t iotime()  {return (m_all.has() ? m_all->m_iotime:0); }
      inline uint64_t iowtime() {return (m_all.has() ? m_all->m_iowtime:0); }

  };

};

#endif /* DFRAMEWORK_MONITOR_MONDISKSTATS_H */

