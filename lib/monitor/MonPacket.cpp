#include <dframework/monitor/MonPacket.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/util/StringArray.h>
#include <dframework/lang/Long.h>

namespace dframework {

  int  MonPacket::N_R_BYTES = 0;
  int  MonPacket::N_R_PACKETS = 0;
  int  MonPacket::N_R_ERRS = 0;
  int  MonPacket::N_T_BYTES = 0;
  int  MonPacket::N_T_PACKETS = 0;
  int  MonPacket::N_T_ERRS = 0;
  bool MonPacket::N_HAS = false;

  const char* MonPacket::SAVE_FILENM = "packet";

  MonPacket::MonPacket(uint64_t sec)
          : MonBase(sec)
  {
      m_rbytes = 0;
      m_rpackets = 0;
      m_rerrs = 0;
      m_tbytes = 0;
      m_tpackets = 0;
      m_terrs = 0;
  }

  MonPacket::~MonPacket(){
  }

  sp<MonBase> MonPacket::create(uint64_t sec){
      return new MonPacket(sec);
  }

  const char* MonPacket::source_path(){
      return "/proc/net/dev";
  }

  const char* MonPacket::savename(){
      return SAVE_FILENM;
  }

  const char* MonPacket::rawname(){
      return SAVE_FILENM;
  }

  sp<Retval> MonPacket::readData(){
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
      int round = 0;
      do{
          if( p == NULL )
              break;
          if( (lp = strstr(p, "\n")) == NULL )
              break;
          if( (len = lp - p) == 0 )
              break;
          sLine.set(p, len);
          p += len + 1;

          if( round == 0 ){
          }else if( round == 1 && DFW_RET(retval, parseTitle(sLine)) ){
              return DFW_RETVAL_D(retval);
          }else if( round > 1 && DFW_RET(retval, parseLine(sLine)) ){
              return DFW_RETVAL_D(retval);
          }

          round++;
      }while(true);

      return NULL;
  }

  sp<Retval> MonPacket::parseTitle_l(sp<String>& s
                         , int* all, int* bytes, int* packets, int* errs)
  {
      int find = 0;
      int c = 0;
      String sVal;
      const char* p = s->toChars();
      do{
          if( (*p) == '\0' ) { break; }
          if( ((*p)==' ') || ((*p)=='\t') ){ p++; continue; }

          const char* b = ::strstr(p, " ");
          if( !b ) {
              sVal.set(p);
          }else{
              sVal.set(p, b-p);
          }

          p += sVal.length();

          //bytes    packets errs drop fifo frame compressed multicast
          //bytes    packets errs drop fifo colls carrier compressed
          if( sVal.equals("bytes") ){
              *bytes = c;
              find++;
          }else if( sVal.equals("packets") ){
              *packets = c;
              find++;
          }else if( sVal.equals("errs") ){
              *errs = c;
              find++;
          }

          c++;
      }while(true);

      if( find == 3 ){
          *all = c;
          return NULL;
      }
      return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                 , "Not find need format (%s)", source_path());
  }

  sp<Retval> MonPacket::parseTitle(String& sLine)
  {
      sp<Retval> retval;

      if( N_HAS )
          return NULL;

      StringArray sa1;
      sa1.split(sLine, "|");
      if(sa1.size()!=3){
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Unknown format %s", source_path());
      }

      int all, all2;
      sp<String> s1 = sa1.getString(1);
      if( DFW_RET(retval, parseTitle_l(s1, &all
                        , &N_R_BYTES, &N_R_PACKETS, &N_R_ERRS)) )
          return DFW_RETVAL_D(retval);

      sp<String> s2 = sa1.getString(2);
      if( DFW_RET(retval, parseTitle_l(s2, &all2
                        , &N_T_BYTES, &N_T_PACKETS, &N_T_ERRS)) )
          return DFW_RETVAL_D(retval);

      N_T_BYTES += all;
      N_T_PACKETS += all;
      N_T_ERRS += all;
      N_HAS = true;
      return NULL;
  }

  sp<Retval> MonPacket::parseLine(String& sLine){
      sp<Retval> retval;

      Regexp b("^[\\s]*([\\S]+):[\\s]*([\\S\\s]+)");
      if( DFW_RET(retval, b.regexp(sLine.toChars())) )
          return DFW_RETVAL_D(retval);

      String sName;
      String sTemp;
      sName.set(b.getMatch(1), b.getMatchLength(1));
      sTemp.set(b.getMatch(2), b.getMatchLength(2));

     //face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
     //eno16777736: 43582102  237978    0  495    0     0          0         0  4023838   47391    0    0    0     0       0          0
     //lo: 6879295   64596    0    0    0     0          0         0  6879295   64596    0    0    0     0       0          0

      if( sName.equals("lo") ) return NULL;

      int c = 0;
      String sVal;
      const char* p = sTemp.toChars();
      do{
          if( (*p) == '\0' ) break;
          if( ((*p)==' ') || ((*p)=='\t') ){
              p++;
              continue;
          }

          const char* b = ::strstr(p, " ");
          if( !b ) {
              sVal.set(p);
          }else{
              sVal.set(p, b-p);
          }

          p += sVal.length();

          if( N_R_BYTES == c ) {
              m_rbytes += Long::parseLong(sVal);
          }else if( N_R_PACKETS == c ) {
              m_rpackets += Long::parseLong(sVal);
          }else if( N_R_ERRS == c ) {
              m_rerrs += Long::parseLong(sVal);
          }else if( N_T_BYTES == c ) {
              m_tbytes += Long::parseLong(sVal);
          }else if( N_T_PACKETS == c ) {
              m_tpackets += Long::parseLong(sVal);
          }else if( N_T_ERRS == c ) {
              m_terrs += Long::parseLong(sVal);
          }

          c++;
      }while(true);

      return NULL;
  }

  sp<MonBase> MonPacket::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      sp<MonPacket> old = old_;
      sp<MonPacket> ret = create(sec);

      if( no == 0 ){
          ret->m_rbytes = m_rbytes - old->m_rbytes;
          ret->m_rpackets = m_rpackets - old->m_rpackets;
          ret->m_rerrs = m_rerrs - old->m_rerrs;
          ret->m_tbytes = m_tbytes - old->m_tbytes;
          ret->m_tpackets = m_tpackets - old->m_tpackets;
          ret->m_terrs = m_terrs - old->m_terrs;
      }else{
          ret->m_rbytes = m_rbytes;
          ret->m_rpackets = m_rpackets;
          ret->m_rerrs = m_rerrs;
          ret->m_tbytes = m_tbytes;
          ret->m_tpackets = m_tpackets;
          ret->m_terrs = m_terrs;
      }

      return ret;
  }

  void MonPacket::plus(sp<MonBase>& old_){
      sp<MonPacket> old = old_;

      m_rbytes += old->m_rbytes;
      m_rpackets += old->m_rpackets;
      m_rerrs += old->m_rerrs;
      m_tbytes += old->m_tbytes;
      m_tpackets += old->m_tpackets;
      m_terrs += old->m_terrs;
  }

  bool MonPacket::getRawString(String& s, sp<MonBase>& b){
      sp<MonPacket> c = b;
      if( !c.has() ) return false;

      s = String::format(
              "%lu\t"
              "|%lu %lu %lu\t"
              "|%lu %lu %lu"
              "\n"
          , c->m_sec
          , c->m_rbytes, c->m_rpackets, c->m_rerrs
          , c->m_tbytes, c->m_tpackets, c->m_terrs
      );
      return true;
  }

  sp<MonBase> MonPacket::createBlank(uint64_t sec, sp<MonBase>& old_){
      DFW_UNUSED(old_);
      return create(sec);
  }

  sp<Retval> MonPacket::loadData(sp<MonBase>& out, String& sLine)
  {
      sp<Retval> retval;

      String s_sec;
      String s_rbytes, s_rpackets, s_rerrs;
      String s_tbytes, s_tpackets, s_terrs;

      StringArray sa;
      sa.split(sLine.toChars(), "|");
      if(sa.size()!=3)
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0, "Unknown format : %s", sLine.toChars());

      sp<String> sa1 = sa.getString(0);
      sp<String> sa2 = sa.getString(1);
      sp<String> sa3 = sa.getString(2);

      uint64_t d_sec = Long::parseLong(sa1->toChars());
      {
          int round = 0;
          const char* v = NULL;
          const char* p = sa2->toChars();
          do{
              if( *p == ' ' || *p == '\t' || *p == '|' || *p=='\0'){
                  if( v ){
                      switch(round){
                      case 0: s_rbytes.set(v, p-v); break;
                      case 1: s_rpackets.set(v, p-v); break;
                      case 2: s_rerrs.set(v, p-v); break;
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
          if( round != 3 ){
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                         , "Unknown format %s", sLine.toChars());
          }
      }

      {
          int round = 0;
          const char* v = NULL;
          const char* p = sa3->toChars();
          do{
              if( *p == ' ' || *p == '\t' || *p == '|' || *p=='\0'){
                  if( v ){
                      switch(round){
                      case 0: s_tbytes.set(v, p-v); break;
                      case 1: s_tpackets.set(v, p-v); break;
                      case 2: s_terrs.set(v, p-v); break;
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
          if( round != 3 ){
              return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                         , "Unknown format %s", sLine.toChars());
          }
      }

      sp<MonPacket> dest = create(d_sec);
      dest->m_rbytes   = Long::parseLong(s_rbytes);
      dest->m_rpackets = Long::parseLong(s_rpackets);
      dest->m_rerrs    = Long::parseLong(s_rerrs);
      dest->m_tbytes   = Long::parseLong(s_tbytes);
      dest->m_tpackets = Long::parseLong(s_tpackets);
      dest->m_terrs    = Long::parseLong(s_terrs);

      out = dest;
      return NULL;
  }

  sp<Retval> MonPacket::draw(int num, sp<info>& info, sp<MonBase>& thiz
                     , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

