#pragma once

#include <othercore/structure/Tuple.h>
#include <othercore/array/Array.h>
#include <othercore/mesh/forward.h>
#include <othercore/python/Ptr.h>

namespace other {

template<class TV, int d> class SimplexTree;

OTHER_CORE_EXPORT Array<Vec2> polygon_from_index_list(RawArray<const Vec2> positions, RawArray<const int> indices);
OTHER_CORE_EXPORT Nested<Vec2> polygons_from_index_list(RawArray<const Vec2> positions, Nested<const int> indices);

// Compute signed area of polygon(s)
OTHER_CORE_EXPORT real polygon_area(RawArray<const Vec2> poly);
OTHER_CORE_EXPORT real polygon_area(Nested<const Vec2> polys);

// Compute the length of an open polygon
OTHER_CORE_EXPORT real open_polygon_length(RawArray<const Vec2> poly);

// Compute the circumference of a closed polygon
OTHER_CORE_EXPORT real polygon_length(RawArray<const Vec2> poly);

// Enforce maximum edge length along the polygon
OTHER_CORE_EXPORT Array<Vec2> resample_polygon(RawArray<const Vec2> poly, double maximum_edge_length);

// check whether the outlines of two polygons intersect (returns false if one is completely inside the other)
// if p2_tree is NULL, a search tree is created for p2
OTHER_CORE_EXPORT bool polygon_outlines_intersect(RawArray<const Vec2> p1, RawArray<const Vec2> p2, Ptr<SimplexTree<Vec2,1>> p2_tree = Ptr<>());

// Is the point inside the polygon?  WARNING: Not robust
OTHER_CORE_EXPORT bool inside_polygon(RawArray<const Vec2> poly, const Vec2 p);

// Find a point inside the shape defined by polys, and inside the contour poly.
// TODO: This is used only by CGAL Delaunay to compute seed points.  Our version won't use approximate seed points once, so this function should be discarded once our version exists.
OTHER_CORE_EXPORT Vec2 point_inside_polygon_component(RawArray<const Vec2> poly, Nested<const Vec2> polys);

// Warning: Not robust
OTHER_CORE_EXPORT Tuple<Array<Vec2>,Array<int>> offset_polygon_with_correspondence(RawArray<const Vec2> poly, real offset, real maxangle_deg = 20., real minangle_deg = 10.);

// Turn an array of polygons into a SegmentMesh.
OTHER_CORE_EXPORT Ref<SegmentMesh> nested_array_offsets_to_segment_mesh(RawArray<const int> offsets, bool open);
template<class TV> static inline Tuple<Ref<SegmentMesh>,Array<TV>> to_segment_mesh(const Nested<TV>& polys, bool open) {
  return tuple(nested_array_offsets_to_segment_mesh(polys.offsets,open),polys.flat);
}

// Make it easy to overload python functions to work with one or many polygons
OTHER_CORE_EXPORT Nested<const Vec2> polygons_from_python(PyObject* object);

// Reorder some polygons into canonical form, assuming nondegeneracy.  Primarily for debugging and unit test purposes.
OTHER_CORE_EXPORT Nested<Vec2> canonicalize_polygons(Nested<const Vec2> polys);

// nested.flat[i] connects to nested.flat[closed_contour_next[i]]
// This allows traversing closed contours as a graph instead of with special cases or messy modular arithmetic
template<class T> Array<int> closed_contours_next(const Nested<T>& nested) {
  const int n = nested.total_size();
  Array<int> result(n, false);
  if(n == 0)
    return result;
  for(int i : range(1, n)) {
    result[i-1] = i;
  }
  for(int j : range(nested.size())) {
    const int lo = nested.offsets[j],
              hi = nested.offsets[j+1];
    if(lo < hi) {
      result[hi-1] = lo;
    }
  }
  return result;
}

}
