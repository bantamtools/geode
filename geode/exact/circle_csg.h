// Robust constructive solid geometry for circular arc polygons in the plane
#pragma once

// These routines use algorithms almost identical to the actual polygon case, except using implicit
// representations for arcs and different (much higher order) predicates to check topology.

#include <geode/exact/config.h>
#include <geode/exact/quantize.h>
#include <geode/array/Nested.h>
#include <geode/vector/Frame.h>
namespace geode {

// In floating point, we represent circular arcs by the two endpoints x0,x1 and q = 2*sagitta/|x1-x0|.
// q > 0 is counterclockwise, q < 0 is clockwise, q = 0 is a straight line.  In a circular arc polygon
// Array<CircleArc> a, arc k has endpoints a[k].x, a[k+1].x and q = a[k].q.
// arc k
struct CircleArc {
  Vector<real,2> x;
  real q;

  CircleArc()
    : q() {}

  CircleArc(const Vector<real,2> x, const real q)
    : x(x), q(q) {}

  CircleArc(const real x, const real y, const real q)
    : x(x,y), q(q) {}

  bool operator==(const CircleArc& rhs) const { return x == rhs.x && q == rhs.q; }
  bool operator!=(const CircleArc& rhs) const { return !((*this) == rhs);}
};

std::ostream& operator<<(std::ostream& output, const CircleArc& a);

// After quantization, we represent circles implicitly by center and radius, plus two boolean flags
// describing how to connect adjacent arcs.
struct ExactCircleArc {
  Vector<Quantized,2> center;
  Quantized radius;
  int index; // Index into the symbolic perturbation
  bool positive; // True if the arc is traversed counterclockwise
  bool left; // True if we use the intersection between this arc and the next to the left of the segment joining their centers
  ExactCircleArc() = default;
  ExactCircleArc(const Vector<Quantized,2> _center, const Quantized _radius, const int _index, const bool _positive, const bool _left)
   : center(_center)
   , radius(_radius)
   , index(_index)
   , positive(_positive)
   , left(_left)
  {}
};

// Tweak quantized circles so that they intersect.
// WARNING: This is not safe to call if multiple arcs have the same perturbation index (might tweak one copy without tweaking the others)
void tweak_arcs_to_intersect(RawArray<ExactCircleArc> arcs);
void tweak_arcs_to_intersect(Nested<ExactCircleArc>& arcs);

// Resolve all intersections between circular arc polygons, and extract the contour with given 
// Depth starts at 0 at infinity, and increases by 1 when crossing a contour from outside to inside.
// For example, depth = 0 corresponds to polygon_union.
GEODE_CORE_EXPORT Nested<CircleArc> split_circle_arcs(Nested<const CircleArc> arcs, const int depth);
GEODE_CORE_EXPORT Nested<ExactCircleArc> exact_split_circle_arcs(Nested<const ExactCircleArc> arcs, const int depth);

// The union of possibly intersecting circular arc polygons, assuming consistent ordering
template<class... Arcs> static inline Nested<CircleArc> circle_arc_union(const Arcs&... arcs) {
  return split_circle_arcs(concatenate(arcs...),0);
}

// The intersection of possibly intersecting circular arc polygons, assuming consistent ordering.
template<class... Arcs> static inline Nested<CircleArc> circle_arc_intersection(const Arcs&... arcs) {
  return split_circle_arcs(concatenate(arcs...),sizeof...(Arcs)-1);
}

// Signed area of circular arc polygons
GEODE_CORE_EXPORT real circle_arc_area(RawArray<const CircleArc> arcs);
GEODE_CORE_EXPORT real circle_arc_area(Nested<const CircleArc> arcs);

// Reverse winding of circular arc polygons
GEODE_CORE_EXPORT void reverse_arcs(RawArray<CircleArc> arcs);
GEODE_CORE_EXPORT void reverse_arcs(Nested<CircleArc> arcs);

// Quantize from approximate to exact representations, taking care to ensure validity of the quantized result.
// If min_bounds isn't empty the Quantizer will use an appropriate scale to work with other features inside of min_bounds
GEODE_CORE_EXPORT Tuple<Quantizer<real,2>,Nested<ExactCircleArc>> quantize_circle_arcs(const Nested<const CircleArc> arcs, const Box<Vector<real,2>> min_bounds=Box<Vector<real,2>>::empty_box());
GEODE_CORE_EXPORT Nested<CircleArc> unquantize_circle_arcs(const Quantizer<real,2> quant, Nested<const ExactCircleArc> input);

// exact_split_circle_arcs prunes away contours that are too small to intersect with a horizontal line.  Normally this can be
// ignored, but we expose it here for use in benchmarking highly degenerate cases.
GEODE_CORE_EXPORT Nested<const ExactCircleArc> preprune_small_circle_arcs(Nested<const ExactCircleArc> arcs);

GEODE_CORE_EXPORT Box<Vector<real,2>> approximate_bounding_box(const RawArray<const CircleArc> input);
GEODE_CORE_EXPORT Box<Vector<real,2>> approximate_bounding_box(const Nested<const CircleArc>& input);
GEODE_CORE_EXPORT ostream& operator<<(ostream& output, const CircleArc& arc);
GEODE_CORE_EXPORT ostream& operator<<(ostream& output, const ExactCircleArc& arc);

// Transformation operators for circle arcs
static inline CircleArc operator*(const real a,                      const CircleArc& c) { return CircleArc(a*c.x,c.q); }
static inline CircleArc operator*(const Rotation<Vector<real,2>>& a, const CircleArc& c) { return CircleArc(a*c.x,c.q); }
static inline CircleArc operator*(const Frame<Vector<real,2>>& a,    const CircleArc& c) { return CircleArc(a*c.x,c.q); }
static inline CircleArc operator+(const Vector<real,2>& t, const CircleArc& c) { return CircleArc(t+c.x,c.q); }
static inline CircleArc operator+(const CircleArc& c, const Vector<real,2>& t) { return CircleArc(t+c.x,c.q); }

// Hashing for circle arcs
template<> struct is_packed_pod<CircleArc> : public mpl::true_{};
static inline Hash hash_reduce(const ExactCircleArc& a) {
  return Hash(a.center,a.radius,a.index,a.positive,a.left);
}

}
