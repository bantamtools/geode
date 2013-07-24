// Randomized incremental Delaunay using simulation of simplicity
#pragma once

#include <othercore/exact/config.h>
#include <othercore/mesh/CornerMesh.h>
#include <othercore/mesh/HalfedgeMesh.h>
namespace other {

// Compute an approximate Delaunay triangulation of a point set, by first quantizing and performing exact Delaunay.
template<class Mesh> OTHER_CORE_EXPORT Ref<Mesh> delaunay_points(RawArray<const Vector<real,2>> X, bool validate=false);

// Compute the exact Delaunay triangulation of a quantized point set
template<class Mesh> OTHER_CORE_EXPORT Ref<Mesh> exact_delaunay_points(RawArray<const Vector<Quantized,2>> X, bool validate=false);

}
