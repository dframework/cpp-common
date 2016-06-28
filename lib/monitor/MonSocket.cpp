#include <dframework/monitor/MonSocket.h>
#include <dframework/io/File.h>
#include <dframework/util/Regexp.h>
#include <dframework/lang/Long.h>
#include <dframework/net/Net.h>

namespace dframework {

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

  const char* MonSocket::source_path(){
      return "/proc/net/tcp6";
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

#if 0
printf("localaddr=%s, localport=%d, remoteaddr=%s, remoteport=%d, s=%d, tx=%lu, rx=%lu"
", uid=%d, inode=%lu, last=%d"
"\n"
, line->m_sLocalAddr.toChars()
, line->m_localPort
, line->m_sRemoteAddr.toChars()
, line->m_remotePort
, line->m_status
, line->m_tx
, line->m_rx
, line->m_uid
, line->m_inode
, line->m_7
);
#endif
      return NULL;
  }

  sp<MonBase> MonSocket::depth(int no, uint64_t sec, sp<MonBase>& old_)
  {
      DFW_UNUSED(no);
      DFW_UNUSED(sec);
      DFW_UNUSED(old_);

      sp<MonSocket> ret = new MonSocket(sec);
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

  void MonSocket::draw(int num, sp<info>& info, sp<MonBase>& thiz){
      DFW_UNUSED(num);
      DFW_UNUSED(info);
      DFW_UNUSED(thiz);
  }

};

