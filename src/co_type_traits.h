#ifndef LIBCOROUTINE_TYPE_TRAITS_H
#define LIBCOROUTINE_TYPE_TRAITS_H
#include <type_traits>
#include <define.h>
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
                        typename Tp::PromiseType>
    > : std::true_type {};
    template<class Tp>
    constexpr bool IsFutureV = IsFuture<RemoveCvref<Tp>>::value;

    template<class Tp>
    struct IsGenerator : std::false_type {};
    template <class Tp, class Alloc>
    struct IsGenerator<Generator<Tp, Alloc>> : std::true_type {};
    template<class Tp>
    constexpr bool IsGeneratorV = IsGenerator<RemoveCvrefT<Tp>>::value;

    template<class Tp>
    struct IsPromise : std::false_type {};
    template<class Tp>
    struct IsPromise<Promise<Tp>> : std::true_type {};
    template<class Tp>
    constexpr bool IsPromiseV = IsPromise<RemoveCvref<Tp>>::value;
}

#endif //LIBCOROUTINE_TYPE_TRAITS_H
