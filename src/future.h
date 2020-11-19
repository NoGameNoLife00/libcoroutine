#ifndef LIBCOROUTINE_FUTURE_H
#define LIBCOROUTINE_FUTURE_H
#include <define.h>
#include <state.h>
#include <use_ptr.h>
namespace libcoro {
    template<typename Tp>
    class Future {
    public:
        using ValueType = Tp;
        using PromiseType = Promise<ValueType>;
        using StateType = State<ValueType>;
        using LockType = typename StateType::LcokType;

        Future(use_ptr<StateType> st) : state_(std::move(st)) {}
        Future(const Future&) = default;
        Future(Future&&) = default;

        Future& operator=(const Future&) = default;
        Future& operator=(Future&&) =default;

        bool await_ready() const {
            return state_->FutureAwaitReady();
        }

        template<class PromiseT>
        void await_suspend(coroutine_handle<PromiseT> handler) {
            state_->FutureAwaitSuspend(handler);
        }

        ValueType await_resume() const {
            return state_->FutureAwaitResume();
        }
    private:
        use_ptr<StateType> state_;

    };
}

#endif //LIBCOROUTINE_FUTURE_H
