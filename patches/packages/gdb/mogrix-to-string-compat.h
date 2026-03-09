/* mogrix compat: std::to_string and std::snprintf for IRIX
 * IRIX libstdc++ has _GLIBCXX_USE_C99_STDIO=0, disabling std::to_string
 * and std::snprintf. Provide minimal implementations. */
#ifndef MOGRIX_TO_STRING_COMPAT_H
#define MOGRIX_TO_STRING_COMPAT_H

#ifdef __cplusplus
#include <string>
#include <cstdio>

#if !_GLIBCXX_USE_C99_STDIO
namespace std {
  using ::snprintf;
  using ::sscanf;
  inline string to_string(int val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%d", val); return string(buf);
  }
  inline string to_string(long val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%ld", val); return string(buf);
  }
  inline string to_string(long long val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%lld", val); return string(buf);
  }
  inline string to_string(unsigned val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%u", val); return string(buf);
  }
  inline string to_string(unsigned long val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%lu", val); return string(buf);
  }
  inline string to_string(unsigned long long val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%llu", val); return string(buf);
  }
}
#endif

#endif /* __cplusplus */
#endif /* MOGRIX_TO_STRING_COMPAT_H */
