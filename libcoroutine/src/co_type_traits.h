#ifndef LIBCOROUTINE_TYPE_TRAITS_H
#define LIBCOROUTINE_TYPE_TRAITS_H

#include <libcoro.h>
namespace libcoro::traits {
    template<typename Tp, class = std::void_t<>>
    struct IsCallable : std::false_type{};
    template<typename Tp>
    struct IsCallable<Tp, std::void_t<decltype(std::declval<Tp>()())>> : std::true_type {};
    template<typename Tp>
    constexpr bool IsCallableV = IsCallable<Tp>::value;

    template<class Tp, class = std::void_t<>>
    struct IsFuture : std::false_type {};
    template<class Tp>
    struct IsFuture<Tp,
            std::void_t<decltype(std::declval<Tp>().state_),
                        typename Tp::ValueType,
                        typename Tp::StateType,
                        typename Tp::promise_type>
    > : std::true_type {};
    template<class Tp>
    constexpr bool IsFutureV = IsFuture<RemoveCvRefT<Tp>>::value;

    template<class Tp>
    struct IsGenerator : std::false_type {};
    template <class Tp, class Alloc>
    struct IsGenerator<Generator<Tp, Alloc>> : std::true_type {};
    template<class Tp>
    constexpr bool IsGeneratorV = IsGenerator < RemoveCvRefT < Tp >> ::value;

    template<class Tp>
    struct IsPromise : std::false_type {};
    template<class Tp>
    struct IsPromise<Promise<Tp>> : std::true_type {};
    template<class Tp>
    constexpr bool IsPromiseV = IsPromise<RemoveCvRefT < Tp>>::value;

    template <class Tp, typename = std::void_t<>>
    struct HasState : std::false_type {};
    template <class Tp>
    struct HasState<Tp, std::void_t<decltype(std::declval<Tp>().state_)>> : std::true_type {};
    template <class Tp>
    constexpr bool HasStateV = HasState<RemoveCvRefT < Tp>>::value;
}

#endif //LIBCOROUTINE_TYPE_TRAITS_H
