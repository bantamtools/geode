//#####################################################################
// Function format
//#####################################################################
//
// Similar to boost::format, with the following differences:
//
// 1. Not as powerful (no format("%g")%vector) or as safe.
// 2. More concise.
// 3. Doesn't add 100k to every object file.
//
// The main advantage over raw varargs is that we can pass string, and
// we can add more safety features later if we feel like it.
//
//#####################################################################
#pragma once

#include <geode/utility/config.h>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <string>
namespace geode {

namespace mpl = boost::mpl;
using std::string;

// Unfortunately, since format_helper is called indirectly through format, we can't use gcc's format attribute.
GEODE_CORE_EXPORT string format_helper(const char* format,...);

template<class T> static inline typename mpl::if_<boost::is_enum<T>,int,T>::type format_sanitize(const T d) {
  // Ensure that passing as a vararg is safe
  BOOST_MPL_ASSERT((mpl::or_<boost::is_fundamental<T>,boost::is_enum<T>,boost::is_pointer<T>>));
  return d;
}

static inline const char* format_sanitize(char* s) {
  return s;
}

static inline const char* format_sanitize(const char* s) {
  return s;
}

static inline const char* format_sanitize(const string& s) {
  return s.c_str();
}

#ifdef GEODE_VARIADIC

template<class... Args> static inline string format(const char* format, const Args&... args) {
  return format_helper(format,format_sanitize(args)...);
}

#else // Unpleasant nonvariadic versions

static inline string format(const char* format) {
  return format_helper(format);
}

template<class A0> static inline string format(const char* format, const A0& a0) {
  return format_helper(format,format_sanitize(a0));
}

template<class A0,class A1> static inline string format(const char* format, const A0& a0, const A1& a1) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1));
}

template<class A0,class A1,class A2> static inline string format(const char* format, const A0& a0, const A1& a1, const A2& a2) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2));
}

template<class A0,class A1,class A2,class A3> static inline string format(const char* format, const A0& a0, const A1& a1, const A2& a2, const A3& a3) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3));
}

template<class A0,class A1,class A2,class A3,class A4> static inline string format(const char* format, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3),format_sanitize(a4));
}

template<class A0,class A1,class A2,class A3,class A4,class A5> static inline string format(const char* format, const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3),format_sanitize(a4),format_sanitize(a5));
}

template<class A0,class A1,class A2,class A3,class A4,class A5,class A6> static inline string format(const char* format,
  const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3),format_sanitize(a4),format_sanitize(a5),format_sanitize(a6));
}

template<class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7> static inline string format(const char* format,
  const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3),format_sanitize(a4),format_sanitize(a5),format_sanitize(a6),format_sanitize(a7));
}

template<class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7,class A8> static inline string format(const char* format,
  const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) {
  return format_helper(format,format_sanitize(a0),format_sanitize(a1),format_sanitize(a2),format_sanitize(a3),format_sanitize(a4),format_sanitize(a5),format_sanitize(a6),format_sanitize(a7),format_sanitize(a8));
}

#endif

}
