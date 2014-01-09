//#####################################################################
// Conversion for stl containers
//#####################################################################
//
// Warning: These operations copy the entire container, and are therefore fairly slow.
//
//#####################################################################
#pragma once

#include <geode/python/config.h>
#include <geode/python/Ptr.h>
#include <geode/python/from_python.h>
#include <geode/python/to_python.h>
#include <geode/utility/config.h>
#include <geode/utility/tr1.h>
#include <boost/mpl/and.hpp>
#include <vector>
#include <set>
#include <map>

#ifdef GEODE_PYTHON

namespace geode {
// Fix has_to/from_python for stl containers
template<class T> struct has_to_python<std::vector<T>> : public has_to_python<T> {};
template<class T> struct has_from_python<std::vector<T>> : public has_from_python<T> {};
template<class T,class O> struct has_to_python<std::set<T,O>> : public has_to_python<T> {};
template<class T,class O> struct has_from_python<std::set<T,O>> : public has_from_python<T> {};
template<class K,class V,class O> struct has_to_python<std::map<K,V,O>> : public mpl::and_<has_to_python<K>,has_to_python<V>> {};
template<class K,class V,class O> struct has_from_python<std::map<K,V,O>> : public mpl::and_<has_from_python<K>,has_from_python<V>> {};
template<class T,class H> struct has_to_python<std::tr1::unordered_set<T,H>> : public has_to_python<T> {};
template<class T,class H> struct has_from_python<std::tr1::unordered_set<T,H>> : public has_from_python<T> {};
template<class K,class V,class H> struct has_to_python<std::tr1::unordered_map<K,V,H>> : public mpl::and_<has_from_python<K>,has_from_python<V>> {};
}

// to_python needs to go in the std namespace to make Koenig lookup work
namespace std {

template<class T> PyObject* to_python(const vector<T>& v) {
  using namespace geode;
  using geode::to_python;
  PyObject* list = PyList_New(v.size());
  if (!list) return 0;
  for (size_t i=0;i<v.size();i++) {
    PyObject* o = to_python(v[i]);
    if (!o) {
      Py_DECREF(list);
      return 0;
    }
    PyList_SET_ITEM(list,i,o);
  }
  return list;
}

template<class T,class O> static inline PyObject* to_python(const set<T,O>& s) {
  return geode::to_python_set(s);
}

template<class T,class H> static inline PyObject* to_python(const tr1::unordered_set<T,H>& s) {
  return geode::to_python_set(s);
}

template<class TM> PyObject* to_python_map(const TM& m) {
  using namespace geode;
  using geode::to_python;
  PyObject* dict = PyDict_New();
  if (!dict) goto fail;
  for (auto it=m.begin(),end=m.end();it!=end;++it) { // Avoid foreach since pcl needs gcc 4.4
    PyObject* k = to_python(it->first);
    if (!k) goto fail;
    PyObject* v = to_python(it->second);
    if (!v) {
      Py_DECREF(k);
      goto fail;
    }
    int r = PyDict_SetItem(dict,k,v);
    Py_DECREF(k);
    Py_DECREF(v);
    if (r<0) goto fail;
  }
  return dict;
  fail:
  Py_XDECREF(dict);
  return 0;
}

template<class T,class V,class O> static inline PyObject* to_python(const map<T,V,O>& m) {
  return to_python_map(m);
}

template<class T,class V,class H> static inline PyObject* to_python(const tr1::unordered_map<T,V,H>& m) {
  return to_python_map(m);
}

template<class T0,class T1> PyObject* to_python(const pair<T0,T1>& p) {
  using namespace geode;
  using geode::to_python;
  PyObject *x0=0,*x1=0,*tuple=0;
  x0 = to_python(p.first);
  if (!x0) goto fail;
  x1 = to_python(p.second);
  if (!x1) goto fail;
  tuple = PyTuple_New(2);
  if (!tuple) goto fail;
  PyTuple_SET_ITEM(tuple,0,x0);
  PyTuple_SET_ITEM(tuple,1,x1);
  return tuple;
  fail:
  Py_XDECREF(x0);
  Py_XDECREF(x1);
  Py_XDECREF(tuple);
  return 0;
}

}
namespace geode {

using std::vector;
using std::set;
using std::map;
using std::tr1::unordered_set;
using std::tr1::unordered_map;
using std::pair;

template<class T> struct FromPython<vector<T> >{static vector<T> convert(PyObject* object) {
  vector<T> result;
  Ref<PyObject> iterator = steal_ref_check(PyObject_GetIter(object));
  while (Ptr<PyObject> item = steal_ptr(PyIter_Next(&*iterator)))
    result.push_back(from_python<T>(item.get()));
  if (PyErr_Occurred()) // PyIter_Next returns 0 for both done and error, so check what happened
    throw_python_error();
  return result;
}};

template<class T> struct MutablePair{typedef T type;};
template<class T0,class T1> struct MutablePair<pair<const T0,T1>>{typedef pair<T0,T1> type;};

template<class TC,bool dict> struct FromPythonContainer{static TC convert(PyObject* object) {
  typedef typename MutablePair<typename TC::value_type>::type T;
  vector<T> v = from_python<vector<T> >(dict?&*steal_ref_check(PyDict_Items(object)):object);
  TC result(v.begin(),v.end());
  return result;
}};

template<class T,class O> struct FromPython<set<T,O>>:public FromPythonContainer<set<T,O>,false>{};
template<class T,class H> struct FromPython<unordered_set<T,H>>:public FromPythonContainer<unordered_set<T,H>,false>{};
template<class T,class V,class O> struct FromPython<map<T,V,O>>:public FromPythonContainer<map<T,V,O>,true>{};
template<class T,class V,class H> struct FromPython<unordered_map<T,V,H>>:public FromPythonContainer<unordered_map<T,V,H>,true>{};

template<class T0,class T1> struct FromPython<pair<T0,T1> >{static pair<T0,T1> convert(PyObject* object) {
  Ref<PyObject> seq = steal_ref_check(PySequence_Fast(object,"expected pair"));
  size_t len = PySequence_Length(&*seq);
  if (len!=2) {
    PyErr_Format(PyExc_TypeError,"expected pair, got length %ld",len);
    throw_python_error();
  }
  return pair<T0,T1>(from_python<T0>(ref_check(PySequence_Fast_GET_ITEM(&*seq,0))),
                     from_python<T1>(ref_check(PySequence_Fast_GET_ITEM(&*seq,1))));
}};

}
#endif
