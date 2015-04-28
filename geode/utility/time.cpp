// Current time

#include <geode/utility/time.h>
#if defined(_WIN32)
#ifndef __MINGW32__
#pragma comment(lib, "winmm.lib")
#endif
#include <windows.h>
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__APPLE__)
#include <sys/time.h>
#include <sys/resource.h>
#endif
namespace geode {

// Windows
#ifdef _WIN32

static double get_resolution() {
  __int64 frequency;
  QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
  return 1./frequency;
}
static double resolution = get_resolution();

double get_time() {
  __int64 time;
    QueryPerformanceCounter((LARGE_INTEGER*)&time);
    return resolution*time;
}

// Unix
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__APPLE__)

double get_time() {
  timeval tv;
  gettimeofday(&tv,0);
  return tv.tv_sec+1e-6*tv.tv_usec;
}

#else

#error "Don't know how to get time on this platform"

#endif

}
