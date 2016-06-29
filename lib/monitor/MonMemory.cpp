#include <dframework/monitor/MonMemory.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Long.h>

namespace dframework {

  MonMemory::MonMemory(uint64_t sec)
          : MonBase(sec)
  {
      m_kernelUsed = 0;
      m_swapUsed   = 0;

      m_total = 0;
      m_free  = 0;
      m_available = 0;
      m_buffers   = 0;
      m_cached    = 0;

      m_swapTotal  = 0;
      m_swapFree   = 0;
      m_swapCached = 0;

      m_vmallocTotal = 0;
      m_vmallocUsed  = 0;
      m_vmallocChunk = 0;

      m_active   = 0;
      m_inActive = 0;
  }

  MonMemory::~MonMemory(){
  }

  sp<MonBase> MonMemory::create(uint64_t sec){
      return new MonMemory(sec);
  }

  const char* MonMemory::source_path(){
      return "/proc/meminfo";
  }

  const char* MonMemory::savename(){
      return "memory";
  }

  sp<Retval> MonMemory::readData(){
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

          if( !DFW_RET(retval, parseLine(sLine)) ){
          }
      }while(true);

      m_swapUsed = m_swapTotal - m_swapFree;
      m_kernelUsed = m_total - m_free - m_buffers - m_cached;

#if 0

uint64_t h = m_kernelUsed + m_cached + m_buffers + m_free;
if( m_total == h ){
	printf("ok-");
}else{
	printf("fa-");
}

uint64_t a = m_swapUsed + m_total;
uint64_t u = m_kernelUsed - m_vmallocUsed;
printf("total=%lu, kernel=%lu, cached=%lu, buffer=%lu, free=%lu"
	", a=%lu, swapu=%lu, vmu=%lu"
	"\n"
	, m_total, m_kernelUsed, m_cached, m_buffers, m_free
	, a, m_swapUsed, m_vmallocUsed
);
#endif
      return NULL;
  }


  sp<Retval> MonMemory::parseLine(String& sLine){
      sp<Retval> retval;

      Regexp a("^([\\S]+):[\\s]+([0-9]+)[\\s]+([a-zA-Z]+)");
      if( DFW_RET(retval, a.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      String sName;
      String s_1;
      String s_2;

      sName.set(a.getMatch(1), a.getMatchLength(1));
      s_1.set(a.getMatch(2), a.getMatchLength(2));
      s_2.set(a.getMatch(3), a.getMatchLength(3));

      if( sName.equals("MemTotal")){
          m_total = Long::parseLong(s_1);
      }else if( sName.equals("MemFree")){
          m_free = Long::parseLong(s_1);
      }else if( sName.equals("MemAvailable")){
          m_available = Long::parseLong(s_1);
      }else if( sName.equals("Buffers")){
          m_buffers = Long::parseLong(s_1);
      }else if( sName.equals("Cached")){
          m_cached = Long::parseLong(s_1);

      }else if( sName.equals("SwapCached")){
          m_swapCached = Long::parseLong(s_1);
      }else if( sName.equals("SwapTotal")){
          m_swapTotal = Long::parseLong(s_1);
      }else if( sName.equals("SwapFree")){
          m_swapFree = Long::parseLong(s_1);

      }else if( sName.equals("VmallocTotal")){
          m_vmallocTotal = Long::parseLong(s_1);
      }else if( sName.equals("VmallocUsed")){
          m_vmallocUsed = Long::parseLong(s_1);
      }else if( sName.equals("VmallocChunk")){
          m_vmallocChunk = Long::parseLong(s_1);

      }else if( sName.equals("Active")){
          m_active = Long::parseLong(s_1);
      }else if( sName.equals("Inactive")){
          m_inActive = Long::parseLong(s_1);
      }

      return NULL;
  }

  sp<MonBase> MonMemory::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      DFW_UNUSED(no);
      DFW_UNUSED(old_);

      sp<MonMemory> ret = new MonMemory(sec);

      ret->m_total = m_total;
      ret->m_free  = m_free;
      ret->m_available  = m_available;
      ret->m_buffers = m_buffers;
      ret->m_cached = m_cached;

      ret->m_swapTotal = m_swapTotal;
      ret->m_swapFree = m_swapFree;
      ret->m_swapCached = m_swapCached;

      ret->m_vmallocTotal = m_vmallocTotal;
      ret->m_vmallocUsed = m_vmallocUsed;
      ret->m_vmallocChunk = m_vmallocChunk;

      ret->m_active = m_active;
      ret->m_inActive = m_inActive;

      sp<MonBase> base = ret;
      return base;
  }

  void MonMemory::plus(sp<MonBase>& old_){
      sp<MonMemory> old = old_;

      m_total += old->m_total;
      m_free  += old->m_free;
      m_available += old->m_available;
      m_buffers   += old->m_buffers;
      m_cached    += old->m_cached;

      m_swapTotal  += old->m_swapTotal;
      m_swapFree   += old->m_swapFree;
      m_swapCached += old->m_swapCached;

      m_vmallocTotal += old->m_vmallocTotal;
      m_vmallocUsed  += old->m_vmallocUsed;
      m_vmallocChunk += old->m_vmallocChunk;

      m_active   += old->m_active;
      m_inActive += old->m_inActive;
  }

  bool MonMemory::getRawString(String& s, sp<MonBase>& b){
      sp<MonMemory> c = b;
      if( !c.has() ) return false;

      s = String::format(
              "%lu\t"
              "%lu %lu %lu %lu %lu "
              "%lu %lu %lu "
              "%lu %lu %lu "
              "%lu %lu"
          , c->m_sec
          , c->m_total, c->m_free, c->m_available, c->m_buffers, c->m_cached
          , c->m_swapTotal, c->m_swapFree, c->m_swapCached
          , c->m_vmallocTotal, c->m_vmallocUsed, c->m_vmallocChunk
          , c->m_active, c->m_inActive
      );
      return true;
  }

  sp<Retval> MonMemory::draw(int num, sp<info>& info, sp<MonBase>& thiz
                     , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

