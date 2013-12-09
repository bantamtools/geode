//#####################################################################
// Class BoxTree
//#####################################################################
#include <geode/geometry/BoxTree.h>
#include <geode/array/IndirectArray.h>
#include <geode/array/ProjectedArray.h>
#include <geode/array/sort.h>
#include <geode/geometry/Box.h>
#include <geode/geometry/Sphere.h>
#include <geode/geometry/traverse.h>
#include <geode/math/integer_log.h>
#include <geode/python/Class.h>
namespace geode {
using std::cout;
using std::endl;

template<> GEODE_DEFINE_TYPE(BoxTree<Vector<real,2>>)
template<> GEODE_DEFINE_TYPE(BoxTree<Vector<real,3>>)

namespace {

template<class T,int d> inline T center(const Vector<T,d>& x, int axis) {
  return x[axis];
}

template<class T,int d> inline T center(const Box<Vector<T,d>>& box, int axis) {
  return (T).5*(box.min[axis]+box.max[axis]);
}

struct CenterCompare {
  int axis;

  CenterCompare(int axis):axis(axis){}

  template<class TV> bool operator()(const TV& x1,const TV& x2) const
  {return x1[axis]<x2[axis];}

  template<class TV> bool operator()(const Box<TV>& box1,const Box<TV>& box2) const
  {return box1.min[axis]+box1.max[axis]<box2.min[axis]+box2.max[axis];}
};

void ranges_helper(RawArray<Range<int>> ranges, int& count, int leaf_size, int prims, int node) {
  ranges[node].lo = count;
  if (!ranges.valid(2*node+1))
    count = min(count+leaf_size,prims);
  else {
    ranges_helper(ranges,count,leaf_size,prims,2*node+1);
    ranges_helper(ranges,count,leaf_size,prims,2*node+2);
  }
  ranges[node].hi = count;
}

Array<const Range<int>> ranges(int prims, int leaf_size) {
  const int leaves = (prims+leaf_size-1)/leaf_size;
  Array<Range<int>> ranges(leaves?2*leaves-1:0,false);
  if (ranges.size()) {
    int count = 0;
    ranges_helper(ranges,count,leaf_size,prims,0);
  }
  return ranges;
}

template<class Geo,class TV> void
build(BoxTree<TV>& self, RawArray<const Range<int>> ranges, RawArray<const Geo> geo, int node) {
  // Compute box
  const auto r = ranges[node];
  Box<TV>& box = self.boxes[node];
  box = Box<TV>(geo[self.p[r.lo]]);
  for (int i=r.lo+1;i<r.hi;i++)
    box.enlarge_nonempty(geo[self.p[i]]);

  // Recursively split along largest axis if necessary
  if (self.is_leaf(node))
    sort(self.p.slice(r.lo,r.hi).const_cast_());
  else {
    const int axis = box.sizes().argmax();
    int* pp = const_cast<int*>(self.p.data());
    std::nth_element(pp+r.lo,
                     pp+ranges[2*node+1].hi,
                     pp+r.hi,indirect_comparison(geo,CenterCompare(axis)));
    build(self,ranges,geo,2*node+1);
    build(self,ranges,geo,2*node+2);
  }
}

}

static int check_leaf_size(int leaf_size) {
  GEODE_ASSERT(leaf_size>0);
  return leaf_size;
}

static int depth(int leaves) {
  if (!leaves)
    return 0;
  int depth = 1+integer_log(2*leaves-1);
  GEODE_ASSERT(2*leaves-1<=(1<<depth)-1 && (1<<(depth-1))-1<=2*leaves-1);
  return depth;
}

static inline Range<int> leaf_range(int prims, int leaf_size) {
  if (!prims)
    return range(0);
  int leaves = (prims+leaf_size-1)/leaf_size;
  return range(leaves-1,2*leaves-1);
}

template<class TV> BoxTree<TV>::BoxTree(RawArray<const TV> geo,int leaf_size)
  : leaf_size(check_leaf_size(leaf_size))
  , leaves(leaf_range(geo.size(),leaf_size))
  , depth(geode::depth(leaves.size()))
  , p(arange(geo.size()).copy())
  , ranges(geode::ranges(geo.size(),leaf_size))
  , boxes(max(0,leaves.hi),false)
{
  if (leaves.size())
    build(*this,ranges,geo,0);
}

template<class TV> BoxTree<TV>::BoxTree(RawArray<const Box<TV>> geo,int leaf_size)
  : leaf_size(check_leaf_size(leaf_size))
  , leaves(leaf_range(geo.size(),leaf_size))
  , depth(geode::depth(leaves.size()))
  , p(arange(geo.size()).copy())
  , ranges(geode::ranges(geo.size(),leaf_size))
  , boxes(max(0,leaves.hi),false)
{
  if (leaves.size())
    build(*this,ranges,geo,0);
}

template<class TV> BoxTree<TV>::BoxTree(const BoxTree<TV>& other)
  : leaf_size(other.leaf_size)
  , leaves(other.leaves)
  , depth(other.depth)
  , p(other.p)
  , ranges(other.ranges)
  , boxes(other.boxes.copy()) // Don't share ownership with geometry
{}

template<class TV> BoxTree<TV>::~BoxTree() {}

template<class TV> void BoxTree<TV>::update_nonleaf_boxes() {
  for(int n=leaves.lo-1;n>=0;n--)
    boxes[n] = Box<TV>::combine(boxes[2*n+1],boxes[2*n+2]);
}

namespace {
template<class TV> struct CheckVisitor {
  const BoxTree<TV>& tree;
  RawArray<const TV> X;
  int& culls;
  int& leaves;

  CheckVisitor(const BoxTree<TV>& tree, RawArray<const TV> X, int& culls, int& leaves)
    : tree(tree), X(X), culls(culls), leaves(leaves) {}

  bool cull(int n) const {
    if (!tree.is_leaf(n))
      GEODE_ASSERT(tree.boxes[n]==Box<TV>::combine(tree.boxes[2*n+1],tree.boxes[2*n+2]));
    culls++;
    return false;
  }

  void leaf(int n) const {
    GEODE_ASSERT(tree.ranges[n].hi==tree.p.size() || tree.prims(n).size()==tree.leaf_size);
    for (int i : tree.prims(n))
      GEODE_ASSERT(tree.boxes[n].lazy_inside(X[i]));
    leaves++;
  }
};
}

template<class TV> void BoxTree<TV>::
check(RawArray<const TV> X) const {
  GEODE_ASSERT(X.size()==p.size());
  Array<int> count(p.size());
  count.subset(p) += 1;
  GEODE_ASSERT(count.contains_only(1));
  int culls = 0, leaves = 0;
  single_traverse(*this,CheckVisitor<TV>(*this,X,culls,leaves));
  GEODE_ASSERT(culls==boxes.size() && leaves==this->leaves.size());
}

template<class TV,class Shape> static bool any_box_intersection_helper(const BoxTree<TV>& self, const Shape& shape, int node) {
  return shape.lazy_intersects(self.boxes[node])
      && (   self.is_leaf(node)
          || any_box_intersection_helper(self,shape,2*node+1)
          || any_box_intersection_helper(self,shape,2*node+2));
}

template<class TV> template<class Shape> bool BoxTree<TV>::
any_box_intersection(const Shape& shape) const {
  return any_box_intersection_helper(*this,shape,0);
}

#define INSTANTIATE(T,d) \
  template class BoxTree<Vector<T,d>>; \
  template GEODE_CORE_EXPORT bool BoxTree<Vector<T,d>>::any_box_intersection(const Box<Vector<T,d>>&) const; \
  template GEODE_CORE_EXPORT bool BoxTree<Vector<T,d>>::any_box_intersection(const Sphere<Vector<T,d>>&) const;
INSTANTIATE(real,2)
INSTANTIATE(real,3)
}
using namespace geode;

void wrap_box_tree() {
  {typedef Vector<real,2> TV;
  typedef BoxTree<TV> Self;
  Class<Self>("BoxTree2d")
    .GEODE_INIT(RawArray<const TV>,int)
    .GEODE_FIELD(p)
    .GEODE_METHOD(check)
    ;}

  {typedef Vector<real,3> TV;
  typedef BoxTree<TV> Self;
  Class<Self>("BoxTree3d")
    .GEODE_INIT(RawArray<const TV>,int)
    .GEODE_FIELD(p)
    .GEODE_METHOD(check)
    ;}
}
