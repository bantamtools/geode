//#####################################################################
// Module Arrays
//#####################################################################
#include <othercore/array/Array2d.h>
#include <othercore/python/numpy.h>
#include <othercore/python/wrap.h>
using namespace other;

namespace {

Array<int> empty_array() {
  return Array<int>();
}

Array<int> array_test(Array<int> array, int resize) {
  Array<int> test;
  test = array;
  if(resize>=0)
    test.resize(resize);
  return test;
}

Array<const int> const_array_test(Array<const int> array) {
  Array<const int> test;
  test = array;
  return test;
}

#ifdef OTHER_PYTHON

ssize_t base_refcnt(PyObject* array) {
  OTHER_ASSERT(PyArray_Check(array));
  PyObject* base = PyArray_BASE((PyArrayObject*)array);
  return base?base->ob_refcnt:0;
}

Array<uint8_t> array_write_test(const string& filename, RawArray<const real,2> array) {
  write_numpy(filename,array);
  Array<uint8_t> header;
  fill_numpy_header(header,array);
  return header;
}

#endif

}

void wrap_array() {
  OTHER_WRAP(nested_array)

  // for testing purposes
  OTHER_FUNCTION(empty_array)
  OTHER_FUNCTION(array_test)
  OTHER_FUNCTION(const_array_test)
#ifdef OTHER_PYTHON
  OTHER_FUNCTION(base_refcnt)
  OTHER_FUNCTION(array_write_test)
#endif
}
