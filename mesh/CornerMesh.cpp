// A corner data structure representing oriented triangle meshes.

#include <othercore/mesh/CornerMesh.h>
#include <othercore/array/convert.h>
#include <othercore/array/NestedArray.h>
#include <othercore/python/Class.h>
#include <othercore/random/Random.h>
#include <othercore/structure/Hashtable.h>
#include <othercore/utility/Log.h>
#include <othercore/vector/convert.h>
#include <boost/dynamic_bitset.hpp>
namespace other {

using Log::cout;
using std::endl;

OTHER_DEFINE_TYPE(CornerMesh)

// Add numpy conversion support
#ifdef OTHER_PYTHON
namespace {
template<> struct NumpyIsScalar<VertexId>:public mpl::true_{};
template<> struct NumpyIsScalar<HalfedgeId>:public mpl::true_{};
template<> struct NumpyIsScalar<FaceId>:public mpl::true_{};
template<> struct NumpyScalar<VertexId>{enum{value=NPY_INT};};
template<> struct NumpyScalar<HalfedgeId>{enum{value=NPY_INT};};
template<> struct NumpyScalar<FaceId>{enum{value=NPY_INT};};
}
#endif

static string str_halfedge(HalfedgeId e) {
  return e.valid() ? e.id>=0 ? format("e%d%d",e.id/3,e.id%3)
                             : format("b%d",-1-e.id)
                   : "e_";
}

CornerMesh::CornerMesh()
  : n_vertices_(0)
  , n_faces_(0)
  , n_boundary_edges_(0) {}

CornerMesh::CornerMesh(const CornerMesh& mesh)
  : n_vertices_(mesh.n_vertices_)
  , n_faces_(mesh.n_faces_)
  , n_boundary_edges_(mesh.n_boundary_edges_)
  , faces_(mesh.faces_.copy())
  , vertex_to_edge_(mesh.vertex_to_edge_.copy())
  , boundaries_(mesh.boundaries_.copy())
  , deleted_boundaries_(mesh.deleted_boundaries_) {}

CornerMesh::~CornerMesh() {}

Ref<CornerMesh> CornerMesh::copy() const {
  return new_<CornerMesh>(*this);
}

HalfedgeId CornerMesh::halfedge(VertexId v0, VertexId v1) const {
  assert(valid(v0));
  const auto start = halfedge(v0);
  if (start.valid()) {
    auto e = start;
    do {
      if (dst(e)==v1)
        return e;
      e = left(e);
    } while (e != start);
  }
  return HalfedgeId();
}

HalfedgeId CornerMesh::common_halfedge(FaceId f0, FaceId f1) const {
  if (f0.valid()) {
    for (const auto e : halfedges(f0))
      if (face(reverse(e))==f1)
        return e;
  } else if (f1.valid()) {
    for (const auto e : halfedges(f1))
      if (face(reverse(e))==f0)
        return reverse(e);
  }
  return HalfedgeId();
}

VertexId CornerMesh::add_vertex() {
  n_vertices_++;
  return vertex_to_edge_.append(HalfedgeId());
}

void CornerMesh::add_vertices(int n) {
  n_vertices_ += n;
  vertex_to_edge_.flat.resize(vertex_to_edge_.size()+n);
}

static inline HalfedgeId right_around_dst_to_boundary(const CornerMesh& mesh, HalfedgeId e) {
  e = mesh.reverse(mesh.next(mesh.reverse(e)));
  while (!mesh.is_boundary(e))
    e = mesh.reverse(mesh.next(e));
  return e;
}

// Set halfedge(v) to an outgoing boundary halfedge if possible, or any halfedge otherwise.  We start
// looping at the given initial guess, which should ideally be a likely boundary edge.
static inline void fix_vertex_to_edge(CornerMesh& mesh, const VertexId v, const HalfedgeId start) {
  auto e = start;
  for (;;) {
    if (mesh.is_boundary(e))
      break;
    e = mesh.left(e);
    if (e==start)
      break;
  }
  mesh.vertex_to_edge_[v] = e;
}

// Allocate a fresh boundary edge and set its src and reverse pointers
static inline HalfedgeId unsafe_new_boundary(CornerMesh& mesh, const VertexId src, const HalfedgeId reverse) {
  mesh.n_boundary_edges_++;
  HalfedgeId e;
  if (mesh.deleted_boundaries_.valid()) {
    e = mesh.deleted_boundaries_;
    mesh.deleted_boundaries_ = mesh.boundaries_[-1-e.id].next;
  } else {
    const int b = mesh.boundaries_.size();
    mesh.boundaries_.resize(b+1,false);
    e = HalfedgeId(-1-b);
  }
  mesh.boundaries_[-1-e.id].src = src;
  mesh.boundaries_[-1-e.id].reverse = reverse;
  return e;
}

OTHER_COLD static void add_face_error(const Vector<VertexId,3> v, const char* reason) {
  throw RuntimeError(format("CornerMesh::add_face: can't add face (%d,%d,%d)%s",v.x.id,v.y.id,v.z.id,reason));
}

FaceId CornerMesh::add_face(const Vector<VertexId,3> v) {
  // Check for errors
  if (!valid(v.x) || !valid(v.y) || !valid(v.z))
    add_face_error(v," containing invalid vertex");
  if (v.x==v.y || v.y==v.z || v.z==v.x)
    add_face_error(v," containing duplicate vertex");
  if (!is_boundary(v.x) || !is_boundary(v.y) || !is_boundary(v.z))
    add_face_error(v,", one of the vertices is interior");
  const auto e0 = halfedge(v.x,v.y),
             e1 = halfedge(v.y,v.z),
             e2 = halfedge(v.z,v.x);
  if (   (e0.valid() && !is_boundary(e0))
      || (e1.valid() && !is_boundary(e1))
      || (e2.valid() && !is_boundary(e2)))
    add_face_error(v,", one of the edges is interior");

  // If a vertex has multiple triangle fan boundaries, there is ambiguity in how the boundary halfedges are linked
  // together into components.  This arbitrary choice may be incompatible with the new face, in which case surgery
  // is required to correct the linkage.  We check for all errors before we do any actual surgery so that add_face
  // makes changes only when it succeeds.
  #define PREPARE(a,b,c) \
    const auto c = a.valid() && b.valid() && next(a)!=b ? right_around_dst_to_boundary(*this,b) : HalfedgeId(); \
    if (c.valid() && b==c) \
      add_face_error(v,", result would consist of a closed ring plus extra triangles");
  PREPARE(e0,e1,c0)
  PREPARE(e1,e2,c1)
  PREPARE(e2,e0,c2)
  // If we get to this point, all checks have passed, and we can safely add the new face.  For each pair of
  // adjacent ei,ej, we want next(ei)==ej.  We've already computed ci = the other side of ej's triangle fan.
  // Here is the surgery:
  #define RELINK(a,b,c) \
    if (c.valid()) { \
      const auto na = next(a), pb = prev(b), nc = next(c); \
      unsafe_boundary_link(a,b); \
      unsafe_boundary_link(c,na); \
      unsafe_boundary_link(pb,nc); \
    }
  RELINK(e0,e1,c0)
  RELINK(e1,e2,c1)
  RELINK(e2,e0,c2)

  // Create a new face, including three implicit halfedges.
  n_faces_++;
  const auto f = faces_.append(FaceInfo());
  faces_[f].vertices = v;

  // Look up all connectivity we'll need to restructure the mesh.  This includes the reverses
  // r0,r1,r2 of each of e0,e1,e2 (the old halfedges of the triangle), and their future prev/next links.
  // If r0,r1,r2 don't exist yet, they are allocated.
  const auto ve0 = halfedge(v.x),
             ve1 = halfedge(v.y),
             ve2 = halfedge(v.z);
  #define REVERSE(i) \
    const auto r##i = e##i.valid() ? boundaries_[-1-e##i.id].reverse \
                                   : unsafe_new_boundary(*this,v[(i+1)%3],HalfedgeId(3*f.id+i));
  REVERSE(0)
  REVERSE(1)
  REVERSE(2)
  #define NEAR(i,in,ip) \
    HalfedgeId prev##i, next##i; \
    if (!e##i.valid()) { \
      prev##i = ve##in.valid() ? boundaries_[-1-(e##in.valid() ? e##in : ve##in).id].prev : r##in; \
      next##i = ve##i.valid() ? e##ip.valid() ? boundaries_[-1-e##ip.id].next : ve##i : r##ip; \
    }
  NEAR(0,1,2)
  NEAR(1,2,0)
  NEAR(2,0,1)

  // Link everything together
  #define LINK(i) \
    unsafe_set_reverse(f,i,r##i); \
    if (!e##i.valid()) { \
      unsafe_boundary_link(prev##i,r##i); \
      unsafe_boundary_link(r##i,next##i); \
    }
  LINK(0)
  LINK(1)
  LINK(2)

  // Delete edges that used to be boundaries.  We do this after allocating new boundary edges
  // so that fields we need are not overwritten before they are used.
  if (e0.valid()) unsafe_set_deleted(e0);
  if (e1.valid()) unsafe_set_deleted(e1);
  if (e2.valid()) unsafe_set_deleted(e2);

  // Fix vertex to edge pointers to point to boundary halfedges if possible
  fix_vertex_to_edge(*this,v.x,r2);
  fix_vertex_to_edge(*this,v.y,r0);
  fix_vertex_to_edge(*this,v.z,r1);

  // All done!
  return f;
}

void CornerMesh::add_faces(RawArray<const Vector<int,3>> vs) {
  // TODO: We desperately need a batch insertion routine.
  for (auto& v : vs)
    add_face(Vector<VertexId,3>(v));
}

void CornerMesh::split_face(const FaceId f, const VertexId c) {
  OTHER_ASSERT(valid(f) && isolated(c));
  const auto v = faces_[f].vertices;
  const auto n = faces_[f].neighbors;
  const int f_base = faces_.size();
  n_faces_ += 2;
  faces_.flat.resize(f_base+2,false);
  const auto fs = vec(f,FaceId(f_base),FaceId(f_base+1));
  #define UPDATE(i) { \
    const int ip = (i+2)%3, in = (i+1)%3; \
    faces_[fs[i]].vertices.set(v[i],v[in],c); \
    unsafe_set_reverse(fs[i],0,n[i]); \
    faces_[fs[i]].neighbors[1] = HalfedgeId(3*fs[in].id+2); \
    faces_[fs[i]].neighbors[2] = HalfedgeId(3*fs[ip].id+1); \
    if (i && vertex_to_edge_[v[i]].id==3*f.id+i) \
      vertex_to_edge_[v[i]] = HalfedgeId(3*fs[i].id); }
  UPDATE(0)
  UPDATE(1)
  UPDATE(2)
  #undef UPDATE
  vertex_to_edge_[c] = halfedge(f,2);
}

VertexId CornerMesh::split_face(FaceId f) {
  const auto c = add_vertex();
  split_face(f,c);
  return c;
}

bool CornerMesh::is_flip_safe(HalfedgeId e0) const {
  if (!valid(e0) || is_boundary(e0))
    return false;
  const auto e1 = reverse(e0);
  if (is_boundary(e1))
    return false;
  const auto o0 = src(prev(e0)),
             o1 = src(prev(e1));
  return o0!=o1 && !halfedge(o0,o1).valid();
}

HalfedgeId CornerMesh::flip_edge(HalfedgeId e) {
  if (!is_flip_safe(e))
    throw RuntimeError(format("CornerMesh::flip_edge: edge flip %d is invalid",e.id));
  return unsafe_flip_edge(e);
}

HalfedgeId CornerMesh::unsafe_flip_edge(HalfedgeId e0) {
  const auto e1 = reverse(e0);
  const auto f0 = face(e0),
             f1 = face(e1);
  const auto n0 = next(e0), p0 = prev(e0),
             n1 = next(e1), p1 = prev(e1),
             rn0 = reverse(n0), rp0 = reverse(p0),
             rn1 = reverse(n1), rp1 = reverse(p1);
  const auto v0 = src(e0), o0 = src(p0),
             v1 = src(e1), o1 = src(p1);
  faces_[f0].vertices = vec(o0,o1,v1);
  faces_[f1].vertices = vec(o1,o0,v0);
  faces_[f0].neighbors.x = HalfedgeId(3*f1.id);
  faces_[f1].neighbors.x = HalfedgeId(3*f0.id);
  unsafe_set_reverse(f0,1,rp1);
  unsafe_set_reverse(f0,2,rn0);
  unsafe_set_reverse(f1,1,rp0);
  unsafe_set_reverse(f1,2,rn1);
  // Fix vertex to edge links
  auto &ve0 = vertex_to_edge_[v0],
       &ve1 = vertex_to_edge_[v1],
       &oe0 = vertex_to_edge_[o0],
       &oe1 = vertex_to_edge_[o1];
  if (ve0==e0 || ve0==n1) ve0 = HalfedgeId(3*f1.id+2);
  if (ve1==e1 || ve1==n0) ve1 = HalfedgeId(3*f0.id+2);
  if (oe0==p0) oe0 = HalfedgeId(3*f0.id);
  if (oe1==p1) oe1 = HalfedgeId(3*f1.id);
  return HalfedgeId(3*f0.id);
}

void CornerMesh::assert_consistent() const {
  // Check simple vertex properties
  int actual_vertices = 0;
  for (const auto v : vertices()) {
    actual_vertices++;
    const auto e = halfedge(v);
    if (e.valid())
      OTHER_ASSERT(valid(e) && src(e)==v);
  }
  OTHER_ASSERT(actual_vertices==n_vertices());

  // Check simple face properties
  int actual_faces = 0;
  for (const auto f : faces()) {
    actual_faces++;
    const auto es = halfedges(f);
    OTHER_ASSERT(es.x==halfedge(f));
    for (int i=0;i<3;i++) {
      const auto e = es[i];
      OTHER_ASSERT(valid(e) && face(e)==f);
      OTHER_ASSERT(src(e)==vertex(f,i) && dst(e)==vertex(f,(i+1)%3));
      OTHER_ASSERT(next(e)==halfedge(f,(i+1)%3));
    }
  }
  OTHER_ASSERT(actual_faces==n_faces());

  // Check simple edge properties
  OTHER_ASSERT(!((3*n_faces()+n_boundary_edges())&1));
  for (const auto e : halfedges()) {
    const auto f = face(e);
    const auto p = prev(e), n = next(e), r = reverse(e);
    OTHER_ASSERT(e!=p && e!=n && e!=r);
    OTHER_ASSERT(src(r)==src(n));
    OTHER_ASSERT(src(e)!=dst(e));
    OTHER_ASSERT(valid(p) && next(p)==e && face(p)==f);
    OTHER_ASSERT(valid(n) && prev(n)==e && face(n)==f);
    if (f.valid())
      OTHER_ASSERT(valid(f) && halfedges(f).contains(e));
    else
      OTHER_ASSERT(face(r).valid());
    const auto ce = common_halfedge(f,face(r));
    OTHER_ASSERT(ce.valid() && face(ce)==f && face(reverse(ce))==face(r));
  }

  // Check that all faces are triangles
  for (const auto f : faces()) {
    const auto e = halfedge(f);
    OTHER_ASSERT(e==next(next(next(e))));
  }

  // Check that no two halfedges share the same vertices
  {
    Hashtable<Vector<VertexId,2>> pairs;
    for (const auto e : halfedges())
      OTHER_ASSERT(pairs.set(vertices(e)));
  }

  // Check that all halfedges are reachable by swinging around their source vertices, and that
  // boundary vertices point to boundary halfedges.
  boost::dynamic_bitset<> seen(boundaries_.size()+3*faces_.size());
  for (const auto v : vertices())
    if (!isolated(v)) {
      bool boundary = false;
      for (const auto e : outgoing(v)) {
        OTHER_ASSERT(src(e)==v);
        seen[boundaries_.size()+e.id] = true;
        boundary |= is_boundary(e);
      }
      OTHER_ASSERT(boundary==is_boundary(v));
    }
  OTHER_ASSERT(seen.count()==size_t(2*n_edges()));

  // Check that all deleted boundary edges occur in our linked list
  int limit = boundaries_.size();
  int actual_deleted = 0;
  for (auto b=deleted_boundaries_;b.valid();b=boundaries_[-1-b.id].next) {
    OTHER_ASSERT(boundaries_.valid(-1-b.id));
    OTHER_ASSERT(limit--);
    actual_deleted++;
  }
  OTHER_ASSERT(n_boundary_edges()+actual_deleted==boundaries_.size());
}

Array<Vector<int,3>> CornerMesh::elements() const {
  Array<Vector<int,3>> tris(n_faces(),false);
  int i = 0;
  for (const auto f : faces())
    tris[i++] = Vector<int,3>(faces_[f].vertices);
  return tris;
}

bool CornerMesh::has_boundary() const {
  return n_boundary_edges()!=0;
}

bool CornerMesh::is_manifold() const {
  return !has_boundary();
}

bool CornerMesh::is_manifold_with_boundary() const {
  if (is_manifold()) // Finish in O(1) time if possible
    return true;
  for (const auto v : vertices()) {
    const auto start = halfedge(v);
    if (is_boundary(start)) { // If the first halfedge is a boundary, we need to check for a second
      auto e = start;
      for (;;) {
        e = left(e);
        if (e==start)
          break;
        if (is_boundary(e)) // If there are two boundary halfedges, this vertex is bad
          return false;
      }
    }
  }
  return true;
}

bool CornerMesh::has_isolated_vertices() const {
  for (const auto v : vertices())
    if (isolated(v))
      return true;
  return false;
}

int CornerMesh::degree(VertexId v) const {
  int degree = 0;
  for (OTHER_UNUSED auto _ : outgoing(v))
    degree++;
  return degree;
}

NestedArray<HalfedgeId> CornerMesh::boundary_loops() const {
  NestedArray<HalfedgeId> loops;
  boost::dynamic_bitset<> seen(boundaries_.size());
  for (const auto start : boundary_edges())
    if (!seen[-1-start.id]) {
      auto e = start;
      do {
        loops.flat.append(e);
        seen[-1-e.id] = true;
        e = next(e);
      } while (e!=start);
      loops.offsets.const_cast_().append(loops.flat.size());
    }
  return loops;
}

void CornerMesh::unsafe_delete_last_vertex() {
  const VertexId v(vertex_to_edge_.size()-1);
  // Delete all incident faces
  while (!isolated(v))
    unsafe_delete_face(face(reverse(halfedge(v))));
  // Remove the vertex
  vertex_to_edge_.flat.pop();
  n_vertices_--;
}

void CornerMesh::unsafe_delete_face(const FaceId f) {
  // Look up connectivity of neighboring boundary edges, then delete them
  const auto e = faces_[f].neighbors;
  Vector<HalfedgeId,2> near[3]; // (prev,next) for each neighbor boundary edge
  for (int i=0;i<3;i++)
    if (e[i].id<0) {
      const auto& B = boundaries_[-1-e[i].id];
      near[i] = vec(B.prev,B.next);
      unsafe_set_deleted(e[i]);
    }

  // Create any new boundary edges and set src and reverse
  HalfedgeId b[3];
  for (int i=0;i<3;i++)
    if (e[i].id>=0) {
      b[i] = unsafe_new_boundary(*this,faces_[f].vertices[i],e[i]);
      const int fi = e[i].id/3;
      faces_.flat[fi].neighbors[e[i].id-3*fi] = b[i];
    }

  // Fix connectivity around each vertex: link together prev/next adjacent edges and update vertex to edge pointers
  for (int i=0;i<3;i++) {
    const int ip = i?i-1:2;
    const auto v = faces_[f].vertices[i];
    const auto prev = e[ip].id>=0 ? b[ip] : near[ip].x,
               next = e[i ].id>=0 ? b[i ] : near[i ].y;
    if (e[i].id>=0 || e[ip].id>=0 || prev!=e[i]) {
      unsafe_boundary_link(prev,next);
      vertex_to_edge_[v] = next;
    } else if (vertex_to_edge_[v]==e[ip])
      vertex_to_edge_[v] = HalfedgeId();
  }

  // Rename the last face to f
  const FaceId f1(faces_.size()-1);
  if (f.id<f1.id) {
    assert(!deleted(f1));
    const auto I = faces_[f1];
    faces_[f].vertices = I.vertices;
    for (int i=0;i<3;i++) {
      unsafe_set_reverse(f,i,I.neighbors[i]);
      if (vertex_to_edge_[I.vertices[i]].id==3*f1.id+i)
        vertex_to_edge_[I.vertices[i]].id = 3*f.id+i;
    }
  }

  // Remove the deleted face
  faces_.flat.pop();
  n_faces_--;
}

void CornerMesh::dump_internals() const {
  cout << format("corner mesh dump:\n  vertices: %d\n",n_vertices());
  for (const auto v : vertices()) {
    if (isolated(v))
      cout << format("    v%d: e_\n",v.id);
    else {
      // Print all outgoing halfedges, taking care not to crash or loop forever if the structure is invalid
      cout << format("    v%d:",v.id);
      const auto start = halfedge(v);
      auto e = start;
      int limit = n_faces()+1;
      do {
        cout << ' '<<str_halfedge(e);
        if (!valid(e) || !valid(prev(e))) {
          cout << " boom";
          break;
        } else if (!limit--) {
          cout << " ...";
          break;
        } else
          e = reverse(prev(e));
      } while (e!=start);
      cout << (valid(start)?is_boundary(start)?", b\n":", i\n":"\n");
    }
  }
  cout << format("  faces: %d\n",n_faces());
  for (const auto f : faces()) {
    const auto v = vertices(f);
    const auto n = faces_[f].neighbors;
    cout << format("    f%d: v%d v%d v%d, %s %s %s\n",f.id,v.x.id,v.y.id,v.z.id,str_halfedge(n.x),str_halfedge(n.y),str_halfedge(n.z));
  }
  cout << format("  boundary edges: %d\n",n_boundary_edges());
  for (const auto e : boundary_edges()) {
    const auto n = next(e);
    cout << format("    %s: v%d %s, %s %s, r %s\n",str_halfedge(e),src(e).id,(valid(n)?format("v%d",src(n).id):"boom"),str_halfedge(prev(e)),str_halfedge(n),str_halfedge(reverse(e)));
  }
  cout << endl;
}

void CornerMesh::permute_vertices(RawArray<const int> permutation, bool check) {
  OTHER_ASSERT(n_vertices()==permutation.size());
  OTHER_ASSERT(n_vertices()==vertex_to_edge_.size()); // Require no deleted vertices

  // Permute vertex_to_edge_ out of place
  Array<HalfedgeId> new_vertex_to_edge(vertex_to_edge_.size(),false);
  if (check) {
    new_vertex_to_edge.fill(HalfedgeId(deleted_id));
    for (const auto v : unsafe_vertices()) {
      const int pv = permutation[v.id];
      OTHER_ASSERT(new_vertex_to_edge.valid(pv));
      new_vertex_to_edge[pv] = vertex_to_edge_[v];
    }
    OTHER_ASSERT(!new_vertex_to_edge.contains(HalfedgeId(deleted_id)));
  } else
    for (const auto v : unsafe_vertices())
      new_vertex_to_edge[permutation[v.id]] = vertex_to_edge_[v];
  vertex_to_edge_.flat = new_vertex_to_edge;

  // The other arrays can be modified in place
  for (auto& f : faces_.flat)
    if (f.vertices.x.id!=deleted_id)
      for (auto& v : f.vertices)
        v = VertexId(permutation[v.id]);
  for (auto& b : boundaries_)
    if (b.src.id!=deleted_id)
      b.src = VertexId(permutation[b.src.id]);
}

static int corner_random_edge_flips(CornerMesh& mesh, const int attempts, const uint128_t key) {
  int flips = 0;
  if (mesh.n_faces()) {
    const auto random = new_<Random>(key);
    for (int a=0;a<attempts;a++) {
      const HalfedgeId e(random->uniform<int>(0,3*mesh.faces_.size()));
      if (mesh.is_flip_safe(e)) {
        const auto f0 = mesh.face(e),
                   f1 = mesh.face(mesh.reverse(e)); 
        const auto ef = mesh.unsafe_flip_edge(e);
        OTHER_ASSERT(mesh.face(ef)==f0 && mesh.face(mesh.reverse(ef))==f1);
        flips++;
      }
    }
  }
  return flips;
}

static void corner_random_face_splits(CornerMesh& mesh, const int splits, const uint128_t key) {
  if (mesh.n_faces()) {
    const auto random = new_<Random>(key);
    for (int a=0;a<splits;a++) {
      const FaceId f(random->uniform<int>(0,mesh.faces_.size()));
      const auto v = mesh.split_face(f);
      OTHER_ASSERT(mesh.face(mesh.halfedge(v))==f);
    }
  }
}

static void corner_mesh_destruction_test(CornerMesh& mesh, const uint128_t key) {
  const auto random = new_<Random>(key);
  while (mesh.n_vertices()) {
    OTHER_ASSERT(mesh.n_faces()==mesh.faces_.size());
    const int target = random->uniform<int>(0,1+2*mesh.n_faces());
    if (target<mesh.n_faces())
      mesh.unsafe_delete_face(FaceId(target));
    else
      mesh.unsafe_delete_last_vertex();
    mesh.assert_consistent();
  }
}

}
using namespace other;

void wrap_corner_mesh() {
  typedef CornerMesh Self;
  Class<Self>("CornerMesh")
    .OTHER_INIT()
    .OTHER_METHOD(copy)
    .OTHER_GET(n_vertices)
    .OTHER_GET(n_boundary_edges)
    .OTHER_GET(n_edges)
    .OTHER_GET(n_faces)
    .OTHER_GET(chi)
    .OTHER_METHOD(elements)
    .OTHER_METHOD(has_boundary)
    .OTHER_METHOD(is_manifold)
    .OTHER_METHOD(is_manifold_with_boundary)
    .OTHER_METHOD(has_isolated_vertices)
    .OTHER_METHOD(boundary_loops)
    .OTHER_METHOD(add_vertex)
    .OTHER_METHOD(add_vertices)
    .OTHER_METHOD(add_face)
    .OTHER_METHOD(add_faces)
    .OTHER_METHOD(assert_consistent)
    .OTHER_METHOD(dump_internals)
    ;

  // For testing purposes
  OTHER_FUNCTION(corner_random_edge_flips)
  OTHER_FUNCTION(corner_random_face_splits)
  OTHER_FUNCTION(corner_mesh_destruction_test)
}
