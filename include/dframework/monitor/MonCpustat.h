#ifndef DFRAMEWORK_MONITOR_MONCPUSTAT_H
#define DFRAMEWORK_MONITOR_MONCPUSTAT_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonCpustat : public MonBase
  {
  public:
      static const char* SAVE_FILENM;

  public:
      class Data : public Object
      {
      public:
          int      m_no;
          uint64_t m_total;
          uint64_t m_user;
          uint64_t m_nice;
          uint64_t m_system;
          uint64_t m_idle;
          uint64_t m_iowait;
          uint64_t m_irq;
          uint64_t m_softirq;

      public:
          Data();
          virtual ~Data();

          DFW_OPERATOR_EX_DECLARATION(Data, m_no);
      };

  public:
      ArraySorted<Data> m_aLists;
      sp<Data> m_all;
      unsigned m_processes;
      unsigned m_running;
      unsigned m_blocked;

  private:
      sp<Retval> parseLine_cpus(String& sLine);
      sp<Retval> parseLine_processes(String& sLine);
      sp<Retval> parseLine_blocked(String& sLine);
      sp<Retval> parseLine_running(String& sLine);

  public:
      MonCpustat(uint64_t sec);
      virtual ~MonCpustat();

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

      inline sp<Data> getData(int pos) { return m_aLists.get(pos); }
      inline uint64_t total(){ return  (m_all.has() ? m_all->m_total: 0); }
      inline uint64_t user() { return  (m_all.has() ? m_all->m_user : 0); }
      inline uint64_t nice() { return  (m_all.has() ? m_all->m_nice : 0); }
      inline uint64_t system(){return  (m_all.has() ? m_all->m_system:0); }
      inline uint64_t idle() { return  (m_all.has() ? m_all->m_idle : 0); }
      inline uint64_t iowait(){return  (m_all.has() ? m_all->m_iowait:0); }
      inline uint64_t irq()  { return  (m_all.has() ? m_all->m_irq : 0); }
      inline uint64_t softirq(){return (m_all.has() ? m_all->m_softirq:0);}

  };

};

#endif /* DFRAMEWORK_MONITOR_MONCPUSTAT_H */

