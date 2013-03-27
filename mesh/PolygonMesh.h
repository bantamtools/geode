//#####################################################################
// Class PolygonMesh
//#####################################################################
//
// PolygonMesh stores immutable topology for a polygon mesh.  The advantage
// of immutability is that we don't have to worry about acceleration structures
// becoming invalid, and we can check validity once at construction time.
//
//#####################################################################
#pragma once

#include <othercore/array/Array.h>
#include <othercore/mesh/forward.h>
#include <othercore/python/Object.h>
#include <othercore/python/Ptr.h>
#include <othercore/python/Ref.h>
namespace other {

class PolygonMesh : public Object {
public:
  OTHER_DECLARE_TYPE(OTHER_CORE_EXPORT)
  typedef Object Base;

  const Array<const int> counts; // number of vertices in each polygon
  const Array<const int> vertices; // indices of each polygon flattened into a single array
private:
  const int node_count, half_edge_count;
  mutable Ptr<SegmentMesh> segment_mesh_;
  mutable Ptr<TriangleMesh> triangle_mesh_;

protected:
  PolygonMesh(Array<const int> counts, Array<const int> vertices);
public:
  ~PolygonMesh();

  int nodes() const {
    return node_count;
  }

  OTHER_CORE_EXPORT Ref<SegmentMesh> segment_mesh() const;
  OTHER_CORE_EXPORT Ref<TriangleMesh> triangle_mesh() const;
};
}
