#include <dframework/monitor/MonBase.h>
#include <dframework/util/Time.h>

namespace dframework {

  const char* MonBase::SUBDIR = "raw";

  MonBase::info::info(){
  }

  MonBase::info::~info(){
  }

  // ---------------------------------------------------------------

  MonBase::group::group(){
  }

  MonBase::group::~group(){
  }

  // ---------------------------------------------------------------


  MonBase::MonBase(uint64_t sec){
      m_sec = sec;
  }

  MonBase::~MonBase(){
  }

  sp<Retval> MonBase::openTempFile(sp<File>& out
                             , String& sTemp, String& sOrg
                             , const char* path, const char* subdir
                             , uint64_t subnm,  const char* ext)
  {
      sp<Retval> retval;

      String sSubdir = String::format("/%s", subdir);
      String sPath = String::format("%s/%s", path, subdir);
      sOrg = String::format("%s/%s-%lu.%s"
                         , sPath.toChars(), rawname(), subnm, ext);
      sTemp = String::format("%s/%s-%lu.%s.tmp"
                         , sPath.toChars(), rawname(), subnm, ext);


      if( !File::isDirectory(sPath) ){
          if( DFW_RET(retval, File::makeDir(path, sSubdir, 0755)) )
              return DFW_RETVAL_D(retval);
      }

      sp<File> file = new File();
      if( DFW_RET(retval, file->open(sTemp, O_CREAT|O_WRONLY, 0644)) )
          return DFW_RETVAL_D(retval);
      out = file;
      return NULL; 
  }

  sp<Retval> MonBase::replaceTempFile(String& sTemp, String& sOrg){
      sp<Retval> retval;
      if( File::isFile(sOrg) ){
          if( DFW_RET(retval, File::remove(sOrg)) )
              return DFW_RETVAL_D(retval);
      }
      if( DFW_RET(retval, File::rename(sTemp, sOrg)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

  bool MonBase::getRawString(String& s, sp<MonBase>& b){
      DFW_UNUSED(s);
      DFW_UNUSED(b);
      return false;
  }

  sp<Retval> MonBase::saveRawData(int num, sp<info>& info, sp<MonBase>& thiz
                      , const char* path)
  {
      DFW_UNUSED(num);
      DFW_UNUSED(thiz);

      sp<Retval> retval;
      sp<File> file;
      String sTemp, sOrg;

      if( DFW_RET(retval, openTempFile(file, sTemp, sOrg, path, SUBDIR
                                     , info->m_sec, SUBDIR)) ){
          return DFW_RETVAL_D(retval);
      }

      String s;
      for(int k=info->m_aLists.size(); k>0; k--){
          sp<MonBase> b  = info->m_aLists.get(k-1);
          if( !getRawString(s, b) )
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Has not raw string.");
          if( DFW_RET(retval, file->write(s.toChars(), s.length())) ){
              return DFW_RETVAL_D(retval);
          }
      }
      file->close();

      {
          AutoLock _l(this);
          if( DFW_RET(retval, replaceTempFile(sTemp, sOrg)) )
              return DFW_RETVAL_D(retval);
      }

      return NULL;
  }

  sp<Retval> MonBase::loadData(sp<MonBase>& out, String& sLine)
  {
      DFW_UNUSED(out);
      DFW_UNUSED(sLine);
      return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                 , "Not support load data virtual function.");
  }

  sp<Retval> MonBase::loadRawData(unsigned sec, sp<info>& inf
                                , String& sPath, int max_rows)
  {
      sp<Retval> retval;
      String sOrg = String::format("%s/%s/%s-%d.%s"
                         , sPath.toChars(), SUBDIR, rawname(), sec, SUBDIR);
      if( !File::isFile(sOrg) )
          return NULL;

      String sLine;
      String sContents;
      if( DFW_RET(retval, File::contents(sContents, sOrg.toChars())) )
          return DFW_RETVAL_D(retval);
      if( sContents.length()==0 )
          return NULL;

      uint64_t csec = Time::currentTimeMillis()/1000;
      int dsec = csec % sec;
      if(dsec){
          csec += (sec-dsec);
      }

      bool isfirst = false;
      unsigned len;
      const char* lp;
      const char* p = sContents.toChars();
      do{
          if( p == NULL )
              return NULL;
          if( (lp = strstr(p, "\n")) == NULL )
              return NULL;
          if( (len = lp - p) == 0 )
              return NULL;

          sLine.set(p, len);
          p += len + 1;

          sp<MonBase> mb;
          if( DFW_RET(retval, loadData(mb, sLine)) )
              return DFW_RETVAL_D(retval);

          if( mb->m_sec >= csec ) continue;

          if( !isfirst ){
              for(uint64_t st=mb->m_sec+sec; st<csec; st+=sec){
                  sp<MonBase> n = createBlank(st, mb);
                  if( DFW_RET(retval, inf->m_aLists.insert(n)) )
                      return DFW_RETVAL_D(retval);
                  if( inf->m_aLists.size() > max_rows )
                      return NULL;
              }
              isfirst = true;
          }

          if( DFW_RET(retval, inf->m_aLists.insert(mb)) )
              return DFW_RETVAL_D(retval);
          if( inf->m_aLists.size() > max_rows )
              return NULL;
      }while(true);
  }

};

