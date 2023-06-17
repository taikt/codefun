/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Nikolay Kutyavin <nikolai.kutiavin@bmw.de>
 */

#pragma once

#include <nf/Demangle.h>

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <tuple>

NF_BEGIN_NAMESPACE

namespace detail {

// clang-format off
struct PrintContainer {};
struct PrintOptional {};
struct PrintPair {};
struct PrintTuple {};
struct PrintValue {};
struct PrintEnum {};
// clang-format on

template <typename tPrintable, typename T>
struct Printable
{
    constexpr explicit Printable(const T &value) noexcept
        : value(&value)
    {
    }

    const T *value;
};

template <typename T>
struct Printable<PrintValue, T>
{
    constexpr explicit Printable(const T &value) noexcept
        : value(value)
    {
    }

    const T value;
};

// clang-format off
/* These are helper classes which are needed to define an overload resolution order. First,
 * try to use an ostream operator and, if it is not available, use toString(). And so on. */
struct TryHexRepresentation{};
struct TryPrintable : TryHexRepresentation {};
struct TryToString : TryPrintable {};
struct TryOStream : TryToString {};
// clang-format on

// SFINAE-1. Return a passed value if it has a defined streaming operator.
template <typename T>
auto printable(const T &value, TryOStream) noexcept
    -> decltype(std::declval<std::ostream &>() << std::declval<T>(), std::declval<const T &>())
{
    return value;
}

// SFINAE-2. Return a value returned by toString() if this function is defined.
template <typename T>
auto printable(const T &value, TryToString) noexcept -> decltype(toString(std::declval<T>()))
{
    return toString(value);
}

// SFINAE-3. Return a printable container.
template <typename T>
auto printable(const T &value, TryPrintable) noexcept
    -> decltype(std::declval<T>().cbegin() == std::declval<T>().cend(),
                Printable<PrintContainer, T>(std::declval<T>()))
{
    return Printable<PrintContainer, T>(value);
}

// SFINAE-4. Return a printable optional.
template <typename T>
auto printable(const T &value, TryPrintable) noexcept
    -> decltype(std::declval<T>().has_value(), Printable<PrintOptional, T>(std::declval<T>()))
{
    return Printable<PrintOptional, T>(value);
}

// SFINAE-5. Return a printable pair.
template <typename T>
auto printable(const T &value, TryPrintable) noexcept
    -> decltype(std::declval<T>().first, std::declval<T>().second,
                Printable<PrintPair, T>(std::declval<T>()))
{
    return Printable<PrintPair, T>(value);
}

// SFINAE-6. Return a printable tuple.
template <typename... Ts>
auto printable(const std::tuple<Ts...> &value, TryPrintable) noexcept
    -> Printable<PrintTuple, std::tuple<Ts...>>
{
    return Printable<PrintTuple, std::tuple<Ts...>>(value);
}

// SFINAE-7. Return a printable enum.
template <typename T>
auto printable(const T &value, TryPrintable) noexcept
    -> decltype(std::is_enum_v<T>, std::underlying_type_t<T>(),
                Printable<PrintEnum, T>(std::declval<T>()))
{
    return Printable<PrintEnum, T>(value);
}

// implementation for an arbitrary object to a hex-string conversion
std::string toHex(const std::byte *obj, std::size_t size);

// SFINAE-8. Return an object representation as hex-string.
template <typename T>
std::string printable(const T &value, TryHexRepresentation) noexcept
{
    constexpr std::size_t maxSize = 64;
    constexpr std::size_t size = std::min(maxSize, sizeof(T));
    std::string res;
    res.append(nf::demangle<T>());
    res.append("{");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    res.append(toHex(reinterpret_cast<const std::byte *>(&value), size));

    if constexpr (sizeof(T) > maxSize) {
        res.append(" ...");
    }
    res.append("}");

    return res;
}

// Try to use SFINAE-1 overload first, and if it fails, use SFINAE-2. And so on.
template <typename T>
auto printable(const T &value) noexcept -> decltype(printable(std::declval<T>(), TryOStream{}))
{
    return printable(value, TryOStream{});
}

// The same as above but exclude SFINAE-1 (streaming operator). This must be used in operator<<
// wrappers to avoid recursive calls to operator<<.
template <typename T>
auto exclusivePrintable(const T &value) noexcept
    -> decltype(printable(std::declval<T>(), TryToString{}))
{
    return printable(value, TryToString{});
}

inline auto printable(const char *str) noexcept
{
    return std::quoted(str);
}

inline auto printable(const std::string &str) noexcept
{
    return std::quoted(str);
}

inline int printable(std::uint8_t byte) noexcept
{
    return byte;
}

inline auto printable(std::byte byte) noexcept
{
    return Printable<PrintValue, std::byte>(byte);
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Printable<PrintContainer, T> &printable) noexcept
{
    const auto &container = *printable.value;
    const auto flags = os.flags();
    os << "(" << std::setbase(10) << container.size() << ")[";
    os.flags(flags);
    if (!container.empty()) {
        const auto &end = container.cend();
        auto it = container.cbegin();

        os << detail::printable(*it);
        for (++it; it != end; ++it) {
            os << ", " << detail::printable(*it);
        }
    }
    return os << "]";
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Printable<PrintEnum, T> &printable) noexcept
{
    const auto &enumVal = *printable.value;
    return os << static_cast<typename std::underlying_type<T>::type>(enumVal);
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Printable<PrintOptional, T> &printable) noexcept
{
    const auto &optional = *printable.value;
    if (optional.has_value()) {
        return os << detail::printable(optional.value());
    }
    return os << "<nullopt>";
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Printable<PrintPair, T> &printable) noexcept
{
    const auto &pair = *printable.value;
    return os << "{" << detail::printable(pair.first) << ", " << detail::printable(pair.second)
              << "}";
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Printable<PrintTuple, T> &printable) noexcept
{
    os << "{";
    std::apply(
        /* This lambda will be called with args equal to tuple elements. */
        [&os](const auto &...args) {
            bool isFirst{true};
            auto print = [&](auto arg) {
                if (isFirst) {
                    os << detail::printable(arg);
                    isFirst = false;
                } else {
                    os << ", " << detail::printable(arg);
                }
            };
            /* Now we use the (lambda , ...) fold expression to iterate over each argument
             * (tuple element) and print it. */
            (print(args), ...);
        },
        *printable.value);
    return os << "}";
}

inline std::ostream &operator<<(std::ostream &os,
                                const Printable<PrintValue, std::byte> &printable) noexcept
{
    char buffer[3];
    std::snprintf(buffer, 3, "%02hhX", std::to_integer<unsigned char>(printable.value));
    return os << buffer;
}

} // namespace detail

/**
 * @ingroup nf_core_Logging
 * @brief Return a value's printable representation.
 *
 * This function converts the provided value of an arbitrary type @c T to its printable
 * representation, i.e. something that has a streaming operator and can be sent to an output
 * stream or logged:
 * @code
 * std::ostream &operator<<(std::ostream &os, const T &value) noexcept;
 * @endcode
 *
 * @par Formatting Rules
 *
 * It implements this logic:
 * - If @c T is a string, then this function is equivalent to
 *   @cppdoc{std::quoted(value),io/manip/quoted}.
 * - Else, if @c T is @c std::uint8_t, then this function casts it to @c int.
 * - Else, if @c T is @c std::byte, then this function returns a @e printable wrapper object
 *   which prints a byte as two uppercase hex digits.
 * - Else, if there is a streaming operator defined for @c T, then this function returns a
 *   reference to the passed value without any modification.
 * - Else, if @c toString() is defined for @c T, then this function is equivalent to @c
 *   toString(value).
 * - Else, if @c T is a container type (e.g. @c std::vector), this function returns a @e
 *   printable wrapper object which prints a container size and all its elements, recursively
 *   applying @ref nf::printable():
 *   @code (container-size)[printable-elem-0, printable-elem-1, ...] @endcode
 * - Else, if @c T is @c std::optional, this function return a @e printable wrapper object
 *   which prints the provided value, applying @ref nf::printable(), or "<nullopt>" if the
 *   value is not set.
 * - Else, if @c T is @c std::pair or @c std::tuple, this function returns a @e printable
 *   wrapper object which prints all pair or tuple elements, recursively applying @ref
 *   nf::printable():
 *   @code {printable-elem-0, printable-elem-1, ...} @endcode
 * - Else, this function returns @e Base16 encoded binary presentation of @c value as a hex-string.
 *   The returned string also contains the name of @c T. This function prints only the first @b 64
 *   bytes of the object, and appends the resulting string with @e "...", if the object is larger
 *   than that.
 *
 *   An ouput sample for some class @c ClassName :
 *   @code ClassName{0A 41 4F 41} @endcode
 *
 * @par Usage
 *
 * This function is intended to be used in a templated code where one needs to log an element
 * of an arbitrary type. For other cases, when the type is known, one should prefer other
 * alternatives like calling @c toString() directly or including @c ContainerIoStream.h.
 *
 * @since 3.16
 */
template <typename T>
auto printable(const T &value) noexcept -> decltype(detail::printable(value))
{
    return detail::printable(value);
}

/**
 * @brief Thread-safe version of std::strerror().
 *
 * This is a thread-safe version of @cppdoc{std::strerror(),string/byte/strerror}.
 * It returns the textual description of the provided system @p error code.
 *
 * @since 5.0
 */
std::string strerror(int error) noexcept;

NF_END_NAMESPACE
