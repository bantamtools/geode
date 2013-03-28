//#####################################################################
// Function wrap_method
//#####################################################################
//
// Converts a C++ member function into a python function.  It is normally used indirectly through Class<T>.
//
// In order to convert a function of type R(...,Ai,...), there must be From_Python overloads converting PyObject* to Ai,
// and a to_python function converting R to PyObject*.  See to_python.h and from_python.h for details.
//
// Since python has no notion of constness, there is no difference between the wrapped versions of a method with and
// without const qualification.
//
// note: method_inner_wrapper_* unfortunately can't be declared static because gcc disallows static functions as template
// arguments.  Putting it in an unnamed namespace clutters up the stack traces, so we rely on hidden visibility.
//
//#####################################################################
#pragma once

#include <othercore/python/config.h>
#include <othercore/python/exceptions.h>
#include <othercore/python/from_python.h>
#include <othercore/python/to_python.h>
#include <othercore/python/outer_wrapper.h>
#include <othercore/utility/config.h>
#include <othercore/utility/Enumerate.h>
#include <boost/mpl/bool.hpp>
namespace other {

namespace mpl = boost::mpl;

OTHER_CORE_EXPORT PyObject* wrap_method_helper(PyTypeObject* type,const char* name,wrapperfunc wrapper,void* method);

#ifdef OTHER_VARIADIC

// Can't be static because static functions can't be template arguments
template<class M,class R,class T,class... Args> R
method_inner_wrapper(PyObject* self,PyObject* args,void* method) {
  Py_ssize_t size = PyTuple_GET_SIZE(args);
  const int desired = sizeof...(Args);
  if (size!=desired) throw_arity_mismatch(desired,size);
  return (GetSelf<T>::get(self)->*(*(M*)method))(convert_item<Args>(args)...);
}

// wrap_method for static methods
template<class T,class M,class R,class... Args> static PyObject*
wrap_method(const char* name,R (*method)(Args...)) {
  return wrap_function(name,method);
}

template<class T,class M,class R,class... Args> static wrapperfunc wrapped_method(Types<Args...>) {
  return OuterWrapper<R,PyObject*,PyObject*,void*>::template wrap<method_inner_wrapper<M,R,T,Args...> >;
}

// wrap_method for nonconst methods
template<class T,class M,class R,class B,class... Args> static PyObject*
wrap_method(const char* name,R (B::*method)(Args...)) {
  return wrap_method_helper(&T::pytype,name,wrapped_method<T,M,R>(typename Enumerate<Args...>::type()),(void*)new M(method));
}

// wrap_method for const methods
template<class T,class M,class R,class B,class... Args> static PyObject*
wrap_method(const char* name,R (B::*method)(Args...) const) {
  return wrap_method_helper(&T::pytype,name,wrapped_method<T,M,R>(typename Enumerate<Args...>::type()),(void*)new M(method));
}

#else // Unpleasant nonvariadic versions

#define OTHER_WRAP_METHOD(n,ARGS,Args) \
  OTHER_WRAP_METHOD_2(n,(,OTHER_REMOVE_PARENS(ARGS)),(,OTHER_REMOVE_PARENS(Args)),Args)

template<class T,class M> struct DerivedMethod {
  typedef M type;
};

#define OTHER_WRAP_METHOD_2(n,CARGS,CArgs,Args) \
  /* Can't be static because static functions can't be template arguments */ \
  template<class M,class R,class T OTHER_REMOVE_PARENS(CARGS)> R \
  method_inner_wrapper_##n(PyObject* self,PyObject* args,void* method) { \
    Py_ssize_t size = PyTuple_GET_SIZE(args); \
    const int desired = n; \
    if (size!=desired) throw_arity_mismatch(desired,size); \
    return (GetSelf<T>::get(self)->*(*(M*)method))(OTHER_CONVERT_ARGS_##n); \
  } \
  \
  /* wrap_method for static methods */ \
  template<class T,class Self,class R OTHER_REMOVE_PARENS(CARGS)> static PyObject* \
  wrap_method(const char* name,R (*method) Args) { \
    return wrap_function(name,method); \
  } \
  \
  /* wrap_method for nonconst methods */ \
  template<class T,class B,class R OTHER_REMOVE_PARENS(CARGS)> struct DerivedMethod<T,R(B::*) Args> { \
    typedef R (T::*type) Args; \
  }; \
  template<class T,class Self,class R OTHER_REMOVE_PARENS(CARGS)> static PyObject* \
  wrap_method(const char* name,R (Self::*method) Args) { \
    typedef R (Self::*M) Args; \
    return wrap_method_helper(&T::pytype,name,OuterWrapper<R,PyObject*,PyObject*,void*>::template wrap<method_inner_wrapper_##n<M,R,T OTHER_REMOVE_PARENS(CArgs)>>,(void*)new M(method)); \
  } \
  \
  /* wrap_method for const methods */ \
  template<class T,class B,class R OTHER_REMOVE_PARENS(CARGS)> struct DerivedMethod<T,R(B::*) Args const> { \
    typedef R (T::*type) Args const; \
  }; \
  template<class T,class Self,class R OTHER_REMOVE_PARENS(CARGS)> static PyObject* \
  wrap_method(const char* name,R (Self::*method) Args const) { \
    typedef R (Self::*M) Args const; \
    return wrap_method_helper(&T::pytype,name,OuterWrapper<R,PyObject*,PyObject*,void*>::template wrap<method_inner_wrapper_##n<M,R,T OTHER_REMOVE_PARENS(CArgs)>>,(void*)new M(method)); \
  }

OTHER_WRAP_METHOD_2(0,(),(),())
OTHER_WRAP_METHOD(1,(class A0),(A0))
OTHER_WRAP_METHOD(2,(class A0,class A1),(A0,A1))
OTHER_WRAP_METHOD(3,(class A0,class A1,class A2),(A0,A1,A2))
OTHER_WRAP_METHOD(4,(class A0,class A1,class A2,class A3),(A0,A1,A2,A3))
OTHER_WRAP_METHOD(5,(class A0,class A1,class A2,class A3,class A4),(A0,A1,A2,A3,A4))
OTHER_WRAP_METHOD(6,(class A0,class A1,class A2,class A3,class A4,class A5),(A0,A1,A2,A3,A4,A5))
OTHER_WRAP_METHOD(7,(class A0,class A1,class A2,class A3,class A4,class A5,class A6),(A0,A1,A2,A3,A4,A5,A6))
OTHER_WRAP_METHOD(8,(class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7),(A0,A1,A2,A3,A4,A5,A6,A7))

#undef OTHER_WRAP_METHOD_2
#undef OTHER_WRAP_METHOD

#endif

}
