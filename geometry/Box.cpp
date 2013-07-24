// Class Box

#include <othercore/geometry/Box.h>
namespace other {

#ifndef _WIN32
template Box<real> bounding_box(const RawArray<const real>&);
template Box<Vector<real,2>> bounding_box(const RawArray<const Vector<real,2>>&);
template Box<Vector<real,3>> bounding_box(const RawArray<const Vector<real,3>>&);
#endif

}
