// C++17/20 utility polyfills for GCC 9 libstdc++ on IRIX
// Force-included by irix-cxx for all C++ compilations.
// Provides: std::to_string, std::cmp_*, quick_exit
// Auto-disables individual features when the real implementation exists.
#pragma once
#ifdef __cplusplus

// --- std::to_string polyfill ---
// GCC 9 libstdc++ has _GLIBCXX_USE_C99_STDIO=0 which disables std::to_string.
// Previously shimmed per-package in cmake, doxygen, gdb, btop.
// Now provided systemically.
#include <string>
#include <cstdio>

#if !defined(_GLIBCXX_USE_C99_STDIO) || !_GLIBCXX_USE_C99_STDIO
#ifndef _MOGRIX_TO_STRING_POLYFILL
#define _MOGRIX_TO_STRING_POLYFILL
namespace std {
    inline string to_string(int v)                { char b[32]; snprintf(b,sizeof(b),"%d",v); return b; }
    inline string to_string(long v)               { char b[32]; snprintf(b,sizeof(b),"%ld",v); return b; }
    inline string to_string(long long v)          { char b[32]; snprintf(b,sizeof(b),"%lld",v); return b; }
    inline string to_string(unsigned v)           { char b[32]; snprintf(b,sizeof(b),"%u",v); return b; }
    inline string to_string(unsigned long v)      { char b[32]; snprintf(b,sizeof(b),"%lu",v); return b; }
    inline string to_string(unsigned long long v) { char b[32]; snprintf(b,sizeof(b),"%llu",v); return b; }
    inline string to_string(float v)              { char b[64]; snprintf(b,sizeof(b),"%f",v); return b; }
    inline string to_string(double v)             { char b[64]; snprintf(b,sizeof(b),"%f",v); return b; }
    inline string to_string(long double v)        { char b[64]; snprintf(b,sizeof(b),"%Lf",v); return b; }
}
#endif
#endif

// --- std::cmp_* integer comparison (C++20 <utility>) ---
// Safe integer comparison between signed/unsigned types.
#ifndef __cpp_lib_integer_comparison_functions
#ifndef _MOGRIX_CMP_POLYFILL
#define _MOGRIX_CMP_POLYFILL
#include <type_traits>
#include <limits>
namespace std {
    template <class T, class U>
    constexpr bool cmp_equal(T t, U u) noexcept {
        if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
            return t == u;
        else if constexpr (std::is_signed_v<T>)
            return t >= 0 && static_cast<std::make_unsigned_t<T>>(t) == u;
        else
            return u >= 0 && t == static_cast<std::make_unsigned_t<U>>(u);
    }
    template <class T, class U>
    constexpr bool cmp_not_equal(T t, U u) noexcept { return !cmp_equal(t, u); }
    template <class T, class U>
    constexpr bool cmp_less(T t, U u) noexcept {
        if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
            return t < u;
        else if constexpr (std::is_signed_v<T>)
            return t < 0 || static_cast<std::make_unsigned_t<T>>(t) < u;
        else
            return u >= 0 && t < static_cast<std::make_unsigned_t<U>>(u);
    }
    template <class T, class U>
    constexpr bool cmp_greater(T t, U u) noexcept { return cmp_less(u, t); }
    template <class T, class U>
    constexpr bool cmp_less_equal(T t, U u) noexcept { return !cmp_greater(t, u); }
    template <class T, class U>
    constexpr bool cmp_greater_equal(T t, U u) noexcept { return !cmp_less(t, u); }
}
#endif
#endif

// --- quick_exit ---
// GCC 9 libstdc++ may not provide quick_exit in <cstdlib> on IRIX.
#include <cstdlib>
#ifndef _MOGRIX_QUICK_EXIT_POLYFILL
#define _MOGRIX_QUICK_EXIT_POLYFILL
#if !defined(quick_exit) && !defined(_GLIBCXX_HAVE_QUICK_EXIT)
extern "C" {
    // IRIX libc has _exit but not quick_exit. Use _exit as fallback.
    [[noreturn]] inline void quick_exit(int status) noexcept { _exit(status); }
}
#endif
#endif

// --- std::string::contains (C++23, commonly used) ---
// btop and other modern C++ code uses str.contains("x")
// Not available in GCC 9.
// Can't polyfill as a member function, but packages can use
// str.find("x") != std::string::npos instead.
// Documented here for reference — sed fix needed per-package.

#endif // __cplusplus
