//#####################################################################
// Class SurfacePins
//#####################################################################
#include <othercore/force/SurfacePins.h>
#include <othercore/array/NdArray.h>
#include <othercore/array/IndirectArray.h>
#include <othercore/structure/Pair.h>
#include <othercore/geometry/ParticleTree.h>
#include <othercore/geometry/SimplexTree.h>
#include <othercore/python/Class.h>
#include <othercore/utility/Log.h>
#include <othercore/vector/SolidMatrix.h>
#include <othercore/vector/SymmetricMatrix.h>
namespace other {

using Log::cout;
using std::endl;

typedef real T;
typedef Vector<T,3> TV;
OTHER_DEFINE_TYPE(SurfacePins)

SurfacePins::SurfacePins(Array<const int> particles, Array<const T> mass, TriangleMesh& target_mesh, Array<const TV> target_X, NdArray<const T> stiffness, NdArray<const T> damping_ratio)
  : particles(particles)
  , target_mesh(ref(target_mesh))
  , target_X(target_X)
  , mass(mass)
  , k(particles.size(),false)
  , kd(particles.size(),false)
  , node_X(particles.size(),false)
  , target_tree(new_<SimplexTree<TV,2>>(ref(target_mesh),target_X,10))
  , info(particles.size(),false)
{
  max_node = particles.size()?particles.max()+1:0;
  OTHER_ASSERT(mass.size()>=max_node);
  OTHER_ASSERT(target_mesh.nodes()<=target_X.size());
  OTHER_ASSERT(stiffness.rank()==0 || (stiffness.rank()==1 && stiffness.shape[0]==particles.size()));
  OTHER_ASSERT(damping_ratio.rank()==0 || (damping_ratio.rank()==1 && damping_ratio.shape[0]==particles.size()));

  for (int i=0;i<particles.size();i++) {
    int p = particles[i]; 
    T stiffness_ = stiffness.rank()?stiffness[i]:stiffness();
    T damping_ratio_ = damping_ratio.rank()?damping_ratio[i]:damping_ratio();
    k[i] = stiffness_*mass[p];
    kd[i] = 2*damping_ratio_*mass[p]*sqrt(stiffness_);
  }
}

SurfacePins::~SurfacePins() {}

int SurfacePins::nodes() const {
  return max_node;
}

void SurfacePins::structure(SolidMatrixStructure& structure) const {
  OTHER_ASSERT(structure.size()==mass.size());
  OTHER_NOT_IMPLEMENTED("changing matrix topology");
}

void SurfacePins::update_position(Array<const TV> X_, bool definite) {
  OTHER_ASSERT(X_.size()==mass.size());
  node_X.copy(X_.subset(particles));
  if (!node_tree)
    node_tree = new_<ParticleTree<TV>>(node_X,10);
  else
    node_tree->update(); // update for changes to node_X
  // Compute distances and directions
  evaluate_surface_levelset(*node_tree,*target_tree,info,1e10,false);
}

Array<TV> SurfacePins::closest_points(Array<const TV> X) {
  update_position(X,false); 
  Array<TV> closest(particles.size(),false);
  for (int i=0;i<particles.size();i++)
    closest[i] = X[particles[i]]-info[i].phi*info[i].normal;
  return closest;
}

void SurfacePins::add_frequency_squared(RawArray<T> frequency_squared) const {
  OTHER_ASSERT(frequency_squared.size()==mass.size());
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    frequency_squared[p]+=k[i]/mass[p];
  }
}

T SurfacePins::elastic_energy() const {
  T energy = 0;
  for (int i=0;i<particles.size();i++)
    energy += k[i]*sqr(info[i].phi);
  return energy/2;
}

void SurfacePins::add_elastic_force(RawArray<TV> F) const {
  OTHER_ASSERT(F.size()==mass.size());
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    F[p] -= k[i]*info[i].phi*info[i].normal;
  }
}

void SurfacePins::add_elastic_gradient(SolidMatrix<TV>& matrix) const {
  OTHER_ASSERT(matrix.size()==mass.size());
  OTHER_NOT_IMPLEMENTED();
}

void SurfacePins::add_elastic_differential(RawArray<TV> dF, RawArray<const TV> dX) const {
  OTHER_ASSERT(dF.size()==mass.size());
  OTHER_ASSERT(dX.size()==mass.size());
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    dF[p] -= k[i]*dot(dX[p],info[i].normal)*info[i].normal; // Ignores a curvature term if the closest point is on an edge or vertex
  }
}

void SurfacePins::add_elastic_gradient_block_diagonal(RawArray<SymmetricMatrix<T,m>> dFdX) const {
  OTHER_ASSERT(dFdX.size()==mass.size());
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    dFdX[p] -= scaled_outer_product(k[i],info[i].normal); // Ignores a curvature term if the closest point is on an edge or vertex
  }
}

T SurfacePins::damping_energy(RawArray<const TV> V) const {
  OTHER_ASSERT(V.size()==mass.size());
  T energy = 0;
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    energy += kd[i]*sqr(dot(V[p],info[i].normal));
  }
  return energy/2;
}

void SurfacePins::add_damping_force(RawArray<TV> F, RawArray<const TV> V) const {
  OTHER_ASSERT(V.size()==mass.size());
  OTHER_ASSERT(F.size()==mass.size());
  for (int i=0;i<particles.size();i++) {
    int p = particles[i];
    F[p] -= kd[i]*dot(V[p],info[i].normal)*info[i].normal;
  }
}

void SurfacePins::add_damping_gradient(SolidMatrix<TV>& matrix) const {
  OTHER_ASSERT(matrix.size()==mass.size());
  OTHER_NOT_IMPLEMENTED();
}

T SurfacePins::strain_rate(RawArray<const TV> V) const {
  return 0;
}

}
using namespace other;

void wrap_surface_pins() {
  typedef SurfacePins Self;
  Class<Self>("SurfacePins")
    .OTHER_INIT(Array<const int>,Array<const T>,TriangleMesh&,Array<const TV>,NdArray<const T>,NdArray<const T>)
    .OTHER_METHOD(closest_points)
    ;
}
