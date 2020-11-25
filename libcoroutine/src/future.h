#ifndef LIBCOROUTINE_FUTURE_H
#define LIBCOROUTINE_FUTURE_H
#include <libcoro.h>
namespace libcoro {
    template<typename Tp>
    class Future {
    public:
        using promise_type = Promise<Tp>;
        using ValueType = Tp;
        using StateType = State<ValueType>;
        using FutureType = Future<ValueType>;
        using LockType = typename StateType::LockType;

        Future(use_ptr<StateType> st) : state_(std::move(st)) {}
        Future(const Future&) = default;
        Future(Future&&) = default;

        Future& operator=(const Future&) = default;
        Future& operator=(Future&&) =default;

        bool await_ready() const noexcept {
            return state_->FutureAwaitReady();
        }

        template<class PromiseT>
        void await_suspend(coroutine_handle<PromiseT> handler) {
            state_->FutureAwaitSuspend(handler);
        }

        Tp await_resume() const {
            return state_->FutureAwaitResume();
        }

        use_ptr<StateType> state_;
    };
}

#endif //LIBCOROUTINE_FUTURE_H
