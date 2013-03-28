//#####################################################################
// Class Matrix
//#####################################################################
#include <othercore/vector/Matrix.h>
#include <othercore/array/convert.h>
#include <othercore/python/numpy.h>
#include <othercore/python/wrap.h>
namespace other {

#ifdef OTHER_PYTHON

static PyTypeObject* matrix_type;

namespace {
template<class T,int m,int n> struct NumpyArrayType<Matrix<T,m,n> >{static PyTypeObject* type(){return matrix_type;}};
}

static void set_matrix_type(PyObject* type) {
  OTHER_ASSERT(PyType_Check(type));
  Py_INCREF(type);
  matrix_type = (PyTypeObject*)type;
}

template<class T,int m,int n> PyObject*
to_python(const Matrix<T,m,n>& matrix) {
  return to_numpy(matrix);
}

template<class T,int m,int n> Matrix<T,m,n> FromPython<Matrix<T,m,n> >::
convert(PyObject* object) {
  return from_numpy<Matrix<T,n,m> >(object);
}

#define INSTANTIATE(T,m,n) \
  template OTHER_CORE_EXPORT PyObject* to_python<T,m,n>(const Matrix<T,m,n>&); \
  template OTHER_CORE_EXPORT Matrix<T,m,n> FromPython<Matrix<T,m,n> >::convert(PyObject*); \
  ARRAY_CONVERSIONS(1,Matrix<T,m,n>)
INSTANTIATE(real,2,2)
INSTANTIATE(real,3,3)
INSTANTIATE(real,4,4)

#endif

}
using namespace other;

void wrap_matrix() {
#ifdef OTHER_PYTHON
  using namespace python;
  function("_set_matrix_type",set_matrix_type);
#endif
}
