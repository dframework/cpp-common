#include <dframework/monitor/MonCpustat.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>

namespace dframework {

  MonCpustat::Data::Data(){
      m_no = 0;
      m_total = 0;
      m_user = 0;
      m_nice = 0;
      m_system = 0;
      m_idle = 0;
      m_iowait = 0;
      m_irq = 0;
      m_softirq = 0;
  }

  MonCpustat::Data::~Data(){
  }

  MonCpustat::MonCpustat(uint64_t sec)
          : MonBase(sec)
  {
      m_processes = 0;
      m_running = 0;
      m_blocked = 0;
  }

  MonCpustat::~MonCpustat(){
  }

  sp<MonBase> MonCpustat::create(uint64_t sec){
      return new MonCpustat(sec);
  }

  const char* MonCpustat::source_path(){
      return "/proc/stat";
  }

  const char* MonCpustat::savename(){
      return "cpustat";
  }

  sp<Retval> MonCpustat::readData(){
      sp<Retval> retval;

      String sContents;
      String sLine;
      if( DFW_RET(retval, File::contents(sContents, source_path())) )
          return DFW_RETVAL_D(retval);
      if( sContents.length()==0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     ,"Has not contents at %s", source_path());

      unsigned len;
      const char* lp;
      const char* p = sContents.toChars();
      do{
          if( p == NULL )
              break;
          if( (lp = strstr(p, "\n")) == NULL )
              break;
          if( (len = lp - p) == 0 )
              break;
          sLine.set(p, len);
          p += len + 1;

          if( !DFW_RET(retval, parseLine_cpus(sLine)) ){
          }else if( !DFW_RET(retval, parseLine_processes(sLine)) ){
          }else if( !DFW_RET(retval, parseLine_blocked(sLine)) ){
          }else if( !DFW_RET(retval, parseLine_running(sLine)) ){
          }
      }while(true);

      return NULL;      
  }

  sp<Retval> MonCpustat::parseLine_cpus(String& sLine){
      sp<Retval> retval;

      Regexp cpu("^(cpu)([0-9]*)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)");
      if( DFW_RET(retval, cpu.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      String s_no;
      String s_user;
      String s_nice;
      String s_system;
      String s_idle;
      String s_iowait;
      String s_irq;
      String s_softirq;

      s_no     .set(cpu.getMatch(2), cpu.getMatchLength(2));
      s_user   .set(cpu.getMatch(3), cpu.getMatchLength(3));
      s_nice   .set(cpu.getMatch(4), cpu.getMatchLength(4));
      s_system .set(cpu.getMatch(5), cpu.getMatchLength(5));
      s_idle   .set(cpu.getMatch(6), cpu.getMatchLength(6));
      s_iowait .set(cpu.getMatch(7), cpu.getMatchLength(7));
      s_irq    .set(cpu.getMatch(8), cpu.getMatchLength(8));
      s_softirq.set(cpu.getMatch(9), cpu.getMatchLength(9));

      sp<Data> d = new Data();

      d->m_user    = Long::parseLong(s_user);
      d->m_nice    = Long::parseLong(s_nice);
      d->m_system  = Long::parseLong(s_system);
      d->m_idle    = Long::parseLong(s_idle);
      d->m_iowait  = Long::parseLong(s_iowait);
      d->m_irq     = Long::parseLong(s_irq);
      d->m_softirq = Long::parseLong(s_softirq);
      d->m_total = d->m_user + d->m_nice + d->m_system + d->m_idle 
                 + d->m_iowait + d->m_irq + d->m_softirq;

      if( s_no.length() == 0 ){
          d->m_no  = -1;
          m_all = d;
      }else{
          d->m_no  = Integer::parseInt(s_no);
      }

      if( DFW_RET(retval, m_aLists.insert(d)) ){
          return DFW_RETVAL_D(retval);
      }

      return NULL;
  }

  sp<Retval> MonCpustat::parseLine_processes(String& sLine){
      sp<Retval> retval;

      Regexp cpu("^processes[\\s]+([0-9]+)");
      if( DFW_RET(retval, cpu.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);
      
      String test;
      test.set(cpu.getMatch(1), cpu.getMatchLength(1));
      if( test.length() == 0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "");

      m_processes = Long::parseLong(test);
      return NULL;
  }

  sp<Retval> MonCpustat::parseLine_blocked(String& sLine){
      sp<Retval> retval;

      Regexp cpu("^procs_blocked[\\s]+([0-9]+)");
      if( DFW_RET(retval, cpu.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);
      
      String test;
      test.set(cpu.getMatch(1), cpu.getMatchLength(1));
      if( test.length() == 0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "");

      m_blocked = Long::parseLong(test);
      return NULL;
  }

  sp<Retval> MonCpustat::parseLine_running(String& sLine){
      sp<Retval> retval;

      Regexp cpu("^procs_running[\\s]+([0-9]+)");
      if( DFW_RET(retval, cpu.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);
     
      String test;
      test.set(cpu.getMatch(1), cpu.getMatchLength(1));
      if( test.length() == 0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "");

      m_running = Long::parseLong(test);
      return NULL;
  }

  sp<MonBase> MonCpustat::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      sp<MonCpustat> old = old_;
      sp<MonCpustat> ret = new MonCpustat(sec);

      for( int k=0; k<m_aLists.size(); k++){
          sp<Data> d = new Data();
          sp<Data> c = m_aLists.get(k);
          d->m_no    = c->m_no;
          if( no==0 ){
              sp<Data> o = old->getData(k);
              d->m_total   = c->m_total   - o->m_total;
              d->m_user    = c->m_user    - o->m_user;
              d->m_nice    = c->m_nice    - o->m_nice;
              d->m_system  = c->m_system  - o->m_system;
              d->m_idle    = c->m_idle    - o->m_idle;
              d->m_iowait  = c->m_iowait  - o->m_iowait;
              d->m_irq     = c->m_irq     - o->m_irq;
              d->m_softirq = c->m_softirq - o->m_softirq;
          }else{
              d->m_total   = c->m_total;
              d->m_user    = c->m_user;
              d->m_nice    = c->m_nice;
              d->m_system  = c->m_system;
              d->m_idle    = c->m_idle;
              d->m_iowait  = c->m_iowait;
              d->m_irq     = c->m_irq;
              d->m_softirq = c->m_softirq;
          }

          if( k==0 )
              ret->m_all = d;
          ret->m_aLists.insert(d);
      }

      if( no == 0 )
          ret->m_processes = m_processes - old->m_processes;
      else
          ret->m_processes = m_processes;
      ret->m_running = m_running;
      ret->m_blocked = m_blocked;
      return ret;
  }

  void MonCpustat::plus(sp<MonBase>& old_){
      sp<MonCpustat> old = old_;

      for( int k=0; k<old->m_aLists.size(); k++){
          sp<Data> o = old->m_aLists.get(k);
          sp<Data> c = m_aLists.get(k);
          if( !c.has() ){
              c = new Data();
              c->m_no = o->m_no;
              m_aLists.insert(c);
              if( k==0 )
                  m_all = c;
          }
          c->m_total   += o->m_total;
          c->m_user    += o->m_user;
          c->m_nice    += o->m_nice;
          c->m_system  += o->m_system;
          c->m_idle    += o->m_idle;
          c->m_iowait  += o->m_iowait;
          c->m_irq     += o->m_irq;
          c->m_softirq += o->m_softirq;
      }

      m_processes += old->m_processes;
      m_running += old->m_running;
      m_blocked += old->m_blocked;
  }

  bool MonCpustat::getRawString(String& s, sp<MonBase>& b){
      sp<MonCpustat> c = b;
      if( !c.has() ) return false;

      s = String::format("%lu", c->m_sec);
      for(int k=0; k<c->m_aLists.size(); k++){
          sp<Data> d = c->m_aLists.get(k);
          if( !d.has() ) return false;
          s.appendFmt(
               "\t%u "
               "%lu %lu %lu %lu "
               "%lu %lu %lu %lu"
             , d->m_no
             , d->m_total, d->m_user, d->m_nice, d->m_system
             , d->m_idle, d->m_iowait, d->m_irq,  d->m_softirq
          );
      }
      return true;
  }

  sp<Retval> MonCpustat::draw(int num, sp<info>& info, sp<MonBase>& thiz
                      , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

