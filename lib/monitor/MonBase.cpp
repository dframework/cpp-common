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

};

