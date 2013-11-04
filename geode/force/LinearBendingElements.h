//#####################################################################
// Class LinearBendingElements
//#####################################################################
#pragma once

#include <geode/force/Force.h>
#include <geode/mesh/forward.h>
#include <geode/vector/forward.h>
namespace geode {

template<class TV> class LinearBendingElements : public Force<TV> {
  typedef typename TV::Scalar T;
  enum Workaround {d=TV::m};
public:
  GEODE_DECLARE_TYPE(GEODE_CORE_EXPORT)
  typedef Force<TV> Base;
  typedef typename mpl::if_c<d==2,SegmentSoup,TriangleSoup>::type Mesh;

  const Ref<const Mesh> mesh;
  T stiffness,damping;
private:
  Ref<SparseMatrix> A; // only the upper triangle is stored
  Array<const TV> X;

protected:
  LinearBendingElements(const Mesh& mesh, Array<const TV> X);
public:
  ~LinearBendingElements();

  void update_position(Array<const TV> X, bool definite);
  void add_frequency_squared(RawArray<T> frequency_squared) const;
  T strain_rate(RawArray<const TV> V) const;
  T elastic_energy() const;
  void add_elastic_force(RawArray<TV> F) const;
  void add_elastic_differential(RawArray<TV> dF, RawArray<const TV> dX) const;
  void add_elastic_gradient_block_diagonal(RawArray<SymmetricMatrix<T,d>> dFdX) const;
  T damping_energy(RawArray<const TV> V) const;
  void add_damping_force(RawArray<TV> F, RawArray<const TV> V) const;

  int nodes() const;
  void structure(SolidMatrixStructure& structure) const;
  void add_elastic_gradient(SolidMatrix<TV>& matrix) const;
  void add_damping_gradient(SolidMatrix<TV>& matrix) const;
};

}
