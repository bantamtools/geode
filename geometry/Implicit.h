//#####################################################################
// Class Implicit
//#####################################################################
#pragma once

#include <othercore/geometry/Box.h>
#include <othercore/python/Object.h>
#include <othercore/vector/Vector.h>
namespace other{

template<class TV>
class Implicit:public Object
{
  typedef typename TV::Scalar T;
public:
  OTHER_DECLARE_TYPE(OTHER_CORE_EXPORT)
  static const int d = TV::m;

  Implicit();
  virtual ~Implicit();

  virtual T phi(const TV& X) const=0;
  virtual TV normal(const TV& X) const=0;
  virtual TV surface(const TV& X) const=0;
  virtual bool lazy_inside(const TV& X) const=0;
  virtual Box<TV> bounding_box() const=0;
  virtual string repr() const=0;
};
}
