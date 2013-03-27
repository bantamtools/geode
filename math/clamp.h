//#####################################################################
// Function clamp
//#####################################################################
#pragma once

#include <othercore/math/min.h>
#include <othercore/math/max.h>
namespace other {

template<class T> static inline T clamp(const T x, const T xmin, const T xmax) {
  if (x<=xmin) return xmin;
  else if (x>=xmax) return xmax;
  else return x;
}

template<class T> static inline T clamp_min(const T x, const T xmin) {
  return max(x,xmin);
}

template<class T> static inline T clamp_max(const T x, const T xmax) {
  return min(x,xmax);
}

template<class T> static inline bool in_bounds(const T x, const T xmin, const T xmax) {
  return xmin<=x && x<=xmax;
}

}
