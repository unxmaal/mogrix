// IRIX C++20 polyfills for btop
// GCC 9 libstdc++ lacks <ranges>, <semaphore>, <source_location>

#pragma once

// --- source_location polyfill ---
#if __cplusplus < 202002L || !__has_include(<source_location>)
namespace std {
    struct source_location {
        static constexpr source_location current(
            const char* file = __builtin_FILE(),
            int line = __builtin_LINE(),
            const char* func = __builtin_FUNCTION()
        ) noexcept {
            source_location loc;
            loc._file = file;
            loc._line = line;
            loc._func = func;
            return loc;
        }
        constexpr const char* file_name() const noexcept { return _file; }
        constexpr int line() const noexcept { return _line; }
        constexpr const char* function_name() const noexcept { return _func; }
    private:
        const char* _file = "";
        int _line = 0;
        const char* _func = "";
    };
}
#endif

// --- binary_semaphore polyfill using POSIX ---
#if __cplusplus < 202002L || !__has_include(<semaphore>)
#include <mutex>
#include <condition_variable>
namespace std {
    class binary_semaphore {
    public:
        explicit binary_semaphore(int initial) : _count(initial) {}
        void release() {
            std::lock_guard<std::mutex> lk(_mtx);
            _count = 1;
            _cv.notify_one();
        }
        void acquire() {
            std::unique_lock<std::mutex> lk(_mtx);
            _cv.wait(lk, [this] { return _count > 0; });
            _count = 0;
        }
        bool try_acquire() {
            std::lock_guard<std::mutex> lk(_mtx);
            if (_count > 0) { _count = 0; return true; }
            return false;
        }
    private:
        std::mutex _mtx;
        std::condition_variable _cv;
        int _count;
    };
}
#endif
