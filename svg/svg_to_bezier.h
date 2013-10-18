#pragma once

#include <othercore/svg/nanosvg/nanosvg.h>
#include <othercore/geometry/Bezier.h>
#include <othercore/array/Array.h>
#include <othercore/python/Ref.h>
#include <vector>
namespace other {

using std::vector;
using std::string;

OTHER_CORE_EXPORT vector<Ref<Bezier<2>>> svgfile_to_beziers(const string& filename);
OTHER_CORE_EXPORT vector<Ref<Bezier<2>>> svgstring_to_beziers(const string& svgstring);

}
