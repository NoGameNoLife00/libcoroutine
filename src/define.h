#ifndef LIBCOROUTINE_DEFINE_H
#define LIBCOROUTINE_DEFINE_H

#include <coroutine>
#include <cstdlib>
#include <memory>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif // likely
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif // unlikely

namespace libcoro {
    class Scheduler;
    using std::coroutine_handle;

    template<typename Type = void>
    class Promise;

    class StateBase;

    template <typename Type = void>
    class Awaitable;

    template<class T>
    constexpr size_t AlignSize() {
        const size_t align_req = sizeof(void*) * 2;
        return std::is_empty_v<T> ? 0 :
               (sizeof(T) + align_req - 1) & ~(align_req - 1);
    }
}

#endif //LIBCOROUTINE_DEFINE_H
