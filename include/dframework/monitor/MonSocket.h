#ifndef DFRAMEWORK_MONITOR_MONSOCKET_H
#define DFRAMEWORK_MONITOR_MONSOCKET_H

#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonSocket : public MonBase
  {
  public:
      class Line : public Object
      {
      public:
          String m_sLocalAddr;
          int    m_localPort;
          String m_sRemoteAddr;
          int    m_remotePort;

          int      m_status;
          uint64_t m_tx;
          uint64_t m_rx;
          int      m_tr;
          uint64_t m_when;
          uint64_t m_retrnsmt;
          int      m_uid;
          int      m_timeout;
          uint64_t m_inode;
          int      m_1;
          uint64_t m_2;
          int      m_3;
          int      m_4;
          int      m_5;
          int      m_6;
          int      m_7;

      public:
          Line();
          virtual ~Line();

      };

      class Data : public Object
      {
      public:
          String   m_sName;
          String   m_sKey;

          uint64_t m_total;
          uint64_t m_est;
          uint64_t m_syn_sent;
          uint64_t m_syn_recv;
          uint64_t m_fin_wait1;
          uint64_t m_fin_wait2;
          uint64_t m_time_wait;
          uint64_t m_close;
          uint64_t m_close_wait;
          uint64_t m_last_ack;
          uint64_t m_listen;
          uint64_t m_closing;

      public:
          Data();
          virtual ~Data();

          DFW_OPERATOR_EX_DECLARATION(Data, m_sKey);
      };

  public:
      sp<Data> m_all;

  private:
      sp<Retval> parseLine_l(int round, sp<Line>& line
                           , const char* v, int size);
      sp<Retval> parseLine(String& sLine);

  public:
      MonSocket(uint64_t sec);
      virtual ~MonSocket();

      virtual const char* source_path();

      inline virtual sp<MonBase> create(uint64_t sec){
          return new MonSocket(sec);
      }

      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual void        draw(int num, sp<info>&, sp<MonBase>&);


  };

};

#endif /* DFRAMEWORK_MONITOR_MONSOCKET_H */

