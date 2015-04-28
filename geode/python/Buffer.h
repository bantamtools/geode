//#####################################################################
// Class Buffer
//#####################################################################
//
// This is a minimal python object containing a buffer of memory.
// Since it's a python object, it has a reference count, and can be shared.
// No destructors are called, so is_trivially_destructible<T> must be true.
//
//#####################################################################
#pragma once

#include <geode/python/config.h>
#include <geode/python/forward.h>
#include <geode/utility/config.h>
#include <geode/utility/type_traits.h>
#include <stdlib.h>
namespace geode {

struct Buffer {
  GEODE_DECLARE_TYPE(GEODE_CORE_EXPORT)
  GEODE_PY_OBJECT_HEAD // Contains a reference count and a pointer to the type object
  GEODE_ALIGNED(16) char data[1]; // Use 16 byte alignment for SSE purposes.  Should be data[0], but Windows would complain.

private:
  Buffer(); // Should never be called
  Buffer(const Buffer&);
  void operator=(const Buffer&);
public:

  template<class T> static Buffer* new_(const int m) {
    static_assert(is_trivially_destructible<T>::value,"Array<T> never calls destructors, so T cannot have any");

#if defined(__MINGW32__)
    // MinGW headers break declaration of _aligned_malloc unless you are very careful about include order
    // We use __mingw_aligned_malloc which seems more robust
    Buffer* self = (Buffer*)__mingw_aligned_malloc(16+m*sizeof(T),16);
#elif defined(_WIN32)
    // Windows doesn't guarantee 16 byte alignment, so use _aligned_malloc
    Buffer* self = (Buffer*)_aligned_malloc(16+m*sizeof(T),16);
#else
    // On other platforms, malloc should be 16 byte aligned
    Buffer* self = (Buffer*)malloc(16+m*sizeof(T));
#endif
    return GEODE_PY_OBJECT_INIT(self,&pytype);
  }
};

// Check alignment constraints
static_assert(offsetof(Buffer,data)==16,"data must be 16 byte aligned for SSE purposes");

}
