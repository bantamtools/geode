#include <geode/python/wrap.h>

void wrap_solver() {
  GEODE_WRAP(brent)
  GEODE_WRAP(powell)
}
