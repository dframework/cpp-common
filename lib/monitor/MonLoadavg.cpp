#include <dframework/monitor/MonLoadavg.h>
#include <dframework/lang/Float.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>

namespace dframework {

  const char* MonLoadavg::PATH = "/proc/loadavg";

  MonLoadavg::MonLoadavg(uint64_t sec)
          : MonBase(sec)
  {
      m_1  = 0;
      m_5  = 0;
      m_15 = 0;
  }

  MonLoadavg::~MonLoadavg(){
  }

  sp<Retval> MonLoadavg::readData(){
      sp<Retval> retval;

      String sContents;
      String sLine;
      if( DFW_RET(retval, File::contents(sContents, PATH)) )
          return DFW_RETVAL_D(retval);
      if( sContents.length()==0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     ,"Has not contents at %s", PATH);

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

    //Regexp a("^([.0-9]+)[\\s]+([.0-9]+)[\\s]+([.0-9]+)[\\s]+([\\S]+)[\\s]+([0-9]+)");
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

      sp<MonLoadavg> ret = new MonLoadavg(sec);
      ret->m_1 = m_1;
      ret->m_5 = m_5;
      ret->m_15 = m_15;
      return ret;
  }

  void MonLoadavg::plus(sp<MonBase>& old_){
      sp<MonLoadavg> old = old_;
      m_1 += old->m_1;
      m_5 += old->m_5;
      m_15 += old->m_15;
  }

  void MonLoadavg::draw(int num, sp<info>& info, sp<MonBase>& thiz){
      DFW_UNUSED(num);
      DFW_UNUSED(info);
      DFW_UNUSED(thiz);
  }

};

