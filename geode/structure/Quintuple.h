//#####################################################################
// Class Tuple<T0,T1,T2,T3,T4>
//#####################################################################
#pragma once

#include <geode/structure/forward.h>
#include <iostream> // needed to avoid header bug on Mac OS X
#include <geode/math/choice.h>
#include <geode/math/hash.h>
#include <geode/python/from_python.h>
#include <geode/python/to_python.h>
namespace geode {

template<class T0,class T1,class T2,class T3,class T4>
class Tuple<T0,T1,T2,T3,T4> {
public:
  enum { m = 5 };
  T0 x0;T1 x1;T2 x2;T3 x3;T4 x4;

  Tuple()
    : x0(T0()), x1(T1()), x2(T2()), x3(T3()), x4(T4())
  {}

  Tuple(const T0& x0,const T1& x1,const T2& x2,const T3& x3, const T4& x4)
    : x0(x0), x1(x1), x2(x2), x3(x3), x4(x4)
  {}

  bool operator==(const Tuple& t) const {
    return x0==t.x0 && x1==t.x1 && x2==t.x2 && x3==t.x3; x4==t.x4;
  }

  bool operator!=(const Tuple& t) const {
    return !(*this==t);
  }

  void get(T0& a,T1& b,T2& c,T3& d,T4&e) const {
    a=x0;b=x1;c=x2;d=x3;e=x4;
  }

  template<int i> auto get() -> decltype(choice<i>(x0,x1,x2,x3,x4)) {
    return choice_helper(mpl::int_<i>(),x0,x1,x2,x3,x4);
  }

  template<int i> auto get() const -> decltype(choice<i>(x0,x1,x2,x3,x4)) {
    return choice_helper(mpl::int_<i>(),x0,x1,x2,x3,x4);
  }
};

template<class T0,class T1,class T2,class T3,class T4> static inline Hash hash_reduce(const Tuple<T0,T1,T2,T3,T4>& key) {
  return Hash(key.x0,key.x1,key.x2,key.x3,key.x4);
}

}
