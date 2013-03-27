//#####################################################################
// Class PlasticityModel
//##################################################################### 
#include <othercore/force/PlasticityModel.h>
#include <othercore/python/Class.h>
#include <othercore/vector/SymmetricMatrix.h>
namespace other{

typedef real T;
template<> OTHER_DEFINE_TYPE(PlasticityModel<T,2>)
template<> OTHER_DEFINE_TYPE(PlasticityModel<T,3>)

}

void wrap_plasticity_model()
{
    using namespace other;

    {typedef PlasticityModel<T,2> Self;
    Class<Self>("PlasticityModel2d");}

    {typedef PlasticityModel<T,3> Self;
    Class<Self>("PlasticityModel3d");}
}
