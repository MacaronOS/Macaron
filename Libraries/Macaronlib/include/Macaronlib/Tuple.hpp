#pragma once

#include "Common.hpp"
#include "Runtime.hpp"

template <size_t I, typename T>
struct _TupleTypeHolder {
    T val;
};

template <size_t I, typename... T>
class _RecursiveTuple {
};

template <size_t I, typename BeginT, typename... RestT>
class _RecursiveTuple<I, BeginT, RestT...> : public _TupleTypeHolder<I, RemoveReference<BeginT>>,
                                             public _RecursiveTuple<I + 1, RestT...> {
public:
    template <typename BeginArgs, typename... RestArgs>
    _RecursiveTuple(BeginArgs&& begin_args, RestArgs&&... rest_args)
        : _TupleTypeHolder<I, RemoveReference<BeginT>>(forward<BeginArgs>(begin_args))
        , _RecursiveTuple<I + 1, RestT...>(forward<RestArgs>(rest_args)...)
    {
    }
};

template <size_t I, typename BeginT, typename... RestT>
struct __RecursiveExtractType {
    using Type = typename __RecursiveExtractType<I - 1, RestT...>::Type;
};

template <typename BeginT, typename... RestT>
struct __RecursiveExtractType<0, BeginT, RestT...> {
    using Type = BeginT;
};

template <size_t I, typename BeginT, typename... RestT>
using _RecursiveExtractType = typename __RecursiveExtractType<I, BeginT, RestT...>::Type;

template <typename BeginT, typename... RestT>
class Tuple : public _RecursiveTuple<0, BeginT, RestT...> {
public:
    template <typename... Args>
    Tuple(Args&&... args)
        : _RecursiveTuple<0, BeginT, RestT...>(forward<Args>(args)...)
    {
    }

    template <size_t I>
    auto& get()
    {
        return static_cast<_TupleTypeHolder<I, _RecursiveExtractType<I, BeginT, RestT...>>*>(this)->val;
    }

    constexpr size_t size()
    {
        return 1 + sizeof...(RestT);
    }
};

// Template deduction guideline.
template <typename... Args>
Tuple(Args... args) -> Tuple<Args...>;