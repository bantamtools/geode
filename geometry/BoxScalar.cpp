//#####################################################################
// Class Box<T>
//#####################################################################
#include <othercore/geometry/BoxScalar.h>
#include <othercore/python/exceptions.h>
#include <othercore/structure/Tuple.h>
#include <othercore/vector/Vector.h>
namespace other {

#ifdef OTHER_PYTHON

template<class T> PyObject* to_python(const Box<T>& self) {
  return to_python(tuple(self.min,self.max));
}

template<class T> Box<T> FromPython<Box<T> >::convert(PyObject* object) {
  const auto extents = from_python<Tuple<T,T>>(object);
  return Box<T>(extents.x,extents.y);
}

#define INSTANTIATE(T) \
  template OTHER_CORE_EXPORT PyObject* to_python(const Box<T>&); \
  template OTHER_CORE_EXPORT Box<T> FromPython<Box<T> >::convert(PyObject*);
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(int)
INSTANTIATE(int64_t)

#endif

}
