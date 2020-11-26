#ifndef LIBCOROUTINE_PROMISE_H
#define LIBCOROUTINE_PROMISE_H

#include <libcoro.h>

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
    class PromiseImpl {
    public:
        using ValueType = Tp;
        using StateType = State<ValueType>;
        using promise_type = Promise<ValueType>;
        using FutureType = Future<ValueType>;

        PromiseImpl() = default;
        PromiseImpl(PromiseImpl&&)  noexcept = default;
        PromiseImpl& operator=(PromiseImpl&&) = default;
        PromiseImpl(const  PromiseImpl&) = delete;
        PromiseImpl operator=(const PromiseImpl&) = delete;

        auto GetState() -> StateType*;
        auto RefState() -> StateType*;

        FutureType get_return_object() {
            return {this->GetState()};
        }

        suspend_on_initial initial_suspend() {
            return {};
        }

        suspend_on_final final_suspend() {
            return {};
        }

        template<typename Uty>
        Uty&& await_transform(Uty&& whatever) {
            if constexpr (traits::HasStateV<Uty>) {
                whatever.state_->SetScheduler(GetState()->GetScheduler());
            }
            return std::forward<Uty>(whatever);
        }
        void unhandled_exception() {
            this->RefState()->SetException(std::current_exception());
        }

        void cancellation_request() noexcept {

        }

    private:
#ifndef LIBCORO_INLINE_STATE
        use_ptr<StateType> state_ = StateFuture::AllocState<StateType>(false);
#endif
    };


    template<typename Tp>
    class Promise final : public PromiseImpl<Tp> {
    public:
        using typename PromiseImpl<Tp>::ValueType;
        using PromiseImpl<Tp>::get_return_object;

        template<typename U>
        void return_value(U&& val) {
            return this->RefState()->SetValue(std::forward<U>(val));
        }

        template<typename U>
        suspend_always yield_value(U&& val) {
            this->RefState()->PromiseYieldValue(this, std::forward<U>(val));
            return {};
        }
    };

    template<typename Tp>
    class Promise<Tp&> final : public PromiseImpl<Tp&> {
    public:
        using typename PromiseImpl<Tp&>::ValueType;
        using PromiseImpl<Tp&>::get_return_object;
        void return_value(Tp& val) {
            this->RefState()->SetValue(val);
        }

        suspend_always yield_value(Tp& val) {
            this->RefState()->PromiseYieldValue(this, val);
            return {};
        }
    };

    template<>
    class Promise<void> final : public PromiseImpl<void> {
    public:
        using PromiseImpl<void>::get_return_object;

        void return_value() {
            RefState()->SetValue();
        }
        suspend_always yield_value() {
            RefState()->PromiseYieldValue(this);
            return {};
        }
    };

    template<typename Tp>
    auto PromiseImpl<Tp>::GetState() -> StateType* {
#ifdef LIBCORO_INLINE_STATE
        size_t state_size = AlignSize<StateType>();
        auto h = coroutine_handle<promise_type>::from_promise(*reinterpret_cast<promise_type*>(this));
        char* ptr = reinterpret_cast<char*>(h.address()) - state_size;
        return reinterpret_cast<StateType*>(ptr);
#else
        return state_.get();
#endif
    }

    template<typename Tp>
    auto PromiseImpl<Tp>::RefState() -> PromiseImpl::StateType * {
        return GetState();
    }
}

#endif //LIBCOROUTINE_PROMISE_H
