//#####################################################################
// Class ExrFile
//#####################################################################
#pragma once

#include <othercore/array/forward.h>
#include <othercore/vector/forward.h>
#include <string>
namespace other {

template<class T> class ExrFile {
public:
OTHER_CORE_EXPORT static Array<Vector<T,3>,2> read(const std::string& filename);
OTHER_CORE_EXPORT static void write(const std::string& filename,RawArray<const Vector<T,3>,2> image);
OTHER_CORE_EXPORT static bool is_supported();
};

}
