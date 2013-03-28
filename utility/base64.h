// Base 64 encoding and decoding
#pragma once

#include <othercore/array/RawArray.h>
#include <string>
namespace other {

OTHER_CORE_EXPORT string base64_encode(const string& src);
OTHER_CORE_EXPORT string base64_encode(RawArray<const uint8_t> src);

OTHER_CORE_EXPORT string base64_decode(const string& src);

}
