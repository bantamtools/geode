//#####################################################################
// Header utility/forward
//#####################################################################
#pragma once

#include <othercore/utility/config.h>
namespace boost {
template<class T> class shared_ptr;
}
namespace other {

struct Hasher;
template<class T> class Ptr;
template<class T> class Ref;

// Convenience struct for marking that function semantics have changed
struct Mark {};

// Convenience utility for hiding a type from use in overload resolution
template<class T> struct Hide {
  typedef T type;
};

// Return the first type given (for use in SFINAE)
template<class T0,class T1> struct First {
  typedef T0 type;
};

// A list of types
#ifdef OTHER_VARIADIC
template<class... Args> struct Types {
  typedef Types type;
};
#else
template<class A0=void,class A1=void,class A2=void,class A3=void,class A4=void,class A5=void,class A6=void,class A7=void,class A8=void,class A9=void> struct Types {
  typedef Types type;
};
#endif

// Null pointer convenience class
struct null {
  template<class T> operator Ptr<T>() {
    return Ptr<T>();
  }

  template<typename T> operator boost::shared_ptr<T>() {
    return boost::shared_ptr<T>();
  }
};

// OTHER_REMOVE_PARENS((a,b,c)) = a,b,c
#define OTHER_REMOVE_PARENS_HELPER(...) __VA_ARGS__
#define OTHER_REMOVE_PARENS(arg) OTHER_REMOVE_PARENS_HELPER arg

// Print a type at compile time
#define OTHER_PRINT_TYPE(...) typedef typename other::Types<__VA_ARGS__>::_print _print;

}
