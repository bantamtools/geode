//#####################################################################
// Module core
//#####################################################################
#include <othercore/python/module.h>
#include <othercore/utility/interrupts.h>
using namespace other;

OTHER_PYTHON_MODULE(other_core) {
  OTHER_WRAP(python)
  OTHER_WRAP(utility)

#ifdef OTHER_PYTHON
  // Check for python exceptions in check_interrupts()
  add_interrupt_checker(check_python_interrupts);
#endif

  OTHER_WRAP(math)
  OTHER_WRAP(array)
  OTHER_WRAP(vector)
  OTHER_WRAP(geometry)
  OTHER_WRAP(image)
  OTHER_WRAP(mesh)
  OTHER_WRAP(random)
  OTHER_WRAP(value)
  OTHER_WRAP(force)
  OTHER_WRAP(exact)
  OTHER_WRAP(solver)
  OTHER_WRAP(openmesh)
}
