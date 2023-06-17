/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/FunctionTraits.h>
#include <nf/testing/Test.h>

#include <functional>

#define IMPL_CHECK(__suffix, __arg)                                                                \
    template <typename tExpectedReturnType, typename... tExpectedArgs, typename tFunc>             \
    void check##__suffix(__arg)                                                                    \
    {                                                                                              \
        using Traits = nf::FunctionTraits<tFunc>;                                                  \
        static_assert(std::is_same<tExpectedReturnType, typename Traits::ReturnType>(),            \
                      "Invalid return value");                                                     \
        static_assert(Traits::NrArgs == sizeof...(tExpectedArgs), "Invalid number of arguments");  \
        static_assert(std::is_same<std::tuple<tExpectedArgs...>, typename Traits::ArgTypes>(),     \
                      "Invalid list of arguments");                                                \
    }

#define RUN_CHECK(__suffix, __init, __func, ...)                                                   \
    do {                                                                                           \
        __init;                                                                                    \
        check##__suffix<__VA_ARGS__>(__func);                                                      \
    } while (!42)

IMPL_CHECK(ByValue, tFunc)
IMPL_CHECK(ConstRef, const tFunc &)
IMPL_CHECK(Ref, tFunc &)
IMPL_CHECK(FwdRef, tFunc &&)

#define CHECK_TRAITS_LVALUE(__init, __func, ...)                                                   \
    do {                                                                                           \
        RUN_CHECK(ByValue, __init, __func, __VA_ARGS__);                                           \
        RUN_CHECK(ConstRef, __init, __func, __VA_ARGS__);                                          \
        RUN_CHECK(Ref, __init, __func, __VA_ARGS__);                                               \
        RUN_CHECK(FwdRef, __init, __func, __VA_ARGS__);                                            \
        RUN_CHECK(FwdRef, __init, std::move(__func), __VA_ARGS__);                                 \
    } while (!42)

#define CHECK_TRAITS_RVALUE(__init, __func, ...)                                                   \
    do {                                                                                           \
        RUN_CHECK(ByValue, __init, __func, __VA_ARGS__);                                           \
        RUN_CHECK(ConstRef, __init, __func, __VA_ARGS__);                                          \
        RUN_CHECK(FwdRef, __init, __func, __VA_ARGS__);                                            \
        RUN_CHECK(FwdRef, __init, std::move(__func), __VA_ARGS__);                                 \
    } while (!42)

TEST(FunctionTraitsTest, lambda)
{
    CHECK_TRAITS_LVALUE(
        auto lambda = [](int, double) {}, lambda, void, int, double);
}

int myFunc(char)
{
    return 5;
}

TEST(FunctionTraitsTest, freeFunction)
{
    CHECK_TRAITS_LVALUE(, myFunc, int, char);
    CHECK_TRAITS_RVALUE(, &myFunc, int, char);
}

TEST(FunctionTraitsTest, stdFunction)
{
    CHECK_TRAITS_LVALUE(std::function<double()> stdFunc, stdFunc, double);
}

class MyClass
{
public:
    int func(int)
    {
        return 6;
    }
    void constFunc(double, double) const
    {
    }
};

TEST(FunctionTraitsTest, classMethod)
{
    CHECK_TRAITS_RVALUE(, &MyClass::func, int, int);
    CHECK_TRAITS_RVALUE(, &MyClass::constFunc, void, double, double);
}

TEST(FunctionTraitsTest, getTypeByIndex)
{
    using FT = nf::FunctionTraits<std::function<void(const int &, double)>>;
    static_assert(std::is_same<FT::ArgType<0>, int>::value, "First argument does not match");
    static_assert(std::is_same<FT::ArgType<1>, double>::value, "Second argument does not match");
}

TEST(FunctionTraitsTest, signatures)
{
    auto lambda = [](const int &, double) {};
    using FT = nf::FunctionTraits<decltype(lambda)>;
    static_assert(std::is_same<FT::Signature, void(const int &, double)>::value,
                  "Invalid signature");
}
