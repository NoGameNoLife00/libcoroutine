#ifndef LIBCOROUTINE_DEFINE_H
#define LIBCOROUTINE_DEFINE_H

#include <coroutine>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <macro_define.h>


#if LIBCORO_DEBUG
extern std::mutex g_coro_cout_mutex;
extern std::atomic<intptr_t> g_coro_state_count;
extern std::atomic<intptr_t> g_coro_task_count;
extern std::atomic<intptr_t> g_coro_evtctx_count;
extern std::atomic<intptr_t> g_coro_state_id;
#endif

namespace libcoro {
    class Scheduler;
//    using std::coroutine_handle;

    template<typename Type = void>
    class Promise;

    class StateBase;

    class spinlock;

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
