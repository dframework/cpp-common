#include <dframework/monitor/MonSocket.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Long.h>
#include <dframework/net/Net.h>

namespace dframework {

  const char* MonSocket::SAVE_FILENM = "socket";

  MonSocket::Line::Line(){
      m_localPort = 0;
      m_remotePort = 0;

      m_status = 0;
      m_tx = m_rx = 0;
      m_tr = m_when = 0;
      m_retrnsmt = 0;
      m_uid = 0;
      m_timeout = 0;
      m_inode = 0;
      m_1 = m_2 = m_3 = m_4 = m_5 = m_6 = m_7 = 0;
  }

  MonSocket::Line::~Line(){
  }

  MonSocket::Data::Data(){
      m_total = 0;
      m_est = 0;
      m_syn_sent = 0;
      m_syn_recv = 0;
      m_fin_wait1 = 0;
      m_fin_wait2 = 0;
      m_time_wait = 0;
      m_close = 0;
      m_close_wait = 0;
      m_last_ack = 0;
      m_listen = 0;
      m_closing = 0;
  }

  MonSocket::Data::~Data(){
  }

  MonSocket::MonSocket(uint64_t sec)
          : MonBase(sec)
  {
      m_all = new Data();
  }

  MonSocket::~MonSocket(){
  }

  sp<MonBase> MonSocket::create(uint64_t sec){
      return new MonSocket(sec);
  }

  const char* MonSocket::source_path(){
      return "/proc/net/tcp6";
  }

  const char* MonSocket::savename(){
      return SAVE_FILENM;
  }

  const char* MonSocket::rawname(){
      return SAVE_FILENM;
  }

  sp<Retval> MonSocket::readData(){
      sp<Retval> retval;

      String sContents;
      String sLine;
      if( DFW_RET(retval, File::contents(sContents, source_path())) )
          return DFW_RETVAL_D(retval);
      if( sContents.length()==0 )
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     ,"Has not contents at %s", source_path());

      int round = 0;
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

          if( round == 0 ){ round++; continue; }
          if( DFW_RET(retval, parseLine(sLine)) ){
              // FIXME:
              printf("%s\n", retval->dump().toChars());
          }
          round++;
      }while(true);

      return NULL;
  }

  sp<Retval> MonSocket::parseLine_l(int round, sp<Line>& line
                                  , const char* v, int size)
  {
      String s;
      s.set(v, size);

      //s = "00000000000000000000000001000000"; // 32
      switch( round ){
      case 0:
          break;
      case 1:
	  line->m_sLocalAddr = Net::hexstringToIp(s.toChars());
          break;
      case 2: 
          line->m_localPort = (s.length()?::strtol(s.toChars(),NULL,16):0);
          break;
      case 3:
	  line->m_sRemoteAddr = Net::hexstringToIp(s.toChars());
          break;
      case 4:
          line->m_remotePort = (s.length()?::strtol(s.toChars(),NULL,16):0);
          break;
      case 5:
          line->m_status = (s.length()?::strtol(s.toChars(),NULL,16):0);
          break;
      case 6:
          line->m_tx = (s.length()?::strtoul(s.toChars(),NULL,16):0);
          break;
      case 7:
          line->m_rx = (s.length()?::strtoul(s.toChars(),NULL,16):0);
          break;
      case 8:
          line->m_tr = (s.length()?::strtol(s.toChars(),NULL,16):0);
          break;
      case 9:
          line->m_when = (s.length()?::strtoul(s.toChars(),NULL,16):0);
          break;
      case 10:
          line->m_retrnsmt = (s.length()?::strtoul(s.toChars(),NULL,16):0);
          break;
      case 11:
          line->m_uid = (s.length()?::atoi(s.toChars()):0);
          break;
      case 12:
          line->m_timeout = (s.length()?::atoi(s.toChars()):0);
          break;
      case 13:
          line->m_inode = (s.length()?::strtoul(s.toChars(),NULL,10):0);
          break;
      case 14:
          line->m_1 = (s.length()?::atoi(s.toChars()):0);
          break;
      case 15:
          line->m_2 = (s.length()?::strtoul(s.toChars(),NULL,16):0);
          break;
      case 16:
          line->m_3 = (s.length()?::atoi(s.toChars()):0);
          break;
      case 17:
          line->m_4 = (s.length()?::atoi(s.toChars()):0);
          break;
      case 18:
          line->m_5 = (s.length()?::atoi(s.toChars()):0);
          break;
      case 19:
          line->m_6 = (s.length()?::atoi(s.toChars()):0);
          break;
      case 20:
          line->m_7 = (s.length()?::atoi(s.toChars()):0);
          break;
      }
      return NULL;
  }

  sp<Retval> MonSocket::parseLine(String& sLine)
  {
      sp<Retval> retval;
      sp<Line> line = new Line();

      int round = 0;
      const char* v = NULL;
      const char* p = sLine.toChars();
      while(*p){
          if( (*p==' ') || (*p=='\t') || (*p==':') ){
              if(v){
                  parseLine_l(round, line, v, p-v);
                  v = NULL;
                  round++;
              }
          }else if(!v){
              v = p;
          }
          p++;
      }

      m_all->m_total++;
      switch( line->m_status ){
      case TCP_ESTABLISHED : m_all->m_est++; break;
      case TCP_SYN_SENT: m_all->m_syn_sent++; break;
      case TCP_SYN_RECV: m_all->m_syn_recv++; break;
      case TCP_FIN_WAIT1: m_all->m_fin_wait1++; break;
      case TCP_FIN_WAIT2: m_all->m_fin_wait2++; break;
      case TCP_TIME_WAIT: m_all->m_time_wait++; break;
      case TCP_CLOSE: m_all->m_close++; break;
      case TCP_CLOSE_WAIT: m_all->m_close_wait++; break;
      case TCP_LAST_ACK: m_all->m_last_ack++; break;
      case TCP_LISTEN: m_all->m_listen++; break;
      case TCP_CLOSING: m_all->m_closing++; break;
      }

      return NULL;
  }

  sp<MonBase> MonSocket::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      DFW_UNUSED(no);
      DFW_UNUSED(sec);
      DFW_UNUSED(old_);

      sp<MonSocket> ret = create(sec);
      sp<Data> t = ret->m_all;
      sp<Data> c = m_all;

      t->m_total = c->m_total;
      t->m_est = c->m_est;
      t->m_syn_sent = c->m_syn_sent;
      t->m_syn_recv = c->m_syn_recv;
      t->m_fin_wait1 = c->m_fin_wait1;
      t->m_fin_wait2 = c->m_fin_wait2;
      t->m_time_wait = c->m_time_wait;
      t->m_close = c->m_close;
      t->m_close_wait = c->m_close_wait;
      t->m_last_ack = c->m_last_ack;
      t->m_listen = c->m_listen;
      t->m_closing = c->m_closing;

      return ret;
  }

  void MonSocket::plus(sp<MonBase>& old_){
      sp<MonSocket> old = old_;

      sp<Data> c = m_all;
      sp<Data> o = old->m_all;

      c->m_total      += o->m_total;
      c->m_est        += o->m_est;
      c->m_syn_sent   += o->m_syn_sent;
      c->m_syn_recv   += o->m_syn_recv;
      c->m_fin_wait1  += o->m_fin_wait1;
      c->m_fin_wait2  += o->m_fin_wait2;
      c->m_time_wait  += o->m_time_wait;
      c->m_close      += o->m_close;
      c->m_close_wait += o->m_close_wait;
      c->m_last_ack   += o->m_last_ack;
      c->m_listen     += o->m_listen;
      c->m_closing    += o->m_closing;
  }

  void MonSocket::avg(int count){
      sp<Data> c = m_all;
      c->m_total      /= count;
      c->m_est        /= count;
      c->m_syn_sent   /= count;
      c->m_syn_recv   /= count;
      c->m_fin_wait1  /= count;
      c->m_fin_wait2  /= count;
      c->m_time_wait  /= count;
      c->m_close      /= count;
      c->m_close_wait /= count;
      c->m_last_ack   /= count;
      c->m_listen     /= count;
      c->m_closing    /= count;
  }

  bool MonSocket::getRawString(String& s, sp<MonBase>& b){
      sp<MonSocket> c = b;
      if( !c.has() ) return false;
      sp<Data> d = c->m_all;
      if( !d.has() ) return false;

      s = String::format(
              "%lu\t"
              "%lu %lu "
              "%lu %lu "
              "%lu %lu "
              "%lu "
              "%lu %lu "
              "%lu %lu %lu"
              "\n"
          , c->m_sec
          , d->m_total, d->m_est
          , d->m_syn_sent, d->m_syn_recv
          , d->m_fin_wait1, d->m_fin_wait2
          , d->m_time_wait
          , d->m_close, d->m_close_wait
          , d->m_last_ack, d->m_listen, d->m_closing
      );
      return true;
  }

  sp<MonBase> MonSocket::createBlank(uint64_t sec, sp<MonBase>& old_){
      DFW_UNUSED(old_);
      return create(sec);
  }

  sp<Retval> MonSocket::loadData(sp<MonBase>& out, String& sLine)
  {
      sp<Retval> retval;

      String s_sec;
      String s_total, s_est;
      String s_syn_sent, s_syn_recv;
      String s_fin_wait1, s_fin_wait2;
      String s_time_wait;
      String s_close, s_close_wait;
      String s_last_ack;
      String s_listen, s_closing;

      int round = 0;
      const char* v = NULL;
      const char* p = sLine.toChars();
      do{
          if( *p == ' ' || *p == '\t' || *p == '|' || *p=='\0'){
              if( v ){
                  switch(round){
                  case 0: s_sec.set(v, p-v); break;
                  case 1: s_total.set(v, p-v); break;
                  case 2: s_est.set(v, p-v); break;
                  case 3: s_syn_sent.set(v, p-v); break;
                  case 4: s_syn_recv.set(v, p-v); break;
                  case 5: s_fin_wait1.set(v, p-v); break;
                  case 6: s_fin_wait2.set(v, p-v); break;
                  case 7: s_time_wait.set(v, p-v); break;
                  case 8: s_close.set(v, p-v); break;
                  case 9: s_close_wait.set(v, p-v); break;
                  case 10: s_last_ack.set(v, p-v); break;
                  case 11: s_listen.set(v, p-v); break;
                  case 12: s_closing.set(v, p-v); break;
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

      if( round != 13 ){
          return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                     , "Unknown format %s", sLine.toChars());
      }


      uint64_t d_sec = Long::parseLong(s_sec);
      sp<MonSocket> dest = create(d_sec);
      sp<Data> c = dest->m_all;
      c->m_total      = Long::parseLong(s_total);
      c->m_est        = Long::parseLong(s_est);
      c->m_syn_sent   = Long::parseLong(s_syn_sent);
      c->m_syn_recv   = Long::parseLong(s_syn_recv);
      c->m_fin_wait1  = Long::parseLong(s_fin_wait1);
      c->m_fin_wait2  = Long::parseLong(s_fin_wait2);
      c->m_time_wait  = Long::parseLong(s_time_wait);
      c->m_close      = Long::parseLong(s_close);
      c->m_close_wait = Long::parseLong(s_close_wait);
      c->m_last_ack   = Long::parseLong(s_last_ack);
      c->m_listen     = Long::parseLong(s_listen);
      c->m_closing    = Long::parseLong(s_closing);

      out = dest;
      return NULL;
  }

  sp<Retval> MonSocket::draw(int num, sp<info>& info, sp<MonBase>& thiz
                     , const char* path)
  {
      sp<Retval> retval;
      if( DFW_RET(retval, saveRawData(num, info, thiz, path)) )
          return DFW_RETVAL_D(retval);
      return NULL;
  }

};

