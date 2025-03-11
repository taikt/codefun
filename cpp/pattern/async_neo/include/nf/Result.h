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

#pragma once

#include "Printable.h"

#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

NF_BEGIN_NAMESPACE

namespace detail {
struct Unit
{
    constexpr bool operator==(const Unit &) const noexcept
    {
        return true;
    }
};
} // namespace detail

namespace result {

template <typename T, bool IsRValue = false>
struct Ok
{
    constexpr explicit Ok(const T &v)
        : value{v}
    {
    }

    const T &value;
};

template <typename T>
struct Ok<T, true>
{
    constexpr explicit Ok(T &&value)
        : value(std::move(value))
    {
    }

    T &&value;
};

template <>
struct Ok<void>
{
    constexpr Ok() = default;
    detail::Unit value;
};

template <typename T>
Ok(T &&) -> Ok<T, true>;
template <typename T>
Ok(T &) -> Ok<T, false>;

Ok()->Ok<void>;

template <typename T>
struct Err
{
    constexpr explicit Err(T e)
        : error{std::move(e)}
    {
    }

    T error;
};

template <>
struct Err<void>
{
    constexpr Err() = default;
    detail::Unit error;
};

Err()->Err<void>;

/**
 * @ingroup nf_core_Types
 * @brief Enables template deduction for @p tFrom type for @c nf::result::Err object.
 * @tparam tFrom Type of the value
 *
 * Enables conversion from @p tFrom to a type that is provided
 * in the specialization of the template. This trait simplifies the usage
 * of @c nf::result::Err class with the types that lack explicit conversion constructors.
 *
 * The simplest example would be a CommonAPI type where internal <code>enum Literal</code>
 * defines an information regarding an @c ErrorStatus. Having unreadable code:
 *
 * @code
 * nf::result::Err{ipc::IOLifecycleTypes::ErrorStatus{{ipc::IOLifecycleTypes::ErrorStatus::NOT_IMPLEMENTED}}}
 * @endcode
 *
 * we can simplify it with specialization of the @ref ErrImplicitConversionTraits
 *
 * @code
 * template <>
 * struct nf::result::ErrImplicitConversionTraits<IOLifecycleTypes::ErrorStatus::Literal>
 * {
 *     using type = IOLifecycleTypes::ErrorStatus;
 * };
 * @endcode
 *
 * with the result of
 *
 * @code
 * nf::result::Err{IOLifecycleTypes::ErrorStatus::NOT_IMPLEMENTED}
 * @endcode
 */
template <typename tFrom>
struct ErrImplicitConversionTraits;

template <typename tFrom>
using ErrImplicitConversionTraits_t = typename ErrImplicitConversionTraits<tFrom>::type;

template <typename tFrom>
Err(tFrom) -> Err<ErrImplicitConversionTraits_t<tFrom>>;

} // namespace result

// forward declare Result
template <typename tValue, typename tError>
class Result;

namespace detail {

/**
 * @brief Actual implementation of result class
 *
 * @tparam tValue Type of the value
 * @tparam tError Type of an error
 */
template <typename tValue, typename tError>
class ResultImpl
{
    using StorageValueType = std::conditional_t<std::is_void_v<tValue>, detail::Unit, tValue>;
    using StorageErrorType = std::conditional_t<std::is_void_v<tError>, detail::Unit, tError>;

public:
    using ValueType = tValue;
    using ErrorType = tError;

public:
#ifdef DOXYGEN
    /**
     * @brief Construct an object of this @c Result<tValue, tError> type containing the passed
     * value
     */
    Result<tValue, tError> ok(tValue value) noexcept;

    /**
     * @brief Construct an object of this @c Result<tValue, tError> type containing the error value
     */
    Result<tValue, tError> err(tError error) noexcept;
#else
    template <typename T, std::enable_if_t<!std::is_void_v<T>, void *> = nullptr>
    static Result<tValue, tError> ok(T &&value)
    {
        static_assert(std::is_constructible_v<tValue, T>,
                      "Cannot construct value from the passed type");
        return result::Ok{std::forward<tValue>(value)};
    }

    template <typename T = tValue, std::enable_if_t<std::is_void_v<T>, void *> = nullptr>
    static Result<tValue, tError> ok()
    {
        static_assert(std::is_same_v<T, tValue>);
        return result::Ok{};
    }

    template <typename T, std::enable_if_t<!std::is_void_v<T>, void *> = nullptr>
    static Result<tValue, tError> err(T &&value)
    {
        static_assert(std::is_constructible_v<tError, T>,
                      "Cannot construct error value from the passed type");
        return result::Err{std::forward<T>(value)};
    }

    template <typename T = tError, std::enable_if_t<std::is_void_v<T>, void *> = nullptr>
    static Result<tValue, tError> err()
    {
        static_assert(std::is_same_v<T, tError>);
        return result::Err{};
    }
#endif


public:
    // Implicit conversion is desired for this class and is unambiguous as ctors of Err and Ok
    // are explicit
    template <bool IsRValue> // NOLINTNEXTLINE(google-explicit-constructor)
    ResultImpl(result::Ok<tValue, IsRValue> v)
        : m_value{std::move(v.value)}
    {
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    ResultImpl(result::Err<tError> e)
        : m_error{std::move(e.error)}
    {
    }

    /**
     * Check if result has a value set
     *
     * @return true if result is a value, false otherwise
     */
    bool hasValue() const noexcept
    {
        return m_value.has_value();
    }

    /**
     * Check if result has an error set
     *
     * @return true if result is an error, false otherwise
     */
    bool hasError() const noexcept
    {
        return m_error.has_value();
    }

    /**
     * Checks whether the Result indicates success
     *
     * @return true if no error occurred, false otherwise
     */
    explicit operator bool() const noexcept
    {
        return !hasError();
    }

protected:
    /// Get error value
    const StorageErrorType &errorValue() const
    {
        return m_error.value();
    }

    /// Get value (rvalue)
    StorageValueType &&value() &&
    {
        return std::move(m_value.value());
    }

    /// Get value (lvalue)
    const StorageValueType &value() const &
    {
        return m_value.value();
    }

    constexpr const StorageValueType &operator*() const noexcept
    {
        return *m_value;
    }

    /// Get value if it is set, otherwise return default value (rvalue)
    StorageValueType valueOr(StorageValueType &&defaultValue) &&
    {
        return std::move(m_value).value_or(std::move(defaultValue));
    }

    /// Get value if it is set, otherwise return default value (lvalue)
    StorageValueType valueOr(StorageValueType &&defaultValue) const &
    {
        return m_value.value_or(std::move(defaultValue));
    }

    /// Compare two results
    bool operator==(const Result<tValue, tError> &other) const noexcept
    {
        return equalsTo(other);
    }

    /// Compare two results
    bool operator!=(const Result<tValue, tError> &other) const noexcept
    {
        return !(*this == other);
    }

    bool equalsTo(const ResultImpl<tValue, tError> &other) const noexcept
    {
        if (m_value.has_value() && other.m_value.has_value()) {
            return *m_value == *other.m_value;
        }
        if (m_error.has_value() && other.m_error.has_value()) {
            return *m_error == *other.m_error;
        }

        return false;
    }

    /**
     * Maps Result<T, Error> to Result<U, Error> by applying function to the
     * contained value, leaving the Error untouched.
     *
     * @return Result<U, Error>, where T is transformed to U via function F
     */
    template <typename F>
    auto map(F f) const
    {
        // https://reviews.llvm.org/D71980
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (std::is_void_v<tValue>) { // NOLINT(readability-braces-around-statements)
            static_assert(std::is_invocable_v<F>,
                          "Passed function cannot map tValue of this Result");

            using TReturn = std::invoke_result_t<F>;

            if (hasError()) {
                return Result<TReturn, tError>{result::Err{errorValue()}};
            }

            // NOLINTNEXTLINE(readability-braces-around-statements)
            if constexpr (std::is_void_v<TReturn>) {
                f();
                return Result<TReturn, tError>{result::Ok{}};
            } else { // NOLINT(readability-misleading-indentation)
                return Result<TReturn, tError>{result::Ok{f()}};
            }
        } else { // NOLINT(readability-misleading-indentation)
            static_assert(std::is_invocable_v<F, tValue>,
                          "Passed function cannot map tValue of this Result");

            using TReturn = std::invoke_result_t<F, tValue>;

            if (hasError()) {
                return Result<TReturn, tError>{result::Err{errorValue()}};
            }

            // NOLINTNEXTLINE(readability-braces-around-statements)
            if constexpr (std::is_void_v<TReturn>) {
                f(value());
                return Result<TReturn, tError>{result::Ok{}};
            } else { // NOLINT(readability-misleading-indentation)
                return Result<TReturn, tError>{result::Ok{f(value())}};
            }
        }
    }

private:
    std::optional<StorageValueType> m_value; ///< value holder
    std::optional<StorageErrorType> m_error; ///< error holder
};

} // namespace detail

/**
 * @ingroup nf_core_Types
 * @brief Type that holds a result of an operation.
 * @tparam tValue Type of the value
 * @tparam tError Type of an error (default: @c void)
 *
 * This type holds a result of an operation, e.g. a result of a function call. It can either
 * be the resulting value if the operation was successful, or an error value in case the
 * operation did not succeed. In any case only one value is set. It is mandatory to specify
 * the value type. A custom error type can be specified when additional information needs
 * to be returned in the error case.
 *
 * It is recommended to use @c Result for any function or method where there is an error
 * result possible, even when other alternatives like @c std::optional can be used, for
 * consistency, added semantics, cleaner API and improved readability.
 *
 * <table>
 * <caption>What to return?</caption>
 * <tr><th style="width:22%">Return<th style="width:23%">Instead of<th>Because
 * <tr><td><code>nf::Result<void></code>
 *     <td><code>bool</code>
 *     <td>Boolean @e false does not necessary mean an error. Using @c Result, on the other
 *         hand, clearly shows an intention that the function might fail.
 * <tr><td><code>nf::Result<int></code>
 *     <td><code>std::optional<int></code>
 *     <td>Unlike @c std::optional, @c Result adds explicit error semantics and thus improves
 *         readability.
 * <tr><td><code>nf::Result<int, Error></code>
 *     <td><code>std::variant<int, Error></code>\n\n
 *         <code>std::pair<int, Error></code>
 *     <td>In addition to a clearer semantics, @c Result easily handles it when a value and
 *         an error both have the same type. Although @c std::variant supports that as well,
 *         one would need to always specify an index with @c std::in_place_index which would
 *         make it less readable. In addition, unlike @c std::pair, @c Result does not require
 *         to initialize both the value and the error at the same time.
 * </table>
 *
 * This example shows a basic use-case for the @c Result class:
 *
 * @code
 * // Function returns an integer or an error of type Error
 * nf::Result<int, Error> doSomething(int i) noexcept
 * {
 *     if (!isGoodValue(i)) {
 *         // Errors are returned with nf::result::Err{}
 *         return nf::result::Err{Error::BadValue};
 *     }
 *
 *     int returnValue = compute(i);
 *
 *     // A successful result is implicitly constructed from a return value, or one can
 *     // explicitly use nf::result::Ok{}
 *     return returnValue; // or return nf::result::Ok{returnValue};
 * }
 * @endcode
 * ...
 * @code
 * auto result = doSomething(10);
 * if (result) { // or result.hasValue()
 *     handle(*result); // or result.value()
 * } else {
 *     switch (result.errorValue()) {
 *         ...
 *     }
 * }
 * @endcode
 *
 * To return a @c Result class from a lambda without explicitly specifying lambda return type static
 * constructors can be used. For example:
 * @code
 *  using ImageResult = Result<Image, std::string>;
 *  auto banner = downloadImage(42);
 *  banner.then([](const Image& image) {
 *      if (!image.isAcceptable()) {
 *          return ImageResult::err("Censored");
 *      }
 *      return ImageResult::ok(image);
 *  });
 * @endcode
 */
template <typename tValue, typename tError = void>
class Result final : public detail::ResultImpl<tValue, tError>
{
    static_assert(!std::is_reference_v<tValue>, "Result class cannot store reference to value");
    static_assert(!std::is_reference_v<tError>, "Result class cannot store reference to error");

private:
    using BaseResultType = detail::ResultImpl<tValue, tError>;

public:
    using BaseResultType::err;
    using BaseResultType::ok;

public:
    using BaseResultType::BaseResultType;

    /// Construct directly from a value.
    // Implicit conversion is desired for this class
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(tValue &&value)
        : BaseResultType(result::Ok{std::move(value)})
    {
    }

    /// Construct directly from a value.
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(const tValue &value)
        : BaseResultType(result::Ok{value})
    {
    }

    Result(Result &&) noexcept = default;
    Result &operator=(Result &&) noexcept = default;
    Result(const Result &) = default;
    Result &operator=(const Result &) = default;

    /// Get error value
    using BaseResultType::errorValue;
    /// Get value
    using BaseResultType::value;

    ///@{
    /// Returns a pointer to the value. The operator does not check whether the @ref Result contains
    /// value. The behaviour is undefined if @c *this is empty.
    constexpr const tValue *operator->() const noexcept
    {
        return &BaseResultType::operator*();
    }
    /// Returns a reference to the value. The operator does not check whether the @ref Result
    /// contains value. The behaviour is undefined if @c *this is empty.
    constexpr const tValue &operator*() const noexcept
    {
        return BaseResultType::operator*();
    }
    ///@}

    using BaseResultType::operator==;
    using BaseResultType::operator!=;

    /**
     * Maps @c Result<T, Error> to @c Result<U, Error> by applying function to the
     * contained value, leaving the Error untouched.
     *
     * @return @c Result<U, Error>, where @c T is transformed to @c U via function @c F
     */
    using BaseResultType::map;
    /// Get value if it is set, otherwise return default value
    using BaseResultType::valueOr;
};

template <typename tError>
class Result<void, tError> final : public detail::ResultImpl<void, tError>
{
    static_assert(!std::is_reference_v<tError>, "Result class cannot store reference to error");

private:
    using BaseResultType = detail::ResultImpl<void, tError>;

public:
    using BaseResultType::err;
    using BaseResultType::ok;

public:
    using BaseResultType::BaseResultType;

    Result()
        : BaseResultType(result::Ok{})
    {
    }

    Result(Result &&) noexcept = default;
    Result &operator=(Result &&) noexcept = default;
    Result(const Result &) = default;
    Result &operator=(const Result &) = default;

    using BaseResultType::errorValue;

    using BaseResultType::operator==;
    using BaseResultType::operator!=;

    using BaseResultType::map;
};

template <>
class Result<void, void> final : public detail::ResultImpl<void, void>
{
private:
    using BaseResultType = detail::ResultImpl<void, void>;

public:
    using BaseResultType::BaseResultType;

    Result()
        : BaseResultType(result::Ok{})
    {
    }

    Result(Result &&) noexcept = default;
    Result &operator=(Result &&) noexcept = default;
    Result(const Result &) = default;
    Result &operator=(const Result &) = default;

    using BaseResultType::operator==;
    using BaseResultType::operator!=;

    using BaseResultType::map;
};

template <typename tValue, typename tError>
std::ostream &operator<<(std::ostream &os, const Result<tValue, tError> &result) noexcept
{
    if (result.hasError()) {
        if constexpr (std::is_void_v<tError>) {
            return os << "Err{}";
        } else {
            return os << "Err{" << detail::printable(result.errorValue()) << "}";
        }
    }

    if constexpr (std::is_void_v<tValue>) {
        return os << "Ok{}";
    } else {
        return os << "Ok{" << detail::printable(result.value()) << "}";
    }
}

namespace detail {

template <typename T>
struct IsResult : std::false_type
{
};

template <template <class, class> class T, typename tValue, typename tError>
struct IsResult<T<tValue, tError>> : std::is_same<T<tValue, tError>, Result<tValue, tError>>
{
};

} // namespace detail

/**
 * @ingroup nf_core_Types
 * @brief Type trait to check if the given type is an instance of @ref nf::Result.
 */
template <class T>
inline constexpr bool IsResult = detail::IsResult<T>::value;

NF_END_NAMESPACE
