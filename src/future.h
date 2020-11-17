#ifndef LIBCOROUTINE_FUTURE_H
#define LIBCOROUTINE_FUTURE_H

namespace libcoro {
    template<typename _Type>
    class Future {
    public:
        using PromiseType = Promise<_Type>;
        using ValueType = _Type;
        using StateType = State<_Type>;
        using LockType = typename StateType::LcokType;

        Future(StatePtr<StateType> st) : state_(std::move(st)) {}
        Future(const Future&) = default;
        Future(Future&&) = default;

        Future& operator=(const Future&) = default;
        Future& operator=(Future&&) =default;

        bool AwaitReady() const {
            return ;
        }
    private:
        StatePtr<StateType> state_;

    };
}

#endif //LIBCOROUTINE_FUTURE_H
