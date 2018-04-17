//#####################################################################
// Function wrap_function
//#####################################################################
//
// Converts a free C++ function into a python function.  It is normally used indirectly through function() (see Module.h).
//
// In order to convert a function of type R(...,Ai,...), there must be from_python overloads converting PyObject* to Ai,
// and a to_python function converting R to PyObject*.  See to_python.h and from_python.h for details.
//
// note: function_inner_wrapper unfortunately can't be declared static because gcc disallows static functions as template
// arguments.  Putting it in an unnamed namespace clutters up the stack traces, so we rely on hidden visibility.
//
//#####################################################################
#pragma once

#include <geode/python/config.h>
#include <geode/python/exceptions.h>
#include <geode/python/outer_wrapper.h>
#include <geode/python/utility.h>
#include <geode/utility/config.h>
#include <geode/utility/enumerate.h>
namespace geode {

typedef PyObject* (*FunctionWrapper)(PyObject* args,void* wrapped);
GEODE_CORE_EXPORT PyObject* wrap_function_helper(const char* name,FunctionWrapper wrapper,void* function);

#ifdef GEODE_VARIADIC

// Can't be static because static functions can't be template arguments
template<class F,class R,class... Args> inline R
function_inner_wrapper(PyObject* args,void* wrapped) {
  Py_ssize_t size = PyTuple_GET_SIZE(args);
  const int desired = sizeof...(Args);
  if (size!=desired) throw_arity_mismatch(desired,size);
  return ((F)wrapped)(convert_item<Args>(args)...);
}

template<class F,class R,class... Args> static FunctionWrapper wrapped_function(Types<Args...>) {
  return OuterWrapper<R,PyObject*,void*>::template wrap<function_inner_wrapper<F,R,Args...>>;
}

template<class R,class... Args> static PyObject*
wrap_function(const char* name,R (*function)(Args...)) {
  return wrap_function_helper(name,wrapped_function<decltype(function),R>(typename Enumerate<Args...>::type()),(void*)function);
}

#else // Unpleasant nonvariadic versions

#define GEODE_WRAP_FUNCTION(n,ARGS,Args) \
  GEODE_WRAP_FUNCTION_2(n,(,GEODE_REMOVE_PARENS(ARGS)),(,GEODE_REMOVE_PARENS(Args)),Args)

#define GEODE_WRAP_FUNCTION_2(n,CARGS,CArgs,Args) \
  template<class F,class R GEODE_REMOVE_PARENS(CARGS)> inline R \
  function_inner_wrapper_##n(PyObject* args,void* wrapped) { \
    Py_ssize_t size = PyTuple_GET_SIZE(args); \
    const int desired = n; \
    if (size!=desired) throw_arity_mismatch(desired,size); \
    return ((F)wrapped)(GEODE_CONVERT_ARGS_##n); \
  } \
  \
  template<class R GEODE_REMOVE_PARENS(CARGS)> static PyObject* \
  wrap_function(const char* name,R (*function) Args) { \
    return wrap_function_helper(name,OuterWrapper<R,PyObject*,void*>::template wrap<function_inner_wrapper_##n<decltype(function),R GEODE_REMOVE_PARENS(CArgs)>>,(void*)function); \
  }

GEODE_WRAP_FUNCTION_2(0,(),(),())
GEODE_WRAP_FUNCTION(1,(class A0),(A0))
GEODE_WRAP_FUNCTION(2,(class A0,class A1),(A0,A1))
GEODE_WRAP_FUNCTION(3,(class A0,class A1,class A2),(A0,A1,A2))
GEODE_WRAP_FUNCTION(4,(class A0,class A1,class A2,class A3),(A0,A1,A2,A3))
GEODE_WRAP_FUNCTION(5,(class A0,class A1,class A2,class A3,class A4),(A0,A1,A2,A3,A4))
GEODE_WRAP_FUNCTION(6,(class A0,class A1,class A2,class A3,class A4,class A5),(A0,A1,A2,A3,A4,A5))
GEODE_WRAP_FUNCTION(7,(class A0,class A1,class A2,class A3,class A4,class A5,class A6),(A0,A1,A2,A3,A4,A5,A6))
GEODE_WRAP_FUNCTION(8,(class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7),(A0,A1,A2,A3,A4,A5,A6,A7))
GEODE_WRAP_FUNCTION(9,(class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7,class A8),(A0,A1,A2,A3,A4,A5,A6,A7,A8))

#undef GEODE_WRAP_FUNCTION_2
#undef GEODE_WRAP_FUNCTION

#endif

}
