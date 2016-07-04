#include <dframework/monitor/MonLoadavg.h>
#include <dframework/lang/Float.h>
#include <dframework/lang/Integer.h>
#include <dframework/lang/Long.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>

namespace dframework {

  const char* MonLoadavg::SAVE_FILENM = "loadavg";

  MonLoadavg::MonLoadavg(uint64_t sec)
          : MonBase(sec)
  {
      m_1  = 0;
      m_5  = 0;
      m_15 = 0;
  }

  MonLoadavg::~MonLoadavg(){
  }

  sp<MonBase> MonLoadavg::create(uint64_t sec){
      return new MonLoadavg(sec);
  }

  const char* MonLoadavg::source_path(){
      return "/proc/loadavg";
  }

  const char* MonLoadavg::savename(){
      return SAVE_FILENM;
  }

  const char* MonLoadavg::rawname(){
      return SAVE_FILENM;
  }

  sp<Retval> MonLoadavg::readData(){
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

          if( DFW_RET(retval, parseLine_loadavg(sLine)) ){
printf("%s\n", retval->dump().toChars());
          }
      }while(true);

      return NULL;
  }


  sp<Retval> MonLoadavg::parseLine_loadavg(String& sLine){
      sp<Retval> retval;

      Regexp a("^([.0-9]+)[\\s]+([.0-9]+)[\\s]+([.0-9]+)[\\s\\S]*");
      if( DFW_RET(retval, a.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      String s_1;
      String s_5;
      String s_15;
      s_1.set(a.getMatch(1), a.getMatchLength(1));
      s_5.set(a.getMatch(2), a.getMatchLength(2));
      s_15.set(a.getMatch(3), a.getMatchLength(3));

      m_1 = Float::parseFloat(s_1)*100;
      m_5 = Float::parseFloat(s_5)*100;
      m_15 = Float::parseFloat(s_15)*100;
      return NULL;
  }

  sp<MonBase> MonLoadavg::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      DFW_UNUSED(no);
      DFW_UNUSED(old_);

      sp<MonLoadavg> ret = create(sec);
      ret->m_1 = m_1;
      ret->m_5 = m_5;
      ret->m_15 = m_15;
      return ret;
  }

  void MonLoadavg::plus(sp<MonBase>& old_){
      sp<MonLoadavg> old = old_;
#if 1
      m_1 += old->m_1;
      m_5 += old->m_5;
      m_15 += old->m_15;
#else
      if( m_1 < old->m_1 ) m_1 = old->m_1;
      if( m_5 < old->m_5 ) m_5 = old->m_5;
      if( m_15< old->m_15) m_15 = old->m_15;
#endif
  }

  void MonLoadavg::avg(int count){
#if 1
      if( count == 0 ) return;

      m_1 /= count;
      m_5 /= count;
      m_15 /= count;
#else
      DFW_UNUSED(count);
#endif
  }

  bool MonLoadavg::getRawString(String& s, sp<MonBase>& b){
      sp<MonLoadavg> c = b;
      if( !c.has() ) return false;
      s = String::format("%lu\t%u %lu %lu\n", c->m_sec, c->m_1, c->m_5, c->m_15);
      return true;
  }

  sp<MonBase> MonLoadavg::createBlank(uint64_t sec, sp<MonBase>& old_){
      DFW_UNUSED(old_);
      return create(sec);
  }

  sp<Retval> MonLoadavg::loadData(sp<MonBase>& out, String& sLine)
  {
      sp<Retval> retval;

      Regexp a("^([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)[\\s]+([0-9]+)");
      if( DFW_RET(retval, a.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      String s_sec, s_1, s_5, s_15;

      s_sec.set(a.getMatch(1), a.getMatchLength(1));
      s_1.set(a.getMatch(2), a.getMatchLength(2));
      s_5.set(a.getMatch(3), a.getMatchLength(3));
      s_15.set(a.getMatch(4), a.getMatchLength(4));

      uint64_t d_sec = Long::parseLong(s_sec);
      sp<MonLoadavg> dest = create(d_sec);
      dest->m_1 = Long::parseLong(s_1);
      dest->m_5 = Long::parseLong(s_5);
      dest->m_15 = Long::parseLong(s_15);

      out = dest;
      return NULL;
  }

  sp<Retval> MonLoadavg::draw(int num, sp<info>& info, sp<MonBase>& thiz
                      , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

