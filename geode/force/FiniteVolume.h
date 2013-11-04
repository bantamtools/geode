//#####################################################################
// Class FiniteVolume
//#####################################################################
#pragma once

#include <geode/force/Force.h>
#include <geode/python/Ptr.h>
#include <geode/vector/Matrix.h>
#include <geode/force/StrainMeasure.h>
#include <geode/force/ConstitutiveModel.h>
namespace geode {

template<class TV,int d_>
class FiniteVolume : public Force<TV> {
  typedef typename TV::Scalar T;
  enum {m=TV::m};
  enum {d=d_};
public:
  GEODE_DECLARE_TYPE(GEODE_CORE_EXPORT)
  typedef Force<TV> Base;

  Ref<StrainMeasure<T,d>> strain;
  const T density;
  Ref<ConstitutiveModel<T,d>> model;
  Ptr<PlasticityModel<T,d>> plasticity;
protected:
  Array<T> Be_scales;
  Array<Matrix<T,m>> U;
  Array<Matrix<T,d>> V;
  Array<Matrix<T,d>> De_inverse_hat;
  Array<DiagonalMatrix<T,d>> Fe_hat;
  IsotropicConstitutiveModel<T,d>* isotropic;
  AnisotropicConstitutiveModel<T,d>* anisotropic;
  mutable bool stress_derivatives_valid,definite;
  mutable Array<DiagonalizedIsotropicStressDerivative<T,m,d>> dPi_dFe;
  mutable Array<DiagonalizedStressDerivative<T,d>> dP_dFe;
public:

protected:
  FiniteVolume(StrainMeasure<T,d>& strain, T density, ConstitutiveModel<T,d>& model, Ptr<PlasticityModel<T,d>> plasticity);
public:
  virtual ~FiniteVolume();

  void update_position(Array<const TV> X, bool definite);
  T elastic_energy() const;
  void add_elastic_force(RawArray<TV> F) const;
  void add_elastic_differential(RawArray<TV> dF, RawArray<const TV> dX) const;
  void add_elastic_gradient_block_diagonal(RawArray<SymmetricMatrix<T,m>> dFdX) const;
  T damping_energy(RawArray<const TV> V) const;
  void add_damping_force(RawArray<TV> F, RawArray<const TV> V) const;
  void add_frequency_squared(RawArray<T> frequency_squared) const;
  T strain_rate(RawArray<const TV> V) const;

  int nodes() const;
  void structure(SolidMatrixStructure& structure) const;
  void add_elastic_gradient(SolidMatrix<TV>& matrix) const;
  void add_damping_gradient(SolidMatrix<TV>& matrix) const;
private:
  void update_stress_derivatives() const;
};

}
