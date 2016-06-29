#include <dframework/monitor/MonBase.h>

namespace dframework {

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

      String sPath = String::format("%s/%s", path, subdir);
      sTemp = String::format("%s/%s-%lu.%s.tmp"
                         , sPath.toChars(), savename(), subnm, ext);
      sOrg = String::format("%s/%s-%lu.%s"
                         , sPath.toChars(), savename(), subnm, ext);


      if( !File::isDirectory(sPath)
              && DFW_RET(retval, File::makeDir(path, subdir)) )
          return DFW_RETVAL_D(retval);

      sp<File> file = new File();
      if( DFW_RET(retval, file->open(sTemp, O_RDONLY)) )
          return DFW_RETVAL_D(retval);
      out = file;
      return NULL; 
  }

  sp<Retval> MonBase::replaceTempFile(String& sTemp, String& sOrg){
      sp<Retval> retval;
      if( DFW_RET(retval, File::remove(sOrg)) )
          return DFW_RETVAL_D(retval);
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

      static const char* subdir = "raw";
      sp<Retval> retval;
      sp<File> file;
      String sTemp, sOrg;

      if( DFW_RET(retval, openTempFile(file, sTemp, sOrg, path, subdir
                                     , info->m_sec, "raw")) ){
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
          replaceTempFile(sTemp, sOrg);
      }

      return NULL;
  }

};

