//#####################################################################
// Class TriangleMesh
//#####################################################################
//
// TriangleMesh stores immutable topology for a triangle mesh.  The advantage
// of immutability is that we don't have to worry about acceleration structures
// becoming invalid, and we can check validity once at construction time.
//
//#####################################################################
#pragma once

#include <othercore/array/Array.h>
#include <othercore/array/NestedArray.h>
#include <othercore/mesh/forward.h>
#include <othercore/python/Object.h>
#include <othercore/python/Ptr.h>
#include <othercore/python/Ref.h>
#include <othercore/vector/Vector.h>
namespace other {

class TriangleMesh : public Object {
  typedef real T;
  typedef Vector<T,2> TV2;
  typedef Vector<T,3> TV3;
public:
  OTHER_DECLARE_TYPE(OTHER_CORE_EXPORT)
  typedef Object Base;
  static const int d = 2;

  Array<const int> vertices; // flattened version of triangles
  Array<const Vector<int,3> > elements;
private:
  int node_count;
  mutable Ptr<SegmentMesh> segment_mesh_;
  mutable bool bending_tuples_valid;
  mutable Array<Vector<int,4> > bending_tuples_; // i,j,k,l means triangles (i,j,k),(k,j,l)
  mutable NestedArray<int> incident_elements_;
  mutable Array<Vector<int,3> > adjacent_elements_;
  mutable Ptr<SegmentMesh> boundary_mesh_;
  mutable Array<int> nodes_touched_;
  mutable NestedArray<const int> sorted_neighbors_;

protected:
OTHER_CORE_EXPORT TriangleMesh(Array<const Vector<int,3> > elements);
public:
  ~TriangleMesh();

  int nodes() const {
    return node_count;
  }

  // to match PolygonMesh
  Ref<const TriangleMesh> triangle_mesh() const {
    return ref(*this);
  }

  OTHER_CORE_EXPORT Ref<const SegmentMesh> segment_mesh() const;
  OTHER_CORE_EXPORT NestedArray<const int> incident_elements() const; // vertices to triangles
  OTHER_CORE_EXPORT Array<const Vector<int,3> > adjacent_elements() const; // triangles to triangles
  OTHER_CORE_EXPORT Ref<SegmentMesh> boundary_mesh() const;
  OTHER_CORE_EXPORT Array<const Vector<int,4> > bending_tuples() const;
  OTHER_CORE_EXPORT Array<const int> nodes_touched() const;
  OTHER_CORE_EXPORT NestedArray<const int> sorted_neighbors() const; // vertices to sorted one-ring
  OTHER_CORE_EXPORT T area(RawArray<const TV2> X) const;
  OTHER_CORE_EXPORT T volume(RawArray<const TV3> X) const; // assumes a closed surface
  OTHER_CORE_EXPORT T surface_area(RawArray<const TV3> X) const;
  OTHER_CORE_EXPORT Array<T> vertex_areas(RawArray<const TV3> X) const;
  OTHER_CORE_EXPORT Array<TV3> vertex_normals(RawArray<const TV3> X) const;
  OTHER_CORE_EXPORT Array<TV3> element_normals(RawArray<const TV3> X) const;
  OTHER_CORE_EXPORT Array<int> nonmanifold_nodes(bool allow_boundary) const;
};

}
