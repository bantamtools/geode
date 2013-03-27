//#####################################################################
// Function wrap_field
//#####################################################################
#ifdef OTHER_PYTHON
#include <othercore/python/wrap_field.h>
namespace other {

PyObject* wrap_field_helper(PyTypeObject* type,const char* name,size_t offset,getter get,setter set) {
  // Allocate PyGetSetDef
  PyGetSetDef* def = (PyGetSetDef*)malloc(sizeof(PyGetSetDef));
  if (!def) throw std::bad_alloc();
  memset(def,0,sizeof(PyGetSetDef));

  // Fill in fields
  def->name = (char*)name;
  def->get = get;
  def->set = set;
  def->closure = (void*)offset;

  // allocate property
  PyObject* descr = PyDescr_NewGetSet(type,def);
  if (!descr) throw PythonError();
  return descr;
}

}
#endif
