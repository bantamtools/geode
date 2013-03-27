//#####################################################################
// Python conversions for Array and NdArray
//#####################################################################
//
// Include this header if you need to register array conversion for new types.
//
//#####################################################################
#include <othercore/array/Array.h>
#include <othercore/array/Array2d.h>
#include <othercore/array/Array3d.h>
#include <othercore/array/Array4d.h>
#include <othercore/array/NdArray.h>
#include <othercore/python/numpy.h>
namespace other {

#ifdef OTHER_PYTHON

template<class T,int d> PyObject* to_python(const Array<T,d>& array) {
  return to_numpy(array);
}

template<class T,int d> Array<T,d> FromPython<Array<T,d> >::convert(PyObject* object) {
  return from_numpy<Array<T,d> >(object);
}

#define ARRAY_CONVERSIONS_HELPER(d,...) \
  template OTHER_CORE_EXPORT PyObject* to_python<__VA_ARGS__,d>(const Array<__VA_ARGS__,d>&); \
  template OTHER_CORE_EXPORT Array<__VA_ARGS__,d> FromPython<Array<__VA_ARGS__,d> >::convert(PyObject*);
#define ARRAY_CONVERSIONS(d,...) \
  ARRAY_CONVERSIONS_HELPER(d,__VA_ARGS__) \
  ARRAY_CONVERSIONS_HELPER(d,const __VA_ARGS__)

template<class T> PyObject* to_python(const NdArray<T>& array) {
  return to_numpy(array);
}

template<class T> NdArray<T> FromPython<NdArray<T> >::convert(PyObject* object) {
  return from_numpy<NdArray<T>>(object);
}

#define NDARRAY_CONVERSIONS_HELPER(...) \
  template OTHER_CORE_EXPORT PyObject* to_python<__VA_ARGS__>(const NdArray<__VA_ARGS__>&); \
  template OTHER_CORE_EXPORT NdArray<__VA_ARGS__> FromPython<NdArray<__VA_ARGS__> >::convert(PyObject*);
#define NDARRAY_CONVERSIONS(...) \
  NDARRAY_CONVERSIONS_HELPER(__VA_ARGS__) \
  NDARRAY_CONVERSIONS_HELPER(const __VA_ARGS__)

#else // non-python stubs

#define ARRAY_CONVERSIONS(...)
#define NDARRAY_CONVERSIONS(...)

#endif

}
