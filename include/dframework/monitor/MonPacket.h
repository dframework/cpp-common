#ifndef DFRAMEWORK_MONITOR_MONPACKET_H
#define DFRAMEWORK_MONITOR_MONPACKET_H

#include <dframework/monitor/MonBase.h>
#include <dframework/util/StringArray.h>

namespace dframework {

  class MonPacket : public MonBase
  {
  public:
      static const char* SAVE_FILENM;

  private:
      static int  N_R_BYTES;
      static int  N_R_PACKETS;
      static int  N_R_ERRS;
      static int  N_T_BYTES;
      static int  N_T_PACKETS;
      static int  N_T_ERRS;
      static bool N_HAS;

  public:
      uint64_t m_rbytes;
      uint64_t m_rpackets;
      uint64_t m_rerrs;

      uint64_t m_tbytes;
      uint64_t m_tpackets;
      uint64_t m_terrs;

  private:
      sp<Retval> parseTitle_l(sp<String>& s, int* all
                         , int* bytes, int* packets, int* errs);
      sp<Retval> parseTitle(String& sLine);
      sp<Retval> parseLine(String& sLine);

  public:
      MonPacket(uint64_t sec);
      virtual ~MonPacket();

      virtual sp<MonBase> create(uint64_t sec);
      virtual const char* source_path();
      virtual const char* savename();
      virtual const char* rawname();

      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual sp<Retval>  draw(int num, sp<info>&, sp<MonBase>&
                              , const char* path);

      virtual bool        getRawString(String& s, sp<MonBase>& b);
      virtual sp<MonBase> createBlank(uint64_t, sp<MonBase>&);
      virtual sp<Retval>  loadData(sp<MonBase>& out, String&);

  };

};

#endif /* DFRAMEWORK_MONITOR_MONPACKET_H */

