#ifndef DFRAMEWORK_LANG_FLOAT_H
#define DFRAMEWORK_LANG_FLOAT_H

#include <dframework/base/Retval.h>
#include <dframework/util/D_Sorted.h>

namespace dframework{

  class Float : public Object
  {
  private:
      float m_value;

  public:
      Float();
      Float(float val);
      Float(Float& val);
      inline virtual ~Float() {}

      inline float value() const { return m_value; }

      static float parseFloat(const char* str, float defaultVal=0.f);
      inline static float parseFloat(const String& str, float defaultVal=0.f){
          return parseFloat(str.toChars(), defaultVal);
      }

      virtual Float& operator = (const float val);
      virtual Float& operator = (const Float& val);

      DFW_OPERATOR_EX_DECLARATION(Float, m_value);
  };

};

#endif /* DFRAMEWORK_LANG_FLOAT_H */

