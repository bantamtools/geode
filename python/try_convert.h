// Convert to and from python if possible, otherwise throw an exception
#pragma once

#include <othercore/python/to_python.h>
#include <othercore/python/from_python.h>
#include <othercore/utility/forward.h>
#include <boost/utility/enable_if.hpp>
namespace other {

namespace mpl = boost::mpl;

#ifdef OTHER_PYTHON

OTHER_CORE_EXPORT void set_to_python_failed(const type_info& type);
OTHER_CORE_EXPORT void OTHER_NORETURN(from_python_failed(PyObject* object, const type_info& type));

template<class T> static inline typename boost::enable_if<has_to_python<T>,PyObject*>::type try_to_python(const T& x) {
  return to_python(x);
}

template<class T> static inline typename boost::disable_if<has_to_python<T>,PyObject*>::type try_to_python(const T& x) {
  set_to_python_failed(typeid(T));
  return 0;
}

template<class T> static inline typename boost::enable_if<has_from_python<T>,T>::type try_from_python(PyObject* object) {
  return from_python<T>(object);
}

template<class T> static inline typename boost::disable_if<has_from_python<T>,T>::type try_from_python(PyObject* object) {
  from_python_failed(object,typeid(T));
}

#endif

}
