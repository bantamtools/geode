//#####################################################################
// Various python interface utilities
//#####################################################################
#pragma once
#ifdef GEODE_PYTHON

#include <geode/python/config.h>
#include <geode/python/Object.h>
#include <geode/python/to_python.h>
#include <geode/python/from_python.h>
namespace geode {

template<class T> static inline Ref<> to_python_check(const T& o) {
  return steal_ref_check(to_python(o));
}

template<class T> static inline bool python_has_field(const T& o, const char* name) {
  return PyObject_HasAttrString(&*to_python_check(o),name)!=0;
}

template<class T> static inline Ref<> python_field(const T& o, const char* name) {
  return steal_ref_check(PyObject_GetAttrString(&*to_python_check(o),name));
}

// Given an ITP<n,T> pair, extract the given index and convert to the desired type
template<class A> static inline auto convert_item(PyObject* args)
  -> decltype(from_python<typename A::type>((PyObject*)0)) {
  return from_python<typename A::type>(PyTuple_GET_ITEM(args,A::index));
}

#ifdef GEODE_VARIADIC

// Call a python function
template<class T,class... Args> static inline Ref<> python_call(const T& f, Args&&... args) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(args)...,0));
}

// Call a python method
template<class T,class... Args> static inline Ref<> python_call_method(const T& o, const char* name, Args&&... args) {
  return python_call(python_field(o,name),args...);
}

#else // Unpleasant nonvariadic versions

template<class T> static inline Ref<> python_call(const T& f) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),0));
}

template<class T,class A0> static inline Ref<> python_call(const T& f, A0&& a0) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(a0),0));
}

template<class T,class A0,class A1> static inline Ref<> python_call(const T& f, A0&& a0, A1&& a1) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(a0),&*to_python_check(a1),0));
}

template<class T,class A0,class A1,class A2> static inline Ref<> python_call(const T& f, A0&& a0, A1&& a1, A2&& a2) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(a0),&*to_python_check(a1),&*to_python_check(a2),0));
}

template<class T,class A0,class A1,class A2,class A3> static inline Ref<> python_call(const T& f, A0&& a0, A1&& a1, A2&& a2, A3&& a3) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(a0),&*to_python_check(a1),&*to_python_check(a2),&*to_python_check(a3),0));
}

template<class T,class A0,class A1,class A2,class A3,class A4> static inline Ref<> python_call(const T& f, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) {
  return steal_ref_check(PyObject_CallFunctionObjArgs(&*to_python_check(f),&*to_python_check(a0),&*to_python_check(a1),&*to_python_check(a2),&*to_python_check(a3),&*to_python_check(a4),0));
}

template<class T> static inline Ref<> python_call_method(const T& o, const char* name) {
  return python_call(python_field(o,name));
}

template<class T,class A0> static inline Ref<> python_call_method(const T& o, const char* name, A0&& a0) {
  return python_call(python_field(o,name),a0);
}

template<class T,class A0,class A1> static inline Ref<> python_call_method(const T& o, const char* name, A0&& a0, A1&& a1) {
  return python_call(python_field(o,name),a0,a1);
}

template<class T,class A0,class A1,class A2> static inline Ref<> python_call_method(const T& o, const char* name, A0&& a0, A1&& a1, A2&& a2) {
  return python_call(python_field(o,name),a0,a1,a2);
}

template<class T,class A0,class A1,class A2,class A3> static inline Ref<> python_call_method(const T& o, const char* name, A0&& a0, A1&& a1, A2&& a2, A3&& a3) {
  return python_call(python_field(o,name),a0,a1,a2,a3);
}

template<class T,class A0,class A1,class A2,class A3,class A4> static inline Ref<> python_call_method(const T& o, const char* name, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) {
  return python_call(python_field(o,name),a0,a1,a2,a3,a4);
}

#define GEODE_CONVERT_ARGS_0 
#define GEODE_CONVERT_ARGS_1                      from_python<A0>(PyTuple_GET_ITEM(args,0))
#define GEODE_CONVERT_ARGS_2 GEODE_CONVERT_ARGS_1,from_python<A1>(PyTuple_GET_ITEM(args,1))
#define GEODE_CONVERT_ARGS_3 GEODE_CONVERT_ARGS_2,from_python<A2>(PyTuple_GET_ITEM(args,2))
#define GEODE_CONVERT_ARGS_4 GEODE_CONVERT_ARGS_3,from_python<A3>(PyTuple_GET_ITEM(args,3))
#define GEODE_CONVERT_ARGS_5 GEODE_CONVERT_ARGS_4,from_python<A4>(PyTuple_GET_ITEM(args,4))
#define GEODE_CONVERT_ARGS_6 GEODE_CONVERT_ARGS_5,from_python<A5>(PyTuple_GET_ITEM(args,5))
#define GEODE_CONVERT_ARGS_7 GEODE_CONVERT_ARGS_6,from_python<A6>(PyTuple_GET_ITEM(args,6))
#define GEODE_CONVERT_ARGS_8 GEODE_CONVERT_ARGS_7,from_python<A7>(PyTuple_GET_ITEM(args,7))
#define GEODE_CONVERT_ARGS_9 GEODE_CONVERT_ARGS_8,from_python<A8>(PyTuple_GET_ITEM(args,8))

#endif

}
#endif
