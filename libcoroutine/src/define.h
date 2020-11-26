#ifndef LIBCOROUTINE_DEFINE_H
#define LIBCOROUTINE_DEFINE_H

//#include <libcoro.h>
#ifdef LIBCORO_DEBUG_PTR
extern std::mutex g_coro_cout_mutex;
extern std::atomic<intptr_t> g_coro_state_count;
extern std::atomic<intptr_t> g_coro_task_count;
extern std::atomic<intptr_t> g_coro_evtctx_count;
extern std::atomic<intptr_t> g_coro_state_id;
#endif


namespace libcoro {
    class Scheduler;

    template<typename tp = void>
    class Future;

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

    using stop_token = std::stop_token;
    using stop_source = std::stop_source;

    template<typename Callback>
    using stop_callback = std::stop_callback<Callback>;
    using std::nostopstate;


    Scheduler* ThisScheduler();
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
    using RemoveCvRefT = typename RemoveCvref<T>::type;
}

#endif //LIBCOROUTINE_DEFINE_H
