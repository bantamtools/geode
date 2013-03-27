#include <othercore/value/ConstValue.h>
#include <othercore/python/function.h>
#include <othercore/python/Ptr.h>
#include <othercore/python/wrap.h>
using namespace other;

void wrap_const_value() {
  python::function("const_value",const_value<Ptr<> >);
}
