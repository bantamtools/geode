// Thickened shells generalizing capsules
#pragma once

#include <othercore/geometry/Implicit.h>
#include <othercore/array/Array.h>
#include <othercore/mesh/forward.h>
namespace other {

class ThickShell : public Implicit<Vector<double,3>> {
public:
  OTHER_DECLARE_TYPE(OTHER_CORE_EXPORT)
  typedef double T;
  typedef Vector<T,3> TV;
  typedef Implicit<TV> Base;

  // Warning: The evaluation complexity is linear in the number of elements.
  // This class is intended for small numbers of triangles.
  const Array<const Vector<int,3>> tris;
  const Array<const Vector<int,2>> segs;
  const Array<const TV> X;
  const Array<const T> radii;

protected:
  ThickShell(const SegmentMesh& mesh, Array<const TV> X, Array<const T> radii);
  ThickShell(const TriangleMesh& mesh, Array<const TV> X, Array<const T> radii);
  ThickShell(Ref<> mesh, Array<const TV> X, Array<const T> radii);
public:
  ~ThickShell();

  TV normal(const TV& X) const;
  bool lazy_inside(const TV& X) const;
  TV surface(const TV& X) const;
  T phi(const TV& X) const;
  Box<TV> bounding_box() const;
  string repr() const;

private:
  Tuple<T,TV> phi_normal(const TV& X) const;
};

}
