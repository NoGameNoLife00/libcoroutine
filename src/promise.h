#ifndef LIBCOROUTINE_PROMISE_H
#define LIBCOROUTINE_PROMISE_H

#include <type_traits>
#include <co_type_traits.h>
#include <state.h>
#include <future.h>
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
        using PromiseType = Promise<ValueType>;
        using FutureType = Future<ValueType>;

        PromiseImpl() = default;
        PromiseImpl(PromiseImpl&&)  noexcept = default;
        PromiseImpl& operator=(PromiseImpl&&) = default;
        PromiseImpl(const  PromiseImpl&) = delete;
        PromiseImpl operator=(const PromiseImpl&) = delete;

        auto PromiseImpl<Tp>::GetState() -> StateType*
        StateType* RefState();

        FutureType GetReturnObject() {
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
                whatever.state_->SetScheduler(GetState().GetScheduler());
            }
        }

    };

    template<typename Tp>
    auto PromiseImpl<Tp>::GetState() -> StateType* {
        return GetState();
    }
}

#endif //LIBCOROUTINE_PROMISE_H
