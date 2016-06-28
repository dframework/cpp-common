#include <dframework/lang/Float.h>

namespace dframework {

  Float::Float(){
      m_value = 0.f;
  }

  Float::Float(float val) : Object(){
      m_value = val;
  }

  Float::Float(Float& val) : Object(){
      m_value = val.value();
  }

  float Float::parseFloat(const char* str, float defaultVal){
      if( !str ) return defaultVal;
      return ::atof(str);
  }

  Float& Float::operator = (const float val) {
      m_value = val;
      return *this;
  }

  Float& Float::operator = (const Float& val) {
      m_value = val.m_value;
      return *this;
  }

};

