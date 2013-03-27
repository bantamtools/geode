#include <othercore/value/Compute.h>
#include <othercore/python/from_python.h>
#include <othercore/python/Ptr.h>
#include <othercore/python/Class.h>
#include <othercore/utility/format.h>
namespace other {
#ifdef OTHER_PYTHON

static PyObject* empty_tuple = 0;

namespace {
class CachePython : public Value<Ptr<>>,public Action {
public:
  OTHER_DECLARE_TYPE(OTHER_NO_EXPORT)
  typedef ValueBase Base;

  const Ref<> f;

protected:
  CachePython(PyObject* f)
    : f(ref(*f)) {}
public:

  void input_changed() const {
    this->set_dirty();
  }

  void update() const {
    Executing e(*this);
    this->set_value(e.stop(steal_ref_check(PyObject_Call(&*f,empty_tuple,0))));
  }

  void dump(int indent) const {
    printf("%*scache(%s)\n",2*indent,"",name().c_str());
    Action::dump_dependencies(indent);
  }

  vector<Ref<const ValueBase>> dependencies() const {
    return Action::dependencies();
  }

  string name() const {
    return from_python<string>(python_field(f,"__name__"));
  }

  string repr() const {
    return format("cache(%s)",name());
  }
};

OTHER_DEFINE_TYPE(CachePython)
}

// We write a special version for python to allow easy introspection
static Ref<ValueBase> cache_py(PyObject* f) {
  if (!PyCallable_Check(f))
    throw TypeError("cache: argument is not callable");
  return new_<CachePython>(f);
}

#endif
}
using namespace other;

void wrap_compute() {
#ifdef OTHER_PYTHON
  empty_tuple = PyTuple_New(0);
  OTHER_ASSERT(empty_tuple);

  typedef CachePython Self;
  Class<Self>("Cache")
    .OTHER_FIELD(f)
    .OTHER_GET(name)
    .repr()
    ;

  OTHER_FUNCTION_2(cache,cache_py)
#endif
}
