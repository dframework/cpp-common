#ifndef DFRAMEWORK_MONITOR_MONMEMORY_H
#define DFRAMEWORK_MONITOR_MONMEMORY_H

#include <dframework/monitor/MonBase.h>

namespace dframework {

  class MonMemory : public MonBase
  {
  public:
      uint64_t m_kernelUsed;
      uint64_t m_swapUsed;

      uint64_t m_total;
      uint64_t m_free;
      uint64_t m_available;
      uint64_t m_buffers;
      uint64_t m_cached;

      uint64_t m_swapTotal;
      uint64_t m_swapFree;
      uint64_t m_swapCached;

      uint64_t m_vmallocTotal;
      uint64_t m_vmallocUsed;
      uint64_t m_vmallocChunk;

      uint64_t m_active;
      uint64_t m_inActive;

#if 0
      uint64_t m_kernelStack;
      uint64_t m_pageTables;

      uint64_t m_activeAnone;
      uint64_t m_inActiveAnone;

      uint64_t m_activeFile;
      uint64_t m_inActiveFile;

      uint64_t m_unevictable;
      uint64_t m_mlocked;
      uint64_t m_dirty;
      uint64_t m_writeback;
      uint64_t m_anonPages;
      uint64_t m_mapped;
      uint64_t m_shmem;
      uint64_t m_slab;
      uint64_t m_sreclaimable;
      uint64_t m_sunreclaim;
      uint64_t m_nfsUnstable;
      uint64_t m_bounce;
      uint64_t m_writebackTmp;
      uint64_t m_commitLimit;
      uint64_t m_committedAs
#endif

  private:
      sp<Retval> parseLine(String& sLine);

  public:
      MonMemory(uint64_t sec);
      virtual ~MonMemory();

      virtual sp<MonBase> create(uint64_t sec);
      virtual const char* source_path();
      virtual const char* savename();

      virtual sp<Retval>  readData();
      virtual sp<MonBase> depth(int no, uint64_t sec
                              , sp<MonBase>& old);
      virtual void        plus(sp<MonBase>& old);
      virtual bool        getRawString(String& s, sp<MonBase>& b);
      virtual sp<Retval>  draw(int num, sp<info>&, sp<MonBase>&
                              , const char* path);


  };

};

#endif /* DFRAMEWORK_MONITOR_MONMEMORY_H */

