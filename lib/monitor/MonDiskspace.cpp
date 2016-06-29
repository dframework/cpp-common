#include <dframework/monitor/MonDiskspace.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>
#include <sys/vfs.h>

namespace dframework {

  MonDiskspace::Data::Data(){
  }

  MonDiskspace::Data::~Data(){
  }

  MonDiskspace::MonDiskspace(uint64_t sec)
          : MonBase(sec)
  {
  }

  MonDiskspace::~MonDiskspace(){
  }

  sp<MonBase> MonDiskspace::create(uint64_t sec){
      return new MonDiskspace(sec);
  }

  const char* MonDiskspace::source_path(){
      return "/proc/mounts";
  }

  const char* MonDiskspace::savename(){
      return "diskspace";
  }

  sp<Retval> MonDiskspace::readData(){
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

  sp<Retval> MonDiskspace::parseLine(String& sLine){
      sp<Retval> retval;

      Regexp a("^([\\S]+)[\\s]+([\\S]+)[\\s]+([\\S\\s]+)");
      if( DFW_RET(retval, a.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      sp<Data> d = new Data();
      d->m_sDev    .set(a.getMatch(1), a.getMatchLength(1));
      if( d->m_sDev.indexOf("/dev")!=0 )
          return NULL;

      d->m_sPath   .set(a.getMatch(2), a.getMatchLength(2));
      d->m_sOptions.set(a.getMatch(3), a.getMatchLength(3));

      struct statfs stfs;
      ::memset(&stfs, 0, sizeof(struct statfs));
      if( ::statfs(d->m_sPath.toChars(), &stfs) < 0 ){
          return NULL;
      }

      d->m_blocks = stfs.f_blocks * (stfs.f_bsize/1024);
      d->m_avail  = stfs.f_bavail * (stfs.f_bsize/1024);
      d->m_free   = stfs.f_bfree * (stfs.f_bsize/1024);
      d->m_used   = d->m_blocks - d->m_avail;

      if( DFW_RET(retval, m_aLists.insert(d)) ){
          return DFW_RETVAL_D(retval);
      }

#if 0
printf("%s, %s, b=%lu, a=%lu, f=%lu, u=%lu\n"
, d->m_sDev.toChars()
, d->m_sPath.toChars()
, d->m_blocks, d->m_avail, d->m_free, d->m_used
);
#endif

      return NULL;
  }

  sp<MonBase> MonDiskspace::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      DFW_UNUSED(no);
      sp<MonDiskspace> old = old_;
      sp<MonDiskspace> ret = new MonDiskspace(sec);

      for( int k=0; k<m_aLists.size(); k++){
          sp<Data> d = new Data();
          sp<Data> c = m_aLists.get(k);

          d->m_sDev  = c->m_sDev;
          d->m_sPath = c->m_sPath;
          d->m_sOptions = c->m_sOptions;

          d->m_blocks = c->m_blocks;
          d->m_avail = c->m_avail;
          d->m_free = c->m_free;
          d->m_used = c->m_used;

          ret->m_aLists.insert(d);
      }

      return ret;
  }

  void MonDiskspace::plus(sp<MonBase>& old_){
      sp<MonDiskspace> old = old_;

      for( int k=0; k<old->m_aLists.size(); k++){
          sp<Data> o = old->m_aLists.get(k);
          sp<Data> c = m_aLists.get(k);
          if( !c.has() ){
              c = new Data();
              c->m_sDev     = o->m_sDev;
              c->m_sPath    = o->m_sPath;
              c->m_sOptions = o->m_sOptions;
              m_aLists.insert(c);
          }

          c->m_blocks += o->m_blocks;
          c->m_avail  += o->m_avail;
          c->m_free   += o->m_free;
          c->m_used   += o->m_used;
      }
  }

  bool MonDiskspace::getRawString(String& s, sp<MonBase>& b){
      s = String::format(
          "%lu"
         , b->m_sec
      );
      return false;
  }

  sp<Retval> MonDiskspace::draw(int num, sp<info>& info, sp<MonBase>& thiz
                        , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

