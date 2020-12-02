#ifndef LIBCOROUTINE_YIELD_H
#define LIBCOROUTINE_YIELD_H
#include <libcoro.h>
namespace libcoro {
    class YieldAwaiter {
    public:
        using ValueType = void;
        using StateType = State<ValueType>;
        using promise_type = Promise<ValueType>;
        using LockType = typename StateType::LockType;

        bool await_ready() const {
            return false;
        }

        template<PromiseT PromiseTp>
        bool await_suspend(coroutine_handle<PromiseTp> handler)
        {
            use_ptr<State<void>> state = StateFuture::AllocState<StateType>(true);
            state->SetValue();
            state->FutureAwaitSuspend(handler);
            return true;
        }
        void await_resume() const {
        }

    };

    inline YieldAwaiter Yield() {
        return {};
    }
}
#endif //LIBCOROUTINE_YIELD_H
