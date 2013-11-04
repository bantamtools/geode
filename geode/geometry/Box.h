//#####################################################################
// Class Box
//#####################################################################
//
// Box<TV> is (conservatively) exact for set operations, but *inexact*
// for arithmetic.  See geode/exact if you need exact operations.
//
//#####################################################################
#pragma once

#include <geode/utility/config.h>
#include <geode/array/RawArray.h>
#include <geode/geometry/BoxScalar.h>
#include <geode/geometry/BoxVector.h>
#include <boost/type_traits/remove_const.hpp>
namespace geode {

using std::numeric_limits;

#ifdef GEODE_VARIADIC

template<class TV,class... Rest> static inline Box<TV> bounding_box(const TV& p0, const TV& p1, const Rest&... rest) {
  Box<TV> box(p0);
  box.enlarge_nonempty(p1,rest...);
  return box;
}

#else // Unpleasant nonvariadic versions

template<class TV> static inline Box<TV> bounding_box(const TV& p0, const TV& p1) {
  Box<TV> box(p0);
  box.enlarge_nonempty(p1);
  return box;
}
template<class TV> static inline Box<TV> bounding_box(const TV& p0, const TV& p1, const TV& p2) {
  Box<TV> box(p0);
  box.enlarge_nonempty(p1,p2);
  return box;
}
template<class TV> static inline Box<TV> bounding_box(const TV& p0, const TV& p1, const TV& p2, const TV& p3) {
  Box<TV> box(p0);
  box.enlarge_nonempty(p1,p2,p3);
  return box;
}

#endif

template<class TArray> Box<typename TArray::value_type> bounding_box(const TArray& points) {
  typedef typename TArray::value_type T;
  if (!points.size())
    return Box<T>::empty_box();
  Box<T> box(points[0]);
  for (int i=1;i<(int)points.size();i++)
    box.enlarge_nonempty(points[i]);
  return box;
}

// Instantiate common cases in Box.cpp
#ifndef _WIN32
extern template GEODE_CORE_EXPORT Box<real> bounding_box(const RawArray<const real>&);
extern template GEODE_CORE_EXPORT Box<Vector<real,2>> bounding_box(const RawArray<const Vector<real,2>>&);
extern template GEODE_CORE_EXPORT Box<Vector<real,3>> bounding_box(const RawArray<const Vector<real,3>>&);
#endif

template<class T> static inline Box<typename Array<T>::value_type> bounding_box(const Array<T>& points) {
  return bounding_box(RawArray<typename boost::add_const<T>::type>(points));
}

template<class T> static inline Box<typename boost::remove_const<T>::type> bounding_box(const Nested<T>& points) {
  return bounding_box(points.flat);
}

template<class TV> static inline Box<TV> operator+(const TV& a, const Box<TV>& b) {
  return Box<TV>(a+b.min,a+b.max);
}

template<class TV> static inline Box<TV> operator-(const TV& a, const Box<TV>& b) {
  return Box<TV>(a-b.max,a-b.min);
}

template<class TV> static inline Box<TV> operator*(const typename ScalarPolicy<TV>::type a, const Box<TV>& box) {
  return box*a;
}

template<class TV> static inline Box<TV> exp(const Box<TV>& x) {
  return Box<TV>(exp(x.min),exp(x.max));
}

template<class TV> inline std::ostream& operator<<(std::ostream& output, const Box<TV>& box) {
  return output<<'['<<box.min<<','<<box.max<<']';
}

template<class TV> inline std::istream& operator>>(std::istream& input,Box<TV>& box) {
  return input>>expect('[')>>box.min>>expect(',')>>box.max>>expect(']');
}

}
