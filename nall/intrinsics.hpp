#pragma once

#include <mutex>

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include <dlfcn.h>
#include <pwd.h>
#include <grp.h>

namespace nall {
  using uint = unsigned;
}

/* Compiler detection */

#if defined(__clang__) || defined(__GNUC__)
  #define noinline   __attribute__((noinline))
  #define alwaysinline  inline __attribute__((always_inline))
#else
  #define noinline
  #define alwaysinline  inline
#endif

/* Platform detection */

namespace nall {

#if defined(_WIN32)
  #define PLATFORM_WINDOWS
  #define API_WINDOWS
#elif defined(__APPLE__)
  #define PLATFORM_MACOS
  #define API_POSIX
#elif defined(__ANDROID__)
  #define PLATFORM_ANDROID
  #define API_POSIX
#elif defined(linux) || defined(__linux__)
  #define PLATFORM_LINUX
  #define API_POSIX
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define PLATFORM_BSD
  #define API_POSIX
#else
  #warning "unable to detect platform"
  #define PLATFORM_UNKNOWN
  #define API_UNKNOWN
#endif

}
