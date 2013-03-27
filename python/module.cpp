//#####################################################################
// Module Python
//#####################################################################
#include <othercore/utility/config.h>
#define OTHER_IMPORT_NUMPY
#include <othercore/python/module.h>
#include <othercore/python/enum.h>
#include <othercore/python/numpy.h>
#include <othercore/python/stl.h>
#include <othercore/python/wrap.h>
namespace other {

#ifdef OTHER_PYTHON

static std::vector<PyObject*> modules;

static PyObject* module() {
  if (modules.empty())
    throw RuntimeError("No current module");
  return modules.back();
}

static void import_core() {
#ifdef _WIN32
  // On windows, all code is compiled into a single python module, so there's nothing else to import
  return;
#else
  char* name = PyModule_GetName(module());
  if (!name) throw_python_error();
  if (strcmp(name,"other_core")) {
    PyObject* python_str = PyString_FromString("other.core");
    if (!python_str) throw_python_error();
    PyObject* python = PyImport_Import(python_str);
    Py_DECREF(python_str);
    if (!python) throw_python_error();
  }
#endif
}

void module_push(const char* name) {
  auto module = Py_InitModule3(name,0,0);
  if (!module)
    throw_python_error();
  modules.push_back(module);
  import_core();
}

void module_pop() {
  modules.pop_back();
}

template<class TC> static TC convert_test(const TC& c) {
  return c;
}

namespace python {

void add_object(const char* name, PyObject* object) {
  if (!object) throw PythonError();
  PyModule_AddObject(module(),name,object);
}

}

#else // non-python stubs

namespace python {
void add_object(const char* name, PyObject* object) {}
}

#endif

enum EnumTest { EnumTestA, EnumTestB };
OTHER_DEFINE_ENUM(EnumTest,OTHER_CORE_EXPORT)

}
using namespace other;
using namespace other::python;

void wrap_python() {
#ifdef OTHER_PYTHON
  if(strncmp(PY_VERSION,Py_GetVersion(),3)) {
    PyErr_Format(PyExc_ImportError,"python version mismatch: compiled again %s, linked against %s",PY_VERSION,Py_GetVersion());
    throw_python_error();
  }

  OTHER_WRAP(object)
  OTHER_WRAP(python_function)
  OTHER_WRAP(exceptions)
  OTHER_WRAP(test_class)

  function("list_convert_test",convert_test<vector<int> >);
  function("set_convert_test",convert_test<unordered_set<int> >);
  function("dict_convert_test",convert_test<unordered_map<int,string> >);
  function("enum_convert_test",convert_test<EnumTest>);

  OTHER_ENUM(EnumTest)
  OTHER_ENUM_VALUE(EnumTestA)
  OTHER_ENUM_VALUE(EnumTestB)

  // import numpy
  if (_import_array()<0){
    PyErr_Print();
    PyErr_SetString(PyExc_ImportError,"numpy.core.multiarray failed to import");
    throw_python_error();
  }

  python::add_object("real",(PyObject*)PyArray_DescrFromType(NumpyScalar<real>::value));
#endif
}
