#ifndef LIBCOROUTINE_DEFINE_H
#define LIBCOROUTINE_DEFINE_H

#include <coroutine>
#include <cstdlib>
#include <memory>
#include <mutex>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif // likely
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif // unlikely

namespace libcoro {
    class Scheduler;
//    using std::coroutine_handle;

    template<typename Type = void>
    class Promise;

    class StateBase;

    class SpinLock;

    template <typename Type = std::nullptr_t, typename Alloc = std::allocator<char>>
    class Generator;

    template <typename Type = void>
    class Awaitable;

    template <typename PromiseT = void>
    using coroutine_handle = std::coroutine_handle<PromiseT>;
    using suspend_always = std::suspend_always;
    using suspend_nerver = std::suspend_never;

    template<class... Mutexes>
    using scoped_lock = std::scoped_lock<Mutexes...>;

    template<class T>
    constexpr size_t AlignSize() {
        const size_t align_req = sizeof(void*) * 2;
        return std::is_empty_v<T> ? 0 :
               (sizeof(T) + align_req - 1) & ~(align_req - 1);
    }
    template<class T>
    struct RemoveCvref
    {
        typedef std::remove_cv_t<std::remove_reference_t<T>> type;
    };
    template<class T>
    using RemoveCvrefT = typename RemoveCvref<T>::type;
}

#endif //LIBCOROUTINE_DEFINE_H
