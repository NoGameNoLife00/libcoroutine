#ifndef LIBCOROUTINE_AWAITABLE_H
#define LIBCOROUTINE_AWAITABLE_H
#include <libcoro.h>
namespace libcoro {
    template<typename Tp>
    class AwaitableImpl {
    public:
        using ValueType = Tp;
        using StateType = State<ValueType>;
        using FutureType = Future<ValueType>;
        using LockType = typename StateType::LockType;
        using AllocChar = typename StateType::AllocChar;

        AwaitableImpl() = default;
        AwaitableImpl(const AwaitableImpl&) = default;
        AwaitableImpl(AwaitableImpl&&) = default;

        AwaitableImpl& operator=(const AwaitableImpl&) = default;
        AwaitableImpl& operator=(AwaitableImpl&&) = default;

        void SetException(std::exception_ptr e) const {
            use_ptr<StateType> up(std::move(this->state_));
            up->SetException(std::move(e));
        }

        template<typename Exp>
        inline void ThrowException(Exp e) {
            SetException(std::make_exception_ptr(std::move(e)));
        }

        FutureType GetFuture() const {
            return FutureType {this->state_};
        }
        explicit operator bool () const {
            return state_.get() != nullptr;
        }
        mutable use_ptr<StateType> state_ = StateFuture::AllocState<StateType>(true);
    };

    template<typename Tp>
    class Awaitable : public AwaitableImpl<Tp> {
        using typename AwaitableImpl<Tp>::ValueType;
        using typename AwaitableImpl<Tp>::StateType;
        using AwaitableImpl<Tp>::AwaitableImpl;

        template<class U>
        void SetValue(U&& val) const {
            use_ptr<StateType> up(std::move(val));
            up->SetValue(std::forward<U>(val));
        }
    };

    template<typename Tp>
    class Awaitable<Tp&> : public AwaitableImpl<Tp&> {
    public:
        using typename AwaitableImpl<Tp&>::ValueType;
        using typename AwaitableImpl<Tp&>::StateType;
        using AwaitableImpl<Tp&>::AwaitableImpl;

        void SetValue(Tp& val) const {
            use_ptr<StateType> up(std::move(this->state_));
            up->SetValue(val);
        }
    };

    template<>
    class Awaitable<void> : public AwaitableImpl<void> {
    public:
        using typename AwaitableImpl<void>::ValueType;
        using AwaitableImpl<void>::AwaitableImpl;

        void SetValue() const {
            use_ptr<StateType> up(std::move(this->state_));
            up->SetValue();
        }
    };
}
#endif //LIBCOROUTINE_AWAITABLE_H
