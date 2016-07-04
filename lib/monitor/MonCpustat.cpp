#include <dframework/monitor/MonCpustat.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>
#include <dframework/util/StringArray.h>

namespace dframework {

  const char* MonCpustat::SAVE_FILENM = "cpustat";

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
      m_all = new Data();
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
      return SAVE_FILENM;
  }

  const char* MonCpustat::rawname(){
      return SAVE_FILENM;
  }

  sp<Retval> MonCpustat::readData()
  {
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
      sp<MonCpustat> ret = create(sec);

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

  void MonCpustat::avg(int count){
      if( count == 0 ) return;

      for( int k=0; k<m_aLists.size(); k++){
          sp<Data> c = m_aLists.get(k);
          c->m_total   /= count;
          c->m_user    /= count;
          c->m_nice    /= count;
          c->m_system  /= count;
          c->m_idle    /= count;
          c->m_iowait  /= count;
          c->m_irq     /= count;
          c->m_softirq /= count;
      }

      m_processes /= count;
      m_running /= count;
      m_blocked /= count;
  }

  bool MonCpustat::getRawString(String& s, sp<MonBase>& b){
      sp<MonCpustat> c = b;
      if( !c.has() ) return false;

      s = String::format("%lu", c->m_sec);
      for(int k=0; k<c->m_aLists.size(); k++){
          sp<Data> d = c->m_aLists.get(k);
          if( !d.has() ) return false;
          s.appendFmt(
               "\t|%d "
               "%lu %lu %lu %lu "
               "%lu %lu %lu %lu"
             , d->m_no
             , d->m_total, d->m_user, d->m_nice, d->m_system
             , d->m_idle, d->m_iowait, d->m_irq,  d->m_softirq
          );
      }
      s.append("\n");
      return true;
  }

  sp<MonBase> MonCpustat::createBlank(uint64_t sec, sp<MonBase>& old_)
  {
      sp<MonCpustat> old = old_;
      sp<MonCpustat> ret = create(sec);

      for( int k=0; k<old->m_aLists.size(); k++){
          sp<Data> d = new Data();
          sp<Data> o = old->m_aLists.get(k);
          d->m_no    = o->m_no;
          if( k==0 )
              ret->m_all = d;
          ret->m_aLists.insert(d);
      }

      return ret;
  }

  sp<Retval> MonCpustat::loadData(sp<MonBase>& out, String& sLine)
  {
      sp<Retval> retval;

      StringArray ar;
      if( DFW_RET(retval, ar.split(sLine, "|")) )
          return DFW_RETVAL_D(retval);
      if( !(ar.size() > 1) )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Unknown format. ground-size=%d", ar.size());

      // get seconds.
      String s_sec;
      sp<String> ar1 = ar.getString(0);
#if 0
      sp<Regexp> a = new Regexp("^([0-9]+)[\\s]+");
      if( DFW_RET(retval, a->regexp(ar1->toChars())) )
          return DFW_RETVAL_D(retval);

      s_sec.set(a->getMatch(1), a->getMatchLength(1));
      uint64_t d_sec = Long::parseLong(s_sec);
#endif

      uint64_t d_sec = Long::parseLong(ar1->toChars());
      sp<MonCpustat> dest = create(d_sec);

      // get datas
      String s_no;
      String s_total, s_user, s_nice, s_system;
      String s_idle, s_iowait, s_irq,  s_softirq;
      for(int k=1; k<ar.size(); k++){
          sp<String> arx = ar.getString(k);

          int round = 0;
          const char* v = NULL;
          const char* p = arx->toChars();
          do{
              if( *p==' ' || *p=='\t' || *p=='|' || *p=='\0'){
                  if( v ){
                      switch(round){
                      case 0 : s_no.set(v, p-v); break;
                      case 1 : s_total.set(v, p-v); break;
                      case 2 : s_user.set(v, p-v); break;
                      case 3 : s_nice.set(v, p-v); break;
                      case 4 : s_system.set(v, p-v); break;
                      case 5 : s_idle.set(v, p-v); break;
                      case 6 : s_iowait.set(v, p-v); break;
                      case 7 : s_irq.set(v, p-v); break;
                      case 8 : s_softirq.set(v, p-v); break;
                      }
                      v = NULL;
                      round++;
                  }
                  if( *p=='\0' ) break;
              }else if( !v ){
                  v = p;
              }
              p++;
          }while(true);

          if( round != 9 ){
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                         , "Unknown format %s", sLine.toChars());
          }

          sp<Data> d = new Data();
          d->m_no = Integer::parseInt(s_no);
          d->m_total = Long::parseLong(s_total);
          d->m_user = Long::parseLong(s_user);
          d->m_nice = Long::parseLong(s_nice);
          d->m_system = Long::parseLong(s_system);
          d->m_idle = Long::parseLong(s_idle);
          d->m_iowait = Long::parseLong(s_iowait);
          d->m_irq = Long::parseLong(s_irq);
          d->m_softirq = Long::parseLong(s_softirq);

          if( DFW_RET(retval, dest->m_aLists.insert(d)) )
              return DFW_RETVAL_D(retval);

          if( d->m_no==-1 ){
              dest->m_all = d;
          }
      }

      out = dest;
      return NULL;
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

