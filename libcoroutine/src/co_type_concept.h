#ifndef LIBCOROUTINE_CO_TYPE_CONCEPT_H
#define LIBCOROUTINE_CO_TYPE_CONCEPT_H
#include <libcoro.h>

namespace libcoro {

    template<typename T>
    concept HasStateT = requires(T&& v) {
        {v.state_};
        requires traits::IsStatePointerV<decltype(v.state_)>;
    };

    template<typename T>
    concept AwaiterT = requires(T&& v) {
        { v.await_ready() } -> std::same_as<bool>;
        { v.await_suspend(std::declval<std::coroutine_handle<Promise<>>>())};
        { v.await_resume()};
        requires traits::IsValidAwaitSuspendReturnV<
            decltype(v.await_suspend(std::declval<std::coroutine_handle<Promise<>>>()))
            >;
    };

    template <typename T>
    concept FutureT = AwaiterT<T> && HasStateT<T> && requires {
        typename T::ValueType;
        typename T::StateType;
        typename T::promise_type;
    };

    template<typename T>
    concept CallableT = std::invocable<T>;

    template<typename T>
    concept GeneratorT = traits::IsGeneratorV<T>;

    template<typename T>
    concept PromiseT = traits::IsPromiseV<T>;
}

#endif //LIBCOROUTINE_CO_TYPE_CONCEPT_H
