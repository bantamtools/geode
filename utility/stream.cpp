// Stream I/O related utilities 

#include <othercore/utility/stream.h>
#include <othercore/python/exceptions.h>
namespace other {

void throw_unexpected_error(expect expected,char got) {
  throw ValueError(format("expected '%c' during stream input, got '%c'",expected.c,got));
}

}
