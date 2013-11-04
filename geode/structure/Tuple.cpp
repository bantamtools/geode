#include <geode/structure/Tuple.h>
namespace geode {

#ifdef GEODE_PYTHON
void throw_tuple_mismatch_error(int expected, int got) {
  PyErr_Format(PyExc_TypeError,"expected tuple of length %d, got %d",expected,got);
  throw_python_error();
}
#endif

}
