// relative_error function
#pragma once

#include <othercore/math/max.h>
#include <othercore/vector/ScalarPolicy.h>
namespace other {

template<class T> typename boost::enable_if<IsScalar<T>,T>::type relative_error(const T& a, const T& b, const T& absolute=1e-30) {
  return abs(a-b)/max(abs(a),abs(b),absolute);
}

template<class TV> typename TV::Scalar relative_error(const TV& a, const TV& b, const typename TV::Scalar absolute=1e-30) {
  return (a-b).maxabs()/max(a.maxabs(),b.maxabs(),absolute);
}

}
