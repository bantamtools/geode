//#####################################################################
// Module Random
//#####################################################################
#include <othercore/python/wrap.h>
using namespace other;

void wrap_random() {
  OTHER_WRAP(Random)
  OTHER_WRAP(sobol)
  OTHER_WRAP(counter)
  OTHER_WRAP(permute)
}
