#ifndef DFRAMEWORK_MONITOR_MONBASE_H
#define DFRAMEWORK_MONITOR_MONBASE_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/io/File.h>

namespace dframework {

  class Monitor;

  class MonBase : public Object
  {
  public:
      static const char* SUBDIR;

  public:
      class info : public Object
      {
      private:
          int         m_plus_count;

      public:
          uint64_t    m_sec;
          sp<MonBase> m_last;
          sp<MonBase> m_total;
          ArraySorted<MonBase> m_aLists;

      public:
          info();
          virtual ~info();

          DFW_OPERATOR_EX_DECLARATION(info, m_sec);

          friend class Monitor;
      };

      class group : public Object
      {
      public:
          sp<MonBase> m_base;
          Array<info> m_infos;

      public:
          group();
          virtual ~group();
      };

  public:
      uint64_t m_sec;

  public:
      MonBase(uint64_t sec);
      virtual ~MonBase();

      virtual sp<MonBase> create(uint64_t sec) = 0;
      virtual const char* source_path() = 0;
      virtual const char* savename() = 0;
      virtual const char* rawname() = 0;
      virtual sp<Retval>  readData() = 0;
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old) = 0;
      virtual void        plus(sp<MonBase>& old) = 0;
      virtual void        avg(int count) = 0;
      virtual sp<Retval>  draw(int num, sp<info>&, sp<MonBase>&
                              , const char* path) = 0;

      virtual sp<MonBase> createBlank(uint64_t sec, sp<MonBase>& old)=0;

      virtual bool       getRawString(String& out, sp<MonBase>& base);
      virtual sp<Retval>  loadData(sp<MonBase>& out, String&);
      virtual sp<Retval> loadRawData(unsigned, sp<info>&
                              , String&, int max_rows);
      virtual sp<Retval> saveRawData(int num, sp<info>&, sp<MonBase>&
                              , const char* path);
      virtual sp<Retval> openTempFile(sp<File>& out
                             , String& tempfile, String& orgFile
                             , const char* path, const char* subdir
                             , uint64_t subnm,  const char* ext);
      virtual sp<Retval> replaceTempFile(String& sTemp, String& sOrg);

      DFW_OPERATOR_EX_DECLARATION(MonBase, m_sec);
  };

};

#endif /* DFRAMEWORK_MONITOR_MONBASE_H */

