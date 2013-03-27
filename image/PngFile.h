//#####################################################################
// Class PngFile (requires libpng)
//#####################################################################
#pragma once

#include <string>
#include <vector>
#include <othercore/array/forward.h>
#include <othercore/vector/forward.h>
namespace other {

template<class T> class PngFile {
public:
OTHER_CORE_EXPORT static Array<Vector<T,3>,2> read(const std::string& filename);
OTHER_CORE_EXPORT static Array<Vector<T,4>,2> read_alpha(const std::string& filename);
OTHER_CORE_EXPORT static void write(const std::string& filename,RawArray<const Vector<T,3>,2> image);
OTHER_CORE_EXPORT static void write(const std::string& filename,RawArray<const Vector<T,4>,2> image);
OTHER_CORE_EXPORT static std::vector<unsigned char> write_to_memory(RawArray<const Vector<T,3>,2> image);
OTHER_CORE_EXPORT static std::vector<unsigned char> write_to_memory(RawArray<const Vector<T,4>,2> image);
OTHER_CORE_EXPORT static bool is_supported();
};

}
