#include <dframework/monitor/Monitor.h>
#include <dframework/util/Time.h>

namespace dframework {


  Monitor::Monitor(const char* savepath, unsigned max_rows){
      m_sPath = savepath;
      MAX_ROWS = max_rows;
      setSecondsNum(0,    5);
      setSecondsNum(1,  240);
      setSecondsNum(2, 1800);
      setSecondsNum(3, 7200);
      setSecondsNum(4,    0);
      setSecondsNum(5,    0);
  }

  Monitor::~Monitor(){
  }

  void Monitor::setSecondsNum(unsigned pos, unsigned seconds){
      if( pos < SECONDS_NUM )
          SECONDS[pos] = seconds;
  }

  sp<Retval> Monitor::addDevice(sp<MonBase>& base){
      sp<Retval> retval;

      sp<MonBase::group> grp = new MonBase::group();
      grp->m_base = base;

      unsigned m = 0;
      for(int k=0; ;k++){
          if( SECONDS[k] == 0 )
              break;
          if( SECONDS[k] < m )
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                         , "Bad seconds. (%u < %u)", SECONDS[k], m);
          m = k;

          sp<MonBase::info> inf = new MonBase::info();
          inf->m_sec = SECONDS[k];
          if( DFW_RET(retval, grp->m_infos.insert(inf)) )
              return DFW_RETVAL_D(retval);
      }

      if( DFW_RET(retval, m_aDevices.insert(grp)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

  sp<Retval> Monitor::ready(){
      sp<Retval> retval;

      for(unsigned sn=0; sn<SECONDS_NUM; sn++){
          unsigned seconds = SECONDS[sn];
          for(int k=0; k<m_aDevices.size(); k++){
              sp<MonBase::group> grp = m_aDevices.get(k);
              if( !grp.has() ) continue;

              sp<MonBase::info> inf = grp->m_infos.get(sn);
              if( !inf.has() ){
                  inf = new MonBase::info();
                  inf->m_sec = seconds;
                  if( DFW_RET(retval, grp->m_infos.insert(inf)) )
                      return DFW_RETVAL_D(retval);
              }
              if( DFW_RET(retval, grp->m_base->loadRawData(
                                      seconds, inf, m_sPath, MAX_ROWS)) ){
printf("%s\n", retval->dump().toChars());
                  //return DFW_RETVAL_D(retval);
              }else{
              }
          }
      } // end for[int sn=0; sn<SECONDS_NUM; sn++]

      return NULL;
  }

  void Monitor::run()
  {
      sp<Retval> retval;
      uint64_t   past = 0;

      do{
          dfw_time_t ctime = Time::currentTimeMillis();
          uint64_t csec = ctime / 1000;
          if( ((csec % SECONDS[0]) == 0) && (past<csec) ){
              past = csec;
              run_s(csec);
          }
          usleep(200000);
      }while(true);
  }

  void Monitor::run_s(uint64_t sec){
      sp<Retval> retval;

      for(int k=0; k<m_aDevices.size(); k++){
          sp<MonBase::group> grp = m_aDevices.get(k);
          if( !grp.has() ) continue;

          sp<MonBase> device = grp->m_base->create(sec);
          if( DFW_RET(retval, device->readData()) ){
              DFW_RETVAL_D(retval);
          }else if( DFW_RET(retval, run_s_l(grp, sec, 0, device)) ){
              DFW_RETVAL_D(retval);
          }

          if( retval.has() ){
              // FIXME:
printf("retval=%s\n", retval->dump().toChars());
          }
      }
  }

  sp<Retval> Monitor::run_s_l(sp<MonBase::group>& grp, uint64_t sec, int no
                       , sp<MonBase>& device)
  {
      sp<Retval> retval;
      if( SECONDS[no] == 0 )
          return DFW_RETVAL_NEW(DFW_OK, 0);

      int      next = SECONDS[no+1];
      uint64_t range = SECONDS[no] * MAX_ROWS;
      sp<MonBase::info> inf = grp->m_infos.get(no);

      for(int k=0; k<inf->m_aLists.size(); k++){
          sp<MonBase> e = inf->m_aLists.get(k);
          if( e.has() && (e->m_sec <= (sec - range)) ){
              inf->m_aLists.remove(k);
              --k;
          }else{
              break;
          }
      }

      if( !inf->m_last.has() ){
          inf->m_last = device;
          return NULL;
      }

      sp<MonBase> dep = device->depth(no, sec, inf->m_last);
      inf->m_last = device;
      inf->m_aLists.insert(dep);

      if( next != 0 ){
          if( !inf->m_total.has() )
              inf->m_total = grp->m_base->create(sec);

          if( (sec % next) != 0 ){
              inf->m_total->plus(dep);
              inf->m_plus_count++;
          }else{
              inf->m_total->avg(inf->m_plus_count);
              if( DFW_RET(retval, run_s_l(grp, sec, no+1, inf->m_total)) ){
                  if( retval->error() == DFW_OK )
                      return NULL;
                  return DFW_RETVAL_D(retval);
              }
              inf->m_total = NULL;
          }
      }

      if(DFW_RET(retval, grp->m_base->draw(no, inf, dep, m_sPath.toChars())))
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

