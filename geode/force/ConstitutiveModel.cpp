//#####################################################################
// Class ConstitutiveModel
//##################################################################### 
#include <geode/force/ConstitutiveModel.h>
#include <geode/force/DiagonalizedIsotropicStressDerivative.h>
#include <geode/python/Class.h>
#include <geode/vector/DiagonalMatrix.h>
namespace geode {

typedef real T;
template<> GEODE_DEFINE_TYPE(ConstitutiveModel<T,2>)
template<> GEODE_DEFINE_TYPE(ConstitutiveModel<T,3>)

template<class T,int d> ConstitutiveModel<T,d>::ConstitutiveModel(T failure_threshold) 
  : failure_threshold(failure_threshold) {}

template<class T,int d> ConstitutiveModel<T,d>::~ConstitutiveModel() {}

template<class T,int d> DiagonalMatrix<T,d> ConstitutiveModel<T,d>::clamp_f(const DiagonalMatrix<T,d>& F) const {
  return F.clamp_min(failure_threshold);
}

template class ConstitutiveModel<T,2>;
template class ConstitutiveModel<T,3>;

}
using namespace geode;

void wrap_constitutive_model() {
  {typedef ConstitutiveModel<T,2> Self;
  Class<Self>("ConstitutiveModel2d");}

  {typedef ConstitutiveModel<T,3> Self;
  Class<Self>("ConstitutiveModel3d");}
}
