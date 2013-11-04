// Randomized incremental Delaunay using simulation of simplicity
#pragma once

#include <geode/exact/config.h>
#include <geode/mesh/TriangleTopology.h>
namespace geode {

// Compute an approximate Delaunay triangulation of a point set, by first quantizing and performing exact Delaunay.
GEODE_CORE_EXPORT Ref<TriangleTopology> delaunay_points(RawArray<const Vector<real,2>> X, bool validate=false);

// Compute the exact Delaunay triangulation of a quantized point set
GEODE_CORE_EXPORT Ref<TriangleTopology> exact_delaunay_points(RawArray<const Vector<Quantized,2>> X, bool validate=false);

}
