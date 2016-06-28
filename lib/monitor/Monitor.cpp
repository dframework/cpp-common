#include <dframework/monitor/Monitor.h>
#include <dframework/util/Time.h>

namespace dframework {

  unsigned Monitor::SECONDS[] = {5, 240, 1800, 7200, 0};

  Monitor::Monitor(){
  }

  Monitor::~Monitor(){
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

  void Monitor::run()
  {
      sp<Retval> retval;
      uint64_t   past = 0;

      do{
          dfw_time_t ctime = Time::currentTimeMillis();
          uint64_t csec = ctime / 1000;
          if( ((csec % SECONDS[0]) == 0) && (past<csec) ){ // 5 sec
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
              // FIXME:
printf("retval=%s\n", retval->dump().toChars());
              continue;
          }

          run_s_l(grp, sec, 0, device);
      }
  }

  bool Monitor::run_s_l(sp<MonBase::group>& grp, uint64_t sec, int no
                       , sp<MonBase>& device)
  {
      if( SECONDS[no] == 0 ){
          //printf("end no=%d\n", no);
          return false;
      }

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
          return true;
      }

      sp<MonBase> dep = device->depth(no, sec, inf->m_last);
      inf->m_last = device;
      inf->m_aLists.insert(dep);

      if( next != 0 ){
          if( !inf->m_total.has() )
              inf->m_total = grp->m_base->create(sec);

          if( (sec % next) != 0 ){
              inf->m_total->plus(dep);
          }else{
              if( !run_s_l(grp, sec, no+1, inf->m_total) )
                  return true;
              inf->m_total = NULL;
          }
      }

      grp->m_base->draw(no, inf, dep);

      return true;
  }

};

