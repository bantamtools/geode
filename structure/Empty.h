//#####################################################################
// Class Tuple<>
//#####################################################################
#pragma once

#include <othercore/structure/forward.h>
#include <iostream> // needed to avoid header bug on Mac OS X
#include <othercore/math/choice.h>
#include <othercore/math/hash.h>
#include <othercore/python/from_python.h>
#include <othercore/python/to_python.h>
#include <othercore/utility/stream.h>
namespace other {

template<>
class Tuple<> {
public:
  bool operator==(const Tuple p) const {
    return true;
  }

  bool operator!=(const Tuple p) const {
    return false;
  }

  bool operator<(const Tuple p) const {
    return false;
  }

  bool operator>(const Tuple p) const {
    return false;
  }

  void get() const {}
};

static inline std::istream& operator>>(std::istream& input,Tuple<> p) {
  return input>>expect('(')>>expect(')');
}

static inline std::ostream& operator<<(std::ostream& output,Tuple<> p) {
  return output<<"()";
}

static inline int hash_reduce(Tuple<> key) {
  return 0;
}

}
