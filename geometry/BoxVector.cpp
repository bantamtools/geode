//#####################################################################
// Class Box
//#####################################################################
#include <othercore/geometry/Box.h>
#include <othercore/geometry/AnalyticImplicit.h>
#include <othercore/geometry/Ray.h>
#include <othercore/python/from_python.h>
#include <othercore/python/exceptions.h>
#include <othercore/math/cube.h>
#include <othercore/utility/format.h>
#include <othercore/structure/Tuple.h>
namespace other {

typedef real T;

#ifdef OTHER_PYTHON

template<class T,int d> PyObject* to_python(const Box<Vector<T,d>>& box) {
  return to_python(new_<AnalyticImplicit<Box<Vector<T,d>>>>(box));
}

template<> PyObject* to_python(const Box<Vector<int,2>>& box) {
  return to_python(tuple(box.min,box.max));
}

template<> PyObject* to_python(const Box<Vector<int,3>>& box) {
  return to_python(tuple(box.min,box.max));
}

template<class T,int d> Box<Vector<T,d>> FromPython<Box<Vector<T,d>>>::convert(PyObject* object) {
  return from_python<AnalyticImplicit<Box<Vector<T,d>>>&>(object);
}

template<> Box<Vector<int,2>> FromPython<Box<Vector<int,2>>>::convert(PyObject* object) {
  Tuple<Vector<int,2>,Vector<int,2>> t = from_python<Tuple<Vector<int,2>,Vector<int,2>>>(object);
  return Box<Vector<int,2>>(t.x,t.y);
}

template<> Box<Vector<int,3>> FromPython<Box<Vector<int,3>>>::convert(PyObject* object) {
  Tuple<Vector<int,3>,Vector<int,3>> t = from_python<Tuple<Vector<int,3>,Vector<int,3>>>(object);
  return Box<Vector<int,3>>(t.x,t.y);
}

#endif

template<class T,int d> Vector<T,d> Box<Vector<T,d>>::surface(const TV& X) const {
  if (!lazy_inside(X)) return other::clamp(X,min,max);
  TV min_side = X-min,
     max_side = max-X,
     result = X;
  int min_argmin = min_side.argmin(),
      max_argmin = max_side.argmin();
  if (min_side[min_argmin]<=max_side[max_argmin])
    result[min_argmin] = min[min_argmin];
  else
    result[max_argmin] = max[max_argmin];
  return result;
}

template<class T,int d> T Box<Vector<T,d>>::phi(const TV& X) const {
  TV lengths = sizes();
  TV phi = abs(X-center())-(T).5*lengths;
  if (!all_less_equal(phi,TV()))
    return TV::componentwise_max(phi,TV()).magnitude();
  return phi.max();
}

template<class T,int d> Vector<T,d> Box<Vector<T,d>>::normal(const TV& X) const {
  if (lazy_inside(X)) {
    TV phis_max = X-max,
       phis_min = min-X;
    int axis = TV::componentwise_max(phis_min,phis_max).argmax();
    return T(phis_max[axis]>phis_min[axis]?1:-1)*TV::axis_vector(axis);
  } else {
    TV phis_max = X-max,
       phis_min = min-X;
    TV normal;
    for (int i=0;i<d;i++) {
      T phi = other::max(phis_min[i],phis_max[i]);
      normal[i] = phi>0?(phis_max[i]>phis_min[i]?phi:-phi):0;
    }
    return normal.normalized();
  }
}

template<class T,int d> string Box<Vector<T,d>>::name() {
  return format("Box<Vector<T,%d>",d);
}

template<class T,int d> string Box<Vector<T,d>>::repr() const {
  return format("Box(%s,%s)",tuple_repr(min),tuple_repr(max));
}

// This is a fast routine to do ray box intersections
// box_enlargement modifies the bounds of the box -- it's not a thickness
template<class T,int d> bool Box<Vector<T,d>>::lazy_intersects(const Ray<TV>& ray,T box_enlargement) const {
  OTHER_NOT_IMPLEMENTED();
}

// This is a fast routine to do ray box intersections
// box_enlargement modifies the bounds of the box -- it's not a thickness
template<> bool Box<Vector<T,2>>::lazy_intersects(const Ray<Vector<T,2>>& ray,T box_enlargement) const {
  BOOST_STATIC_ASSERT(d==2);
  // This comes from a paper "An efficient and Robust Ray-Box Intersection algorithm" by williams, barrus, morley, and Shirley
  // http://www.cs.utah.edu/~rmorley/pubs/box.pdf
  if(!ray.computed_lazy_box_intersection_acceleration_data)
      ray.compute_lazy_box_intersection_acceleration_data();
  Vector<T,2> extremes[2] = {min-box_enlargement,max+box_enlargement};
  T tmin =                (extremes[  ray.direction_is_negative.x].x-ray.start.x)*ray.inverse_direction.x;
  T tmax =                (extremes[1-ray.direction_is_negative.x].x-ray.start.x)*ray.inverse_direction.x;
  tmin = other::max(tmin, (extremes[  ray.direction_is_negative.y].y-ray.start.y)*ray.inverse_direction.y);
  tmax = other::min(tmax, (extremes[1-ray.direction_is_negative.y].y-ray.start.y)*ray.inverse_direction.y);
  return tmin<=tmax && 0<=tmax && tmin<=ray.t_max;
}

// This is a fast routine to do ray box intersections
// box_enlargement modifies the bounds of the box -- it's not a thickness
template<> bool Box<Vector<T,3>>::lazy_intersects(const Ray<Vector<T,3>>& ray,T box_enlargement) const {
  BOOST_STATIC_ASSERT(d==3);
  // This comes from a paper "An efficient and Robust Ray-Box Intersection algorithm" by williams, barrus, morley, and Shirley
  // http://www.cs.utah.edu/~rmorley/pubs/box.pdf
  if(!ray.computed_lazy_box_intersection_acceleration_data)
      ray.compute_lazy_box_intersection_acceleration_data();
  Vector<T,3> extremes[2] = {min-box_enlargement,max+box_enlargement};
  T tmin =                (extremes[  ray.direction_is_negative.x].x-ray.start.x)*ray.inverse_direction.x;
  T tmax =                (extremes[1-ray.direction_is_negative.x].x-ray.start.x)*ray.inverse_direction.x;
  tmin = other::max(tmin, (extremes[  ray.direction_is_negative.y].y-ray.start.y)*ray.inverse_direction.y);
  tmax = other::min(tmax, (extremes[1-ray.direction_is_negative.y].y-ray.start.y)*ray.inverse_direction.y);
  tmin = other::max(tmin, (extremes[  ray.direction_is_negative.z].z-ray.start.z)*ray.inverse_direction.z);
  tmax = other::min(tmax, (extremes[1-ray.direction_is_negative.z].z-ray.start.z)*ray.inverse_direction.z);
  return tmin<=tmax && 0<=tmax && tmin<=ray.t_max;
}

typedef Vector<T,3> TV;

#define INSTANTIATION_HELPER(T,d) \
  template OTHER_CORE_EXPORT string Box<Vector<T,d>>::name(); \
  template OTHER_CORE_EXPORT string Box<Vector<T,d>>::repr() const; \
  template OTHER_CORE_EXPORT Vector<T,d> Box<Vector<T,d>>::normal(const Vector<T,d>&) const; \
  template OTHER_CORE_EXPORT Vector<T,d> Box<Vector<T,d>>::surface(const Vector<T,d>&) const; \
  template OTHER_CORE_EXPORT Vector<T,d>::Scalar Box<Vector<T,d>>::phi(const Vector<T,d>&) const; \
  OTHER_ONLY_PYTHON(template OTHER_CORE_EXPORT PyObject* to_python<T,d>(const Box<Vector<T,d>>&)); \
  OTHER_ONLY_PYTHON(template OTHER_CORE_EXPORT Box<Vector<T,d>> FromPython<Box<Vector<T,d>>>::convert(PyObject*));
INSTANTIATION_HELPER(T,1)
INSTANTIATION_HELPER(T,2)
INSTANTIATION_HELPER(T,3)
#ifndef _WIN32
template bool Box<Vector<T,2>>::lazy_intersects(const Ray<Vector<T,2>>&,T) const;
template bool Box<Vector<T,3>>::lazy_intersects(const Ray<Vector<T,3>>&,T) const;
#endif

}
