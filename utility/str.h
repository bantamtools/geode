//#####################################################################
// Stream based convertion from T to str
//#####################################################################
//
// This is similar to boost::lexical_cast, but simpler and detects
// operator<< overloads that exist only in the other namespace.
//
//#####################################################################
#pragma once

#include <sstream>
#include <othercore/utility/stl.h>
#include <othercore/python/Ref.h>
namespace other{

using std::string;

static inline string str() {
  return string();
}

template<class T> string str(const T& x) {
  std::ostringstream os;
  os << x;
  return os.str();
}

static inline string str(const string& x) {
  return x;
}

static inline string str(const char* x) {
  return x;
}

}
