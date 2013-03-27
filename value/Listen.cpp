#include "Listen.h"
#include <othercore/python/Class.h>
#include <othercore/python/function.h>
#include <othercore/python/wrap.h>
namespace other {

OTHER_DEFINE_TYPE(Listen)

Listen::Listen(const ValueBase& value, const function<void()>& f)
  : value(ref(value))
  , f(f) {
  depend_on(value);
}

Listen::~Listen() {}

void Listen::input_changed() const {
  try {
    Executing e; // register no dependencies during execution
    f();
  } catch (const exception& e) {
    print_and_clear_exception("Listen: exception in listener callback",e);
  }
  depend_on(*value);
}

Ref<Listen> listen(const ValueBase& value, const function<void()>& f) {
  return new_<Listen>(value,f);
}

}
using namespace other;

void wrap_listen() {
  typedef Listen Self;
  Class<Self>("Listen");
  OTHER_FUNCTION_2(listen, static_cast<Ref<Listen>(*)(const ValueBase&,const function<void()>&)>(listen))
}
