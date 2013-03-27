//#####################################################################
// Header Register
//#####################################################################
#pragma once

#include <othercore/array/forward.h>
#include <othercore/utility/config.h>
#include <othercore/vector/forward.h>
namespace other {

OTHER_CORE_EXPORT Frame<Vector<real,2> > rigid_register(RawArray<const Vector<real,2> > X0,RawArray<const Vector<real,2> > X1);
OTHER_CORE_EXPORT Frame<Vector<real,3> > rigid_register(RawArray<const Vector<real,3> > X0,RawArray<const Vector<real,3> > X1);

}
