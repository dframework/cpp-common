#include <dframework/util/Util.h>

namespace dframework {

  String Util::SizeToString(uint64_t size){
      static const uint64_t ms = 1024*1024;
      static const uint64_t gs = ms*1024;
      static const uint64_t ts = gs*1024;
      static const uint64_t ps = ts*1024;
      uint64_t v;

      if( size < 1000 )
          return String::format("%4dB", size);

      v = size / 1024;
      if( v < 10 )
          return String::format("%.03fK", SizeToString_l(size, 1024));
      if( v < 100 )
          return String::format("%.02fK", SizeToString_l(size, 1024));
      if( v < 1000 )
          return String::format("%.1fK", SizeToString_l(size, 1024));
      if( v < 10000 )
          return String::format("%4.0fK", SizeToString_l(size, 1024));

      v = size / ms;
      if( v < 10 )
          return String::format("%.03fM", SizeToString_l(size, ms));
      if( v < 100 )
          return String::format("%.02fM", SizeToString_l(size, ms));
      if( v < 1000 )
          return String::format("%.1fM", SizeToString_l(size, ms));
      if( v < 10000 )
          return String::format("%4.0fM", SizeToString_l(size, ms));

      v = size / (gs);
      if( v < 10 )
          return String::format("%.03fG", SizeToString_l(size, gs));
      if( v < 100 )
          return String::format("%.02fG", SizeToString_l(size, gs));
      if( v < 1000 )
          return String::format("%.1fG", SizeToString_l(size, gs));
      if( v < 10000 )
          return String::format("%4.0fG", SizeToString_l(size, gs));

      v = size / (ts);
      if( v < 10 )
          return String::format("%.03fT", SizeToString_l(size, ts));
      if( v < 100 )
          return String::format("%.02fT", SizeToString_l(size, ts));
      if( v < 1000 )
          return String::format("%.1fT", SizeToString_l(size, ts));
      if( v < 10000 )
          return String::format("%4.0fT", SizeToString_l(size, ts));

      v = size / (ps);
      if( v < 10 )
          return String::format("%.03fP", SizeToString_l(size, ps));
      if( v < 100 )
          return String::format("%.02fP", SizeToString_l(size, ps));
      if( v < 1000 )
          return String::format("%.1fP", SizeToString_l(size, ps));

      return String::format("%.0fP", SizeToString_l(size, ps));
  }

};


