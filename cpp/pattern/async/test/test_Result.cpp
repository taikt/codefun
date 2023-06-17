/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#include <nf/Result.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>
#include <nf/testing/TestTracers.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

using namespace nf;
using namespace nf::testing;

enum class Errors
{
    Ok,
    Failed,
};

/** The structures Enumaration and ErrorStatus are copied from
 *  CommonAPI. They have been simplified to test the CTAD in Result class.
 */
template <typename Base>
struct Enumeration
{
    Enumeration(const Base &value) // NOLINT(google-explicit-constructor)
        : value(value)
    {
    }
    Base value;
};

struct ErrorStatus : Enumeration<uint8_t>
{
    enum Literal : uint8_t
    {
        OK = 0,
        ERR = 4
    };
    ErrorStatus(ErrorStatus::Literal literal = OK) // NOLINT(google-explicit-constructor)
        : Enumeration<uint8_t>(static_cast<uint8_t>(literal))
    {
    }
};

template <>
struct nf::result::ErrImplicitConversionTraits<ErrorStatus::Literal>
{
    using type = ErrorStatus;
};

TEST(ResultConstructors, defaultConstruct_voidValue)
{
    Result<void, Errors> res{};
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_FALSE(res.hasError());
}

TEST(ResultConstructors, constructError)
{
    Result<int, Errors> res{result::Err{Errors::Failed}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(Errors::Failed, res.errorValue());
}

TEST(ResultConstructors, constructValue)
{
    Result<std::string, Errors> res{"test"};
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_EQ("test", res.value());
    ASSERT_EQ("test", *res);
}

TEST(ResultConstructors, constructValue_rvaluePerformance)
{
    Result<CopyTracer, Errors> res{CopyTracer()};
    /* Ensure the rvalue is moved at most once. */
    ASSERT_EQ(0, res->sb()->cntCopied);
    ASSERT_GE(1, res->sb()->cntMoved);
}

TEST(ResultConstructors, constructValue_lvaluePerformance)
{
    CopyTracer ct;
    Result<CopyTracer, Errors> res{ct};
    /* Ensure the lvalue is copied at most once. */
    ASSERT_GE(1, res->sb()->cntCopied);
    ASSERT_EQ(0, res->sb()->cntMoved);
}

TEST(ResultConstructors, constructValue_explicitOk_rvalue)
{
    Result<int> res = result::Ok{42};
    ASSERT_EQ(42, *res);
}

TEST(ResultConstructors, constructValue_explicitOk_lvalue)
{
    int value = 42;
    Result<int> res = result::Ok{value};
    ASSERT_EQ(42, *res);
}

TEST(ResultConstructors, constructError_complexType)
{
    Result<std::vector<int>, Errors> res{result::Err{Errors::Failed}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(Errors::Failed, res.errorValue());
}

TEST(ResultConstructors, constructValue_complexType)
{
    Result<std::vector<int>, Errors> res{{42, 43}};
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_EQ(42, res->at(0));
    ASSERT_EQ(43, res->at(1));
}

TEST(ResultConstructors, constructError_noDefaultCtor)
{
    struct NoDefaultCtor
    {
        explicit NoDefaultCtor(int)
        {
        }
    };

    Result<NoDefaultCtor, Errors> res{result::Err{Errors::Failed}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(Errors::Failed, res.errorValue());
}

TEST(ResultConstructors, constructError_voidValue)
{
    Result<void, Errors> res{result::Err{Errors::Failed}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(Errors::Failed, res.errorValue());
}

TEST(ResultConstructors, constructVoidError_voidValue)
{
    Result<void> res{result::Err{}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

TEST(ResultConstructors, constructStatic_ok)
{
    using Result1 = Result<std::unique_ptr<int>, std::string>;
    auto res = []() { return Result1::ok(std::make_unique<int>(42)); }();
    static_assert(IsResult<decltype(res)>, "Must be a result type");

    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_FALSE(res.hasError());
}

TEST(ResultConstructors, constructStatic_voidOk)
{
    using Result1 = Result<void, std::string>;
    auto res = []() { return Result1::ok(); }();
    static_assert(IsResult<decltype(res)>, "Must be a result type");

    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_FALSE(res.hasError());
}

TEST(ResultConstructors, constructStatic_voidError)
{
    using Result1 = Result<std::string>;
    auto res = []() { return Result1::err(); }();
    static_assert(IsResult<decltype(res)>, "Must be a result type");

    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

TEST(ResultConstructors, constructStatic_error)
{
    using Result1 = Result<std::string, std::unique_ptr<int>>;
    auto res = []() { return Result1::err(std::make_unique<int>(42)); }();
    static_assert(IsResult<decltype(res)>, "Must be a result type");

    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

namespace test_ns {

TEST(ResultImplicitConstructor, implicitCtor_error)
{
    auto res = []() -> Result<int, Errors> { return result::Err{Errors::Ok}; }();
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(Errors::Ok, res.errorValue());
}

TEST(ResultImplicitConstructor, implicitCtor_sameTypes_value)
{
    auto res = []() -> Result<int, int> { return 42; }();
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_FALSE(res.hasError());
    ASSERT_EQ(42, res.value());
}

TEST(ResultImplicitConstructor, implicitCtor_sameTypes_error)
{
    auto res = []() -> Result<int, int> { return result::Err{42}; }();
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(42, res.errorValue());
}

TEST(ResultImplicitConstructor, implicitCtor_voidError)
{
    auto res = []() -> Result<void, int> { return result::Err{42}; }();
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
    ASSERT_EQ(42, res.errorValue());
}

} // namespace test_ns

TEST(ResultValueHandling, assign)
{
    auto res1 = Result<int>{42};
    auto res2 = Result<int>{23};

    res1 = res2;
    ASSERT_TRUE(static_cast<bool>(res1));
    ASSERT_EQ(23, *res1);
}

TEST(ResultValueHandling, moveAssign)
{
    auto res1 = Result<std::unique_ptr<int>>{std::make_unique<int>(42)};
    auto res2 = Result<std::unique_ptr<int>>{std::make_unique<int>(23)};

    res1 = std::move(res2);
    ASSERT_TRUE(static_cast<bool>(res1));
    ASSERT_EQ(23, **res1);
}

TEST(ResultValueHandling, assignError)
{
    auto res1 = Result<void, int>{result::Err{42}};
    auto res2 = Result<void, int>{result::Err{23}};

    res1 = res2;
    ASSERT_FALSE(static_cast<bool>(res1));
    ASSERT_EQ(23, res1.errorValue());
}

TEST(ResultVoidErrorType, trivialType_error)
{
    auto res = Result<int>{result::Err{}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

TEST(ResultVoidErrorType, voidType_error)
{
    auto res = Result<void>{result::Err{}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

TEST(ResultVoidErrorType, complexType_error)
{
    auto res = Result<std::vector<std::string>>(result::Err{});
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_TRUE(res.hasError());
}

TEST(ResultInConditionals, trivialType)
{
    auto res = Result<int>{result::Err{}};
    if (res) {
        FAIL();
    }
}

TEST(ResultInConditionals, complexType)
{
    auto res = Result<std::tuple<int, int, bool>>{std::make_tuple(1, 5, false)};
    if (!res) {
        FAIL();
    }

    auto [i, j, flag] = *res;
    ASSERT_EQ(1, i);
    ASSERT_EQ(5, j);
    ASSERT_FALSE(flag);
}

TEST(ResultComparision, defaultErrorType)
{
    auto res0 = Result<int>{42};
    auto res1 = Result<int>{42};
    auto res2 = Result<int>{23};
    auto res3 = Result<int>{result::Err{}};
    auto res4 = Result<int>{result::Err{}};

    ASSERT_TRUE(res0 == res1);
    ASSERT_TRUE(res1 == res0);
    ASSERT_TRUE(res0 != res2);
    ASSERT_TRUE(res2 != res1);
    ASSERT_TRUE(res0 != res3);
    ASSERT_TRUE(res3 != res1);
    ASSERT_TRUE(res3 == res4);
}

TEST(ResultComparision, customErrorType)
{
    auto res0 = Result<int, bool>{42};
    auto res1 = Result<int, bool>{42};
    auto res2 = Result<int, bool>{23};
    auto res3 = Result<int, bool>{result::Err{true}};
    auto res4 = Result<int, bool>{result::Err{false}};
    auto res5 = Result<int, bool>{result::Err{true}};

    ASSERT_TRUE(res0 == res1);
    ASSERT_TRUE(res1 == res0);
    ASSERT_TRUE(res0 != res2);
    ASSERT_TRUE(res2 != res1);
    ASSERT_TRUE(res0 != res3);
    ASSERT_TRUE(res3 != res4);
    ASSERT_TRUE(res4 != res5);
    ASSERT_TRUE(res3 == res5);
}

TEST(ResultComparision, voidValue_defaultError)
{
    auto res0 = Result<void>{};
    auto res1 = Result<void>{};
    auto res2 = Result<void>{result::Err{}};
    auto res3 = Result<void>{result::Err{}};

    ASSERT_TRUE(res0 == res1);
    ASSERT_TRUE(res1 == res0);
    ASSERT_TRUE(res0 != res2);
    ASSERT_TRUE(res2 == res3);
    ASSERT_TRUE(res3 != res1);
}

TEST(ResultComparision, voidValue_customError)
{
    auto res0 = Result<void, bool>{};
    auto res1 = Result<void, bool>{};
    auto res2 = Result<void, bool>{result::Err{true}};
    auto res3 = Result<void, bool>{result::Err{false}};
    auto res4 = Result<void, bool>{result::Err{true}};

    ASSERT_TRUE(res0 == res1);
    ASSERT_TRUE(res1 == res0);
    ASSERT_TRUE(res0 != res2);
    ASSERT_TRUE(res2 != res1);
    ASSERT_TRUE(res0 != res3);
    ASSERT_TRUE(res2 != res3);
    ASSERT_TRUE(res3 != res4);
    ASSERT_TRUE(res2 == res4);
}

TEST(ResultSameTypes, integers)
{
    auto res = Result<int, int>{result::Err(1)};
    ASSERT_FALSE(static_cast<bool>(res));
}

TEST(ResultSameTypes, boolean)
{
    auto res = Result<bool, bool>{false};
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_FALSE(*res);
}

TEST(ResultSameTypes, functionReturn)
{
    auto func = [](int i) -> Result<int, int> { return result::Err(i); };
    auto res = func(1);
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_EQ(1, res.errorValue());
}

TEST(ResultAmbiguousTypes, error)
{
    auto res = Result<double, int>{result::Err{1}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_EQ(1, res.errorValue());
}

TEST(ResultAmbiguousTypes, value)
{
    auto res = Result<double, int>{1.0};
    ASSERT_TRUE(static_cast<bool>(res));
}

TEST(ResultAmbiguousTypes, complexType_error)
{
    constexpr auto value = "Error string";
    auto res = Result<std::string, const char *>{result::Err{value}};
    ASSERT_FALSE(static_cast<bool>(res));
    ASSERT_EQ(value, res.errorValue());
}

TEST(ResultMoveOnlyType, uniquePtr)
{
    auto func = []() -> Result<std::unique_ptr<int>> { return std::make_unique<int>(1); };

    auto res = func();
    ASSERT_TRUE(static_cast<bool>(res));
    ASSERT_EQ(1, *res.value());

    auto ptr = std::move(res).value();
    ASSERT_EQ(1, *ptr);

    // Deliberately using for test
    // NOLINTNEXTLINE(bugprone-use-after-move, hicpp-invalid-access-moved)
    ASSERT_EQ(nullptr, res.value());
}

TEST(ResultValueOr, valueDefaultError)
{
    {
        auto res = Result<int>{10};
        auto v = res.valueOr(42);
        ASSERT_EQ(10, v);
    }
    {
        auto res = Result<int>{result::Err{}};
        auto v = res.valueOr(42);
        ASSERT_EQ(42, v);
    }
    {
        auto res = Result<std::unique_ptr<int>>{std::make_unique<int>(10)};
        auto v = std::move(res).valueOr(std::make_unique<int>(42));
        ASSERT_EQ(10, *v);
    }
}

TEST(ResultMapValue, mapValue_hasValue)
{
    auto res = Result<int, int>{10};
    auto mapped = res.map([](const int &value) { return std::to_string(value); });
    ASSERT_FALSE(mapped.hasError());
    ASSERT_EQ("10", mapped.value());
}

TEST(ResultMapValue, mapValue_hasError)
{
    auto res = Result<int, int>{result::Err{10}};
    auto mapped = res.map([](const int &) -> std::string { throw std::bad_function_call(); });
    ASSERT_TRUE(mapped.hasError());
    ASSERT_EQ(10, mapped.errorValue());
}

TEST(ResultMapValue, mapValue_toVoid)
{
    auto res = Result<int, int>{10};
    int sideEffect = 0;
    auto mapped = res.map([&](int i) { sideEffect = i; });
    ASSERT_FALSE(mapped.hasError());
    ASSERT_EQ(10, sideEffect);
}

TEST(ResultMapValue, mapVoid_toValue)
{
    auto res = Result<void, int>{};
    auto mapped = res.map([]() { return 42; });
    ASSERT_FALSE(mapped.hasError());
    ASSERT_EQ(42, mapped.value());
}

TEST(ResultMapValue, mapVoid_toError)
{
    auto res = Result<void, int>{result::Err{42}};
    auto mapped = res.map([]() -> std::string { throw std::bad_function_call(); });
    ASSERT_TRUE(mapped.hasError());
}

TEST(ResultIoStream, int_int)
{
    EXPECT_EQ("Ok{42}", streamToString(Result<int, int>{42}));
    EXPECT_EQ("Err{42}", streamToString(Result<int, int>{result::Err{42}}));
}

TEST(ResultIoStream, int_void)
{
    EXPECT_EQ("Ok{42}", streamToString(Result<int>{42}));
    EXPECT_EQ("Err{}", streamToString(Result<int>{result::Err{}}));
}

TEST(ResultIoStream, void_int)
{
    EXPECT_EQ("Ok{}", streamToString(Result<void, int>{}));
    EXPECT_EQ("Err{42}", streamToString(Result<void, int>{result::Err{42}}));
}

TEST(ResultIoStream, void_void)
{
    EXPECT_EQ("Ok{}", streamToString(Result<void, void>{}));
    EXPECT_EQ("Err{}", streamToString(Result<void, void>{result::Err{}}));
}


/**
 * @brief Test explicit deduction trait. The example is taken from the CommonAPI.
 */
TEST(ErrImplicitConversionTraits, test)
{
    /*
     * Implictly, with CTAD and template specialization define the internal type of Result to
     * ErrorStatus, not an ErrorStatus::Literal.
     */
    auto implicitly = nf::result::Err{ErrorStatus::ERR};
    /*
     * Explicitly define the internal type of Result to ErrorStatus
     */
    auto explicitly = nf::result::Err<ErrorStatus>{ErrorStatus::Literal::ERR};

    EXPECT_EQ(typeid(implicitly), typeid(explicitly));         // Compare the types
    EXPECT_EQ(implicitly.error.value, explicitly.error.value); // Compare values
}

TEST(ResultClangTidy, clang_tidy_warnings_with_no_except)
{
    {
        // const &
        [[maybe_unused]] auto lambda = []() noexcept {
            const Result<int> res = Result<int>{42};
            if (!res) {
                return;
            }
            if (*res == 10) {
                return;
            }
        };
        lambda();
    }

    {
        // &
        [[maybe_unused]] auto lambda = []() noexcept {
            auto res = Result<int>{42};
            if (*res == 10) {
                return;
            }
        };
        lambda();
    }

    {
        // rvalue &&
        [[maybe_unused]] auto lambda = []() noexcept { return *Result<int>{42}; };
        lambda();
    }
}
