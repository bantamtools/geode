// Path convenience functions
#pragma once

// We could use boost::filesystem for this, but it's nice to
// keep core independent of as much as possible.
// (also, boost::filesystem breaks with c++0x, as of 1.51,
// and changes its API too frequently to be acceptable)

#include <othercore/utility/config.h>
#include <string>
namespace other {
namespace path {

using std::string;

#ifdef _WIN32
const char sep = '\\';
static inline bool is_sep(char c) {
  return c=='\\' || c=='/';
}
#else
const char sep = '/';
static inline bool is_sep(char c) {
  return c==sep;
}
#endif

OTHER_CORE_EXPORT string join(const string& p0, const string& p1);
OTHER_CORE_EXPORT string join(const string& p0, const string& p1, const string& p2);
OTHER_CORE_EXPORT string join(const string& p0, const string& p1, const string& p2, const string& p3);

OTHER_CORE_EXPORT string extension(const string& path);

OTHER_CORE_EXPORT string remove_extension(const string& path);

OTHER_CORE_EXPORT string basename(const string& path);

OTHER_CORE_EXPORT string dirname(const string& path);

OTHER_CORE_EXPORT void copy_file(const string &from_path, const string &to_path);

// Add quotes and escape any special characters to make safe for use as shell argument
// Can throw a RuntimeError if the string contains any non-printable characters 
OTHER_CORE_EXPORT string shell_quote(const string& s);

}
}
