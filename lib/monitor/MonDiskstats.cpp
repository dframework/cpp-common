#include <dframework/monitor/MonDiskstats.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>
#include <dframework/util/StringArray.h>

namespace dframework {

  const char* MonDiskstats::SAVE_FILENM = "diskstats";

  MonDiskstats::Data::Data(){
      m_type = 0;
      m_devtype = 0;

      m_rcount = 0;
      m_rmerged = 0;
      m_rsector = 0;
      m_rtime = 0;

      m_wcount = 0;
      m_wmerged = 0;
      m_wsector = 0;
      m_wtime = 0;

      m_iocount = 0;
      m_iotime = 0;
      m_iowtime = 0;
  }

  MonDiskstats::Data::~Data(){
  }

  MonDiskstats::MonDiskstats(uint64_t sec)
          : MonBase(sec)
  {
      m_all = new Data();
  }

  MonDiskstats::~MonDiskstats(){
  }

  sp<MonBase> MonDiskstats::create(uint64_t sec){
      return new MonDiskstats(sec);
  }

  const char* MonDiskstats::source_path(){
      return "/proc/diskstats";
  }

  const char* MonDiskstats::savename(){
      return SAVE_FILENM;
  }

  const char* MonDiskstats::rawname(){
      return SAVE_FILENM;
  }

  sp<Retval> MonDiskstats::readData(){
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

          if( DFW_RET(retval, parseLine(sLine)) ){
              // FIXME:
printf("%s\n", retval->dump().toChars());
          }
      }while(true);

      return NULL;      
  }

  sp<Retval> MonDiskstats::parseLine(String& sLine){
      sp<Retval> retval;

      String s_type, s_devtype, sName;
      String s_rcount, s_rmerged, s_rsector, s_rtime;
      String s_wcount, s_wmerged, s_wsector, s_wtime;
      String s_iocount, s_iotime, s_iowtime;

      int round = 0;
      const char* v = NULL;
      const char* p = sLine.toChars();

      while( true ){
          if( (*p==' ') || (*p=='\t') || (*p=='\0') ){
              if( v ){
                  if( round == 0 ){
                      s_type.set(v, p-v);
                  }else if( round == 1 ){
                      s_devtype.set(v, p-v);
                  }else if( round == 2 ){
                      sName.set(v, p-v);
                  }else if( round == 3 ){
                      s_rcount.set(v, p-v);
                  }else if( round == 4 ){
                      s_rmerged.set(v, p-v);
                  }else if( round == 5 ){
                      s_rsector.set(v, p-v);
                  }else if( round == 6 ){
                      s_rtime.set(v, p-v);
                  }else if( round == 7 ){
                      s_wcount.set(v, p-v);
                  }else if( round == 8 ){
                      s_wmerged.set(v, p-v);
                  }else if( round == 9 ){
                      s_wsector.set(v, p-v);
                  }else if( round == 10 ){
                      s_wtime.set(v, p-v);
                  }else if( round == 11 ){
                      s_iocount.set(v, p-v);
                  }else if( round == 12 ){
                      s_iotime.set(v, p-v);
                  }else if( round == 13 ){
                      s_iowtime.set(v, p-v);
                  }else if( round > 13 ){
                      break;
                  }
                  v = NULL;
                  round++;
              }
              if( *p == '\0' ) break;
          }else if(!v){
              v = p;
          }
          p++;
      } // end while[ *p ]

      if( round!=14 ){
          return DFW_RETVAL_NEW_MSG(DFW_ERROR,0
                     ,"Unknown format : %s", sLine.toChars());
      }

      sp<Data> d = new Data();

      d->m_type    = Integer::parseInt(s_type);
      d->m_devtype = Integer::parseInt(s_devtype);
      d->m_sName   = sName;
      d->m_rcount  = Long::parseLong(s_rcount);
      d->m_rmerged = Long::parseLong(s_rmerged);
      d->m_rsector = Long::parseLong(s_rsector);
      d->m_rtime   = Long::parseLong(s_rtime);
      d->m_wcount  = Long::parseLong(s_wcount);
      d->m_wmerged = Long::parseLong(s_wmerged);
      d->m_wsector = Long::parseLong(s_wsector);
      d->m_wtime   = Long::parseLong(s_wtime);
      d->m_iocount = Long::parseLong(s_iocount);
      d->m_iotime  = Long::parseLong(s_iotime);
      d->m_iowtime = Long::parseLong(s_iowtime);

      if( d->m_type != 8 ) { return NULL; }
      if( d->m_devtype == 0 ) {
          m_all->m_rcount  += d->m_rcount;
          m_all->m_rmerged += d->m_rmerged;
          m_all->m_rsector += d->m_rsector;
          m_all->m_rtime   += d->m_rtime;
          m_all->m_wcount  += d->m_wcount;
          m_all->m_wmerged += d->m_wmerged;
          m_all->m_wsector += d->m_wsector;
          m_all->m_wtime   += d->m_wtime;
          m_all->m_iocount += d->m_iocount;
          m_all->m_iotime  += d->m_iotime;
          m_all->m_iowtime += d->m_iowtime;
      }

      if( DFW_RET(retval, m_aLists.insert(d)) ){
          return DFW_RETVAL_D(retval);
      }

      return NULL;
  }

  void MonDiskstats::depth_l(int no, sp<Data>& d, sp<Data>& c, sp<Data>& o){
      if( no==0 && o.has() ){
          d->m_rcount  = c->m_rcount  - o->m_rcount;
          d->m_rmerged = c->m_rmerged - o->m_rmerged;
          d->m_rsector = c->m_rsector - o->m_rsector;
          d->m_rtime   = c->m_rtime   - o->m_rtime;
          d->m_wcount  = c->m_wcount  - o->m_wcount;
          d->m_wmerged = c->m_wmerged - o->m_wmerged;
          d->m_wsector = c->m_wsector - o->m_wsector;
          d->m_wtime   = c->m_wtime   - o->m_wtime;
          d->m_iocount = c->m_iocount - o->m_iocount;
          d->m_iotime  = c->m_iotime  - o->m_iotime;
          d->m_iowtime = c->m_iowtime - o->m_iowtime;
      }else if( c.has() ){
          d->m_rcount  = c->m_rcount;
          d->m_rmerged = c->m_rmerged;
          d->m_rsector = c->m_rsector;
          d->m_rtime   = c->m_rtime;
          d->m_wcount  = c->m_wcount;
          d->m_wmerged = c->m_wmerged;
          d->m_wsector = c->m_wsector;
          d->m_wtime   = c->m_wtime;
          d->m_iocount = c->m_iocount;
          d->m_iotime  = c->m_iotime;
          d->m_iowtime = c->m_iowtime;
      }
  }

  sp<MonBase> MonDiskstats::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      sp<MonDiskstats> old = old_;
      sp<MonDiskstats> ret = create(sec);

      sp<Data> d, c, o;

      for( int k=0; k<m_aLists.size(); k++){
          d = new Data();
          c = m_aLists.get(k);
          o = old->getData(k);

          d->m_type    = c->m_type;
          d->m_devtype = c->m_devtype;
          d->m_sName   = c->m_sName;

          depth_l(no, d, c, o);

          ret->m_aLists.insert(d);
      }

      d = ret->m_all;
      c = m_all;
      o = old->m_all;

      depth_l(no, d, c, o);

      return ret;
  }

  void MonDiskstats::plus(sp<MonBase>& old_){
      sp<MonDiskstats> old = old_;

      for( int k=0; k<old->m_aLists.size(); k++){
          sp<Data> o = old->m_aLists.get(k);
          sp<Data> c = m_aLists.get(k);
          if( !c.has() ){
              c = new Data();
              c->m_type    = o->m_type;
              c->m_devtype = o->m_devtype;
              c->m_sName   = o->m_sName;
              m_aLists.insert(c);
          }

          c->m_rcount  += o->m_rcount;
          c->m_rmerged += o->m_rmerged;
          c->m_rsector += o->m_rsector;
          c->m_rtime   += o->m_rtime;
          c->m_wcount  += o->m_wcount;
          c->m_wmerged += o->m_wmerged;
          c->m_wsector += o->m_wsector;
          c->m_wtime   += o->m_wtime;
          c->m_iocount += o->m_iocount;
          c->m_iotime  += o->m_iotime;
          c->m_iowtime += o->m_iowtime;
      }

      sp<Data> ca = m_all;
      sp<Data> oa = old->m_all;
      ca->m_rcount  += oa->m_rcount;
      ca->m_rmerged += oa->m_rmerged;
      ca->m_rsector += oa->m_rsector;
      ca->m_rtime   += oa->m_rtime;
      ca->m_wcount  += oa->m_wcount;
      ca->m_wmerged += oa->m_wmerged;
      ca->m_wsector += oa->m_wsector;
      ca->m_wtime   += oa->m_wtime;
      ca->m_iocount += oa->m_iocount;
      ca->m_iotime  += oa->m_iotime;
      ca->m_iowtime += oa->m_iowtime;
  }

  void MonDiskstats::avg(int count){
      DFW_UNUSED(count);
#if 0
      if( count == 0 ) return;

      for( int k=0; k<m_aLists.size(); k++){
          sp<Data> c = m_aLists.get(k);

          c->m_rcount  /= count;
          c->m_rmerged /= count;
          c->m_rsector /= count;
          c->m_rtime   /= count;
          c->m_wcount  /= count;
          c->m_wmerged /= count;
          c->m_wsector /= count;
          c->m_wtime   /= count;
          c->m_iocount /= count;
          c->m_iotime  /= count;
          c->m_iowtime /= count;
      }

      sp<Data> ca = m_all;
      ca->m_rcount  /= count;
      ca->m_rmerged /= count;
      ca->m_rsector /= count;
      ca->m_rtime   /= count;
      ca->m_wcount  /= count;
      ca->m_wmerged /= count;
      ca->m_wsector /= count;
      ca->m_wtime   /= count;
      ca->m_iocount /= count;
      ca->m_iotime  /= count;
      ca->m_iowtime /= count;
#endif
  }

  bool MonDiskstats::getRawString(String& s, sp<MonBase>& b){
      sp<MonDiskstats> c = b;
      if( !c.has() ) return false;

      s = String::format("%lu", c->m_sec);
      for(int k=0; k<c->m_aLists.size(); k++){
          sp<Data> d = c->m_aLists.get(k);
          if( !d.has() ) return false;
          s.appendFmt(
               "\t|%d %d %s\t"
               "%lu %lu %lu %lu\t"
               "%lu %lu %lu %lu\t"
               "%lu %lu %lu"
             , d->m_type, d->m_devtype, d->m_sName.toChars()
             , d->m_rcount, d->m_rmerged, d->m_rsector, d->m_rtime
             , d->m_wcount, d->m_wmerged, d->m_wsector, d->m_wtime
             , d->m_iocount, d->m_iotime, d->m_iowtime
          );
      }
      s.append("\n");
      return true;
  }

  sp<MonBase> MonDiskstats::createBlank(uint64_t sec, sp<MonBase>& old_)
  {
      sp<MonDiskstats> old = old_;
      sp<MonDiskstats> ret = create(sec);

      for( int k=0; k<old->m_aLists.size(); k++){
          sp<Data> d = new Data();
          sp<Data> o = old->m_aLists.get(k);
          d->m_type    = o->m_type;
          d->m_devtype = o->m_devtype;
          d->m_sName   = o->m_sName;
          ret->m_aLists.insert(d);
      }

      return ret;
  }

  sp<Retval> MonDiskstats::loadData(sp<MonBase>& out, String& sLine)
  {
      sp<Retval> retval;

      StringArray ar;
      if( DFW_RET(retval, ar.split(sLine, "|")) )
          return DFW_RETVAL_D(retval);
      if( !(ar.size() > 1) )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Unknown format. ground-size=%d", ar.size());

      sp<String> ar1 = ar.getString(0);
      uint64_t d_sec = Long::parseLong(ar1->toChars());
      sp<MonDiskstats> dest = create(d_sec);

      // get datas
      String s_type, s_devtype, s_name;
      String s_rcount, s_rmerged, s_rsector, s_rtime;
      String s_wcount, s_wmerged, s_wsector, s_wtime;
      String s_iocount, s_iotime, s_iowtime;

      for(int k=1; k<ar.size(); k++){
          sp<String> arx = ar.getString(k);

          int round = 0;
          const char* v = NULL;
          const char* p = arx->toChars();
          do{
              if( *p == ' ' || *p == '\t' || *p == '|' || *p=='\0'){
                  if( v ){
                      switch(round){
                      case 0: s_type.set(v, p-v); break;
                      case 1: s_devtype.set(v, p-v); break;
                      case 2: s_name.set(v, p-v); break;
                      case 3: s_rcount.set(v, p-v); break;
                      case 4: s_rmerged.set(v, p-v); break;
                      case 5: s_rsector.set(v, p-v); break;
                      case 6: s_rtime.set(v, p-v); break;
                      case 7: s_wcount.set(v, p-v); break;
                      case 8: s_wmerged.set(v, p-v); break;
                      case 9: s_wsector.set(v, p-v); break;
                      case 10: s_wtime.set(v, p-v); break;
                      case 11: s_iocount.set(v, p-v); break;
                      case 12: s_iotime.set(v, p-v); break;
                      case 13: s_iowtime.set(v, p-v); break;
                      }
                      v= NULL;
                      round++;
                  }
                  if( *p=='\0' ) break;
              }else if(!v){
                  v = p;
              }
              p++;
          }while(true);
          if( round != 14 ){
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                         , "Unknown format %s", sLine.toChars());
          }

          sp<Data> d = new Data();
          d->m_type    = Integer::parseInt(s_type);
          d->m_devtype = Integer::parseInt(s_devtype);
          d->m_sName = s_name;

          d->m_rcount = Long::parseLong(s_rcount);
          d->m_rmerged = Long::parseLong(s_rmerged);
          d->m_rsector = Long::parseLong(s_rsector);
          d->m_rtime = Long::parseLong(s_rtime);
          d->m_wcount = Long::parseLong(s_wcount);
          d->m_wmerged = Long::parseLong(s_wmerged);
          d->m_wsector = Long::parseLong(s_wsector);
          d->m_wtime = Long::parseLong(s_wtime);

          d->m_iocount = Long::parseLong(s_iocount);
          d->m_iotime = Long::parseLong(s_iotime);
          d->m_iowtime = Long::parseLong(s_iowtime);

          if( d->m_devtype == 0 ) {
              dest->m_all->m_rcount  += d->m_rcount;
              dest->m_all->m_rmerged += d->m_rmerged;
              dest->m_all->m_rsector += d->m_rsector;
              dest->m_all->m_rtime   += d->m_rtime;
              dest->m_all->m_wcount  += d->m_wcount;
              dest->m_all->m_wmerged += d->m_wmerged;
              dest->m_all->m_wsector += d->m_wsector;
              dest->m_all->m_wtime   += d->m_wtime;
              dest->m_all->m_iocount += d->m_iocount;
              dest->m_all->m_iotime  += d->m_iotime;
              dest->m_all->m_iowtime += d->m_iowtime;
          }

          if( DFW_RET(retval, dest->m_aLists.insert(d)) )
              return DFW_RETVAL_D(retval);
      }

      out = dest;
      return NULL;
  }

  sp<Retval> MonDiskstats::draw(int num, sp<info>& info, sp<MonBase>& thiz
                              , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

