// Quadratic formula
#pragma once

#include <othercore/math/copysign.h>
#include <othercore/vector/Vector.h>
namespace other {

template<class T> static inline Vector<T,2> solve_quadratic(const T a, const T b, const T c) {
  const T disc = sqr(b)-4*a*c;
  if (disc < 0) {
    const T nan = numeric_limits<T>::quiet_NaN();
    return vec(nan,nan);
  }
  const T num = -.5*(b+copysign(sqrt(disc),b));
  return vec(num/a,c/num);
}

}
