//#####################################################################
// Class TriangleSoup
//#####################################################################
#include <geode/mesh/TriangleSoup.h>
#include <geode/mesh/SegmentSoup.h>
#include <geode/array/sort.h>
#include <geode/array/view.h>
#include <geode/structure/Hashtable.h>
#include <geode/python/Class.h>
namespace geode {

using std::cout;
using std::endl;
typedef real T;
typedef Vector<T,3> TV3;

GEODE_DEFINE_TYPE(TriangleSoup)

#ifndef _WIN32
const int TriangleSoup::d;
#endif

TriangleSoup::TriangleSoup(Array<const Vector<int,3>> elements)
  : vertices(scalar_view_own(elements))
  , elements(elements)
  , bending_tuples_valid(false) {
  // Assert validity and compute counts
  node_count = 0;
  for (int i=0;i<vertices.size();i++) {
    if (vertices[i] < 0)
      throw ValueError(format("TriangleSoup: invalid negative vertex %d",vertices[i]));
    node_count = max(node_count,vertices[i]+1);
  }
}

TriangleSoup::~TriangleSoup() {}

Ref<const SegmentSoup> TriangleSoup::segment_soup() const {
  if (!segment_soup_) {
    Hashtable<Vector<int,2>> hash;
    Array<Vector<int,2>> edges;
    for (const auto t : elements) {
      const auto e0 = vec(t.x,t.y).sorted(),
                 e1 = vec(t.y,t.z).sorted(),
                 e2 = vec(t.z,t.x).sorted();
      if (hash.set(e0)) edges.append(e0);
      if (hash.set(e1)) edges.append(e1);
      if (hash.set(e2)) edges.append(e2);
    }
    segment_soup_ = new_<SegmentSoup>(edges);
  }
  return ref(segment_soup_);
}

Array<const Vector<int,3>> TriangleSoup::triangle_edges() const {
  if (!triangle_edges_.size() && nodes()) {
    // Must match the algorithm of segment_soup() exactly
    Hashtable<Vector<int,2>,int> hash;
    triangle_edges_.resize(elements.size(),uninit);
    for (int i=0;i<elements.size();i++) {
      const auto t = elements[i];
      const int e0 = hash.get_or_insert(vec(t.x,t.y).sorted(),hash.size()),
                e1 = hash.get_or_insert(vec(t.y,t.z).sorted(),hash.size()),
                e2 = hash.get_or_insert(vec(t.z,t.x).sorted(),hash.size());
      triangle_edges_[i] = vec(e0,e1,e2);
    }
  }
  return triangle_edges_;
}

Nested<const int> TriangleSoup::incident_elements() const {
  if (!incident_elements_.size() && nodes()) {
    Array<int> lengths(nodes());
    for (int i=0;i<vertices.size();i++)
      lengths[vertices[i]]++;
    incident_elements_ = Nested<int>(lengths);
    for (int t=0;t<elements.size();t++) for(int i=0;i<3;i++) {
      int p = elements[t][i];
      incident_elements_(p,incident_elements_.size(p)-lengths[p]--)=t;
    }
  }
  return incident_elements_;
}

Array<const Vector<int,3>> TriangleSoup::adjacent_elements() const {
  if (!adjacent_elements_.size() && nodes()) {
    adjacent_elements_.resize(elements.size(),uninit);
    Nested<const int> incident = incident_elements();
    for (int t=0;t<elements.size();t++) {
      Vector<int,3> tri = elements[t];
      for (int j=0,i=2;j<3;i=j++) {
        for (int t2 : incident[tri[i]])
          if (t!=t2) {
            int a = elements[t2].find(tri[i]);
            if (elements[t2][(a+2)%3]==tri[j])  {
              adjacent_elements_[t][i] = t2;
              goto found;
            }
          }
        adjacent_elements_[t][i] = -1;
        found:;
      }
    }
  }
  return adjacent_elements_;
}

Ref<SegmentSoup> TriangleSoup::boundary_mesh() const {
  if (!boundary_mesh_) {
    Hashtable<Vector<int,2>,int> hash;
    for (int t=0;t<elements.size();t++)
      for (int i=0;i<3;i++) {
        int a = elements[t][i],
            b = elements[t][(i+1)%3];
        hash.get_or_insert(vec(a,b))++;
        hash.get_or_insert(vec(b,a))+=2;
      }
    Array<Vector<int,2>> segments;
    Ref<const SegmentSoup> segment_soup_ = segment_soup();
    for (int s=0;s<segment_soup_->elements.size();s++) {
      int i,j;segment_soup_->elements[s].get(i,j);
      if (hash.get_default(vec(i,j))==1)
        segments.append(vec(i,j));
      else if (hash.get_default(vec(j,i))==1)
        segments.append(vec(j,i));
    }
    boundary_mesh_ = new_<SegmentSoup>(segments);
  }
  return ref(boundary_mesh_);
}

Array<const Vector<int,4>> TriangleSoup::bending_tuples() const {
  if (!bending_tuples_valid) {
    Hashtable<Vector<int,2>,Array<int>> edge_to_face;
    for (int t=0;t<elements.size();t++) {
      Vector<int,3> nodes = elements[t];
      for (int a=0;a<3;a++)
        edge_to_face.get_or_insert(vec(nodes[a],nodes[(a+1)%3]).sorted()).append(t);
    }
    Array<int> other;
    Array<bool> flipped;
    for (const auto& it : edge_to_face) {
      Vector<int,2> sn = it.x;
      RawArray<const int> tris(it.y);
      other.clear();
      other.resize(tris.size(),uninit);
      flipped.clear();
      flipped.resize(tris.size(),uninit);
      for (int a=0;a<tris.size();a++) {
        Vector<int,3> tn = elements[tris[a]];
        int b = !sn.contains(tn[0])?0:!sn.contains(tn[1])?1:2;
        other[a] = tn[b];
        flipped[a] = tn[(b+1)%3]!=sn[0];
        assert(tn[(b+1)%3]==sn[flipped[a]] && tn[(b+2)%3]==sn[1-flipped[a]]);}
      for (int a=0;a<tris.size();a++) for (int b=a+1;b<tris.size();b++)
        bending_tuples_.append(vec(other[a],sn[flipped[a]],sn[1-flipped[a]],other[b]));
    }
    bending_tuples_valid = true;
  }
  return bending_tuples_;
}

Array<const int> TriangleSoup::nodes_touched() const {
  if (!nodes_touched_.size() && elements.size()) {
    Hashtable<int> hash;
    for (int t=0;t<elements.size();t++) for (int i=0;i<3;i++)
      if (hash.set(elements[t][i]))
        nodes_touched_.append(elements[t][i]);
    sort(nodes_touched_);
  }
  return nodes_touched_;
}

Nested<const int> TriangleSoup::sorted_neighbors() const {
  if (!sorted_neighbors_.size() && elements.size()) {
    Hashtable<Vector<int,2>,int> next; // next[(i,j)] = k if (i,j,k) is a triangle
    Hashtable<Vector<int,2>,int> prev; // prev[(i,k)] = j if (i,j,k) is a triangle
    for (const Vector<int,3>& tri : elements) {
      next.set(vec(tri[0],tri[1]),tri[2]);
      next.set(vec(tri[1],tri[2]),tri[0]);
      next.set(vec(tri[2],tri[0]),tri[1]);
      prev.set(vec(tri[0],tri[2]),tri[1]);
      prev.set(vec(tri[1],tri[0]),tri[2]);
      prev.set(vec(tri[2],tri[1]),tri[0]);
    }
    Nested<const int> neighbors = segment_soup()->neighbors();
    Nested<int> sorted_neighbors = Nested<int>::empty_like(neighbors);
    Hashtable<Vector<int,2>> done;
    for (int i=0;i<node_count;i++) {
      if (!neighbors.size(i))
        continue;
      // Find a node with no predecessor if one exists
      int j = neighbors(i,0);
      for (int a=1;a<neighbors.size(i);a++) {
        if (int* p = prev.get_pointer(vec(i,j)))
          j = *p;
        else
          break;
      }
      // Walk around boundary.  Note that we assume the mesh is manifold (possibly with boundary)
      sorted_neighbors(i,0) = j;
      try {
        for (int a=1;a<neighbors.size(i);a++) {
          j = next.get(vec(i,j));
          sorted_neighbors(i,a) = j;
        }
      } catch (const KeyError&) {
        throw RuntimeError(format("TriangleSoup::sorted_neighbors failed: node %d",i));
      }
    }
    sorted_neighbors_ = sorted_neighbors;
  }
  return sorted_neighbors_;
}

T TriangleSoup::area(RawArray<const TV2> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  T sum = 0;
  for (int t=0;t<elements.size();t++) {
    int i,j,k;elements[t].get(i,j,k);
    sum += cross(X[j]-X[i],X[k]-X[i]);
  }
  return (T).5*sum;
}

T TriangleSoup::volume(RawArray<const TV3> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  // If S is the surface and I is the interior, Stokes theorem gives
  //     V = int_I dV = 1/3 int_I (div x) dV = 1/3 int_S x . dA
  //       = 1/3 sum_t c_t . A_t
  //       = 1/18 sum_t (a + b + c) . (b - a) x (c - a)
  //       = 1/18 sum_t det (a+b+c, b-a, c-a)
  //       = 1/18 sum_t det (3a, b-a, c-a)
  //       = 1/6 sum_t det (a,b,c)
  // where a,b,c are the vertices of each triangle.
  T sum = 0;
  for (int t=0;t<elements.size();t++) {
    int i,j,k;elements[t].get(i,j,k);
    sum += det(X[i],X[j],X[k]);
  }
  return T(1./6)*sum;
}

T TriangleSoup::surface_area(RawArray<const TV3> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  T sum = 0;
  for (int t=0;t<elements.size();t++) {
    int i,j,k;elements[t].get(i,j,k);
    sum += magnitude(cross(X[j]-X[i],X[k]-X[i]));
  }
  return T(.5)*sum;
}

Array<T> TriangleSoup::vertex_areas(RawArray<const TV3> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  Array<T> areas(X.size());
  for (int t=0;t<elements.size();t++) {
    int i,j,k;elements[t].get(i,j,k);
    T area = T(1./6)*magnitude(cross(X[j]-X[i],X[k]-X[i]));
    areas[i] += area;
    areas[j] += area;
    areas[k] += area;
  }
  return areas;
}

Array<TV3> TriangleSoup::vertex_normals(RawArray<const TV3> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  Array<TV3> normals(X.size());
  for(int t=0;t<elements.size();t++){
    int i,j,k;elements[t].get(i,j,k);
    TV3 n = cross(X[j]-X[i],X[k]-X[i]);
    normals[i]+=n;normals[j]+=n;normals[k]+=n;
  }
  for(int i=0;i<X.size();i++)
    normals[i].normalize();
  return normals;
}

Array<TV3> TriangleSoup::element_normals(RawArray<const TV3> X) const {
  GEODE_ASSERT(X.size()>=nodes());
  Array<TV3> normals(elements.size(),uninit);
  for (int t=0;t<elements.size();t++) {
    int i,j,k;elements[t].get(i,j,k);
    normals[t] = cross(X[j]-X[i],X[k]-X[i]).normalized();
  }
  return normals;
}

Array<int> TriangleSoup::nonmanifold_nodes(bool allow_boundary) const {
  Array<int> nonmanifold;
  Nested<const int> incident_elements = this->incident_elements();
  Array<Vector<int,2>> ring;
  Hashtable<int,Vector<int,2>> neighbors(32); // prev,next for each node in the ring
  const Vector<int,2> none(-1,-1);
  for (int i=0;i<incident_elements.size();i++) {
    RawArray<const int> incident = incident_elements[i];
    if (!incident.size())
      continue;
    // Collect oriented boundary segments of the one ring
    ring.clear();
    ring.resize(incident.size(),uninit);
    for (int t=0;t<incident.size();t++) {
      Vector<int,3> tri = elements[incident[t]];
      if (tri.x==tri.y || tri.x==tri.z || tri.y==tri.z)
        goto bad;
      ring[t] = tri.x==i?vec(tri.y,tri.z):tri.y==i?vec(tri.z,tri.x):vec(tri.x,tri.y);
    }
    // Determine topology
    neighbors.clear();
    for (int t=0;t<ring.size();t++) {
      Vector<int,2>& nx = neighbors.get_or_insert(ring[t].x,none);
      if (nx.y>=0) goto bad; // node already has a next
      nx.y = ring[t].y;
      Vector<int,2>& ny = neighbors.get_or_insert(ring[t].y,none);
      if (ny.x>=0) goto bad; // node already has a prev
      ny.x = ring[t].x;
    }
    // At this point the ring is definitely a 1-manifold with boundary, possibly consisting of many component curves.
    if (neighbors.size()==ring.size()) { // All components are closed loops
      // Ensure there's only component by checking the size of one of them
      int start = ring[0].x, node = ring[0].y, steps = ring.size()-2;
      for (int j=0;j<steps;j++) {
        node = neighbors.get(node).y;
        if (node==start)
          goto bad;
      }
    } else if (neighbors.size()>ring.size()+1) // There are least two open curves
      goto bad;
    else { // There's exactly one open curve
      if (!allow_boundary)
        goto bad;
      // Ensure there's only one component by checking the size of one of them
      int middle = ring[0].x, count = 2;
      for (int start=middle;;) {
        start = neighbors.get(start).x;
        if (start<0) break;
        if (start==middle) goto bad; // Loop is closed, so there must be two components
        count++;
      }
      for (int end=ring[0].y;;) {
        end = neighbors.get(end).y;
        if (end<0) break;
        // No need to check for middle since we know the curve is open
        count++;
      }
      if (count!=neighbors.size())
        goto bad;
    }
    continue; // manifold node
    bad: nonmanifold.append(i);
  }
  return nonmanifold;
}

}
using namespace geode;

void wrap_triangle_mesh() {
  typedef TriangleSoup Self;
  Class<Self>("TriangleSoup")
    .GEODE_INIT(Array<const Vector<int,3>>)
    .GEODE_FIELD(d)
    .GEODE_FIELD(elements)
    .GEODE_FIELD(vertices)
    .GEODE_METHOD(segment_soup)
    .GEODE_METHOD(triangle_mesh)
    .GEODE_METHOD(triangle_edges)
    .GEODE_METHOD(incident_elements)
    .GEODE_METHOD(adjacent_elements)
    .GEODE_METHOD(boundary_mesh)
    .GEODE_METHOD(bending_tuples)
    .GEODE_METHOD(nodes_touched)
    .GEODE_METHOD(area)
    .GEODE_METHOD(volume)
    .GEODE_METHOD(surface_area)
    .GEODE_METHOD(vertex_areas)
    .GEODE_METHOD(vertex_normals)
    .GEODE_METHOD(element_normals)
    .GEODE_METHOD(nodes)
    .GEODE_METHOD(nonmanifold_nodes)
    .GEODE_METHOD(sorted_neighbors)
    ;
}
