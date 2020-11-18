#ifndef LIBCOROUTINE_PROMISE_H
#define LIBCOROUTINE_PROMISE_H

#include <type_traits>
#include <co_type_traits.h>
namespace libcoro {
    struct suspend_on_initial {
        inline bool await_ready() {
            return false;
        }

        template<class PromiseT, typename = std::enable_if_t<traits::IsPromiseV<PromiseT>>>
        inline void await_suspend(coroutine_handle<PromiseT> handler) {
            PromiseT& promise = handler.promise();
            auto* state = promise.GetState();
            state->PromiseInitialSuspend(handler);
        }
        inline void await_resume() {
        }
    };

    struct suspend_on_final {
        inline bool await_ready() {
            return false;
        }
        template<class PromiseT, typename = std::enable_if_t<traits::IsPromiseV<PromiseT>>>
        inline void await_suspend(coroutine_handle<PromiseT> handler) {
            PromiseT& promise = handler.promise();
            auto* state = promise.RefState();
            state->PromiseFinalSuspend(handler);
        }
        inline void await_resume() {
        }
    };

    template <typename Tp>
    struct PromiseImpl {
        using ValueType = Tp;
        using StateType = State<ValueType>;
    };
}

#endif //LIBCOROUTINE_PROMISE_H
