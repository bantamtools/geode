#include <geode/mesh/improve_mesh.h>

namespace geode {

bool improve_mesh_inplace(MutableTriangleTopology &mesh,
                          Field<Vector<real,3>, VertexId> const &pos,
                          ImproveOptions const &o) {
  auto Q = [](Triangle<Vector<real,3>> const &t) { return t.quality(); };
  auto EL = [](VertexId, VertexId) { return false; };
  auto VL = [](VertexId) { return false; };

  return improve_mesh_inplace(mesh, pos, o, EL, VL, Q);
}

// positions are assumed to be at default location
Ref<MutableTriangleTopology> improve_mesh(MutableTriangleTopology const &mesh, real min_quality, real max_distance, real max_silhouette_distance, real min_normal_dot, int max_iter, real min_relevant_area, real min_quality_improvement) {
  FieldId<Vector<real,3>,VertexId> posid(vertex_position_id);
  Ref<MutableTriangleTopology> copy = mesh.copy();
  improve_mesh_inplace(copy, copy->field(posid), ImproveOptions(min_quality, max_distance, max_silhouette_distance, min_normal_dot, max_iter, min_relevant_area, min_quality_improvement));
  return copy;
}

}

#include <geode/python/wrap.h>

using namespace geode;

void wrap_improve_mesh() {
  GEODE_FUNCTION(improve_mesh);
}
