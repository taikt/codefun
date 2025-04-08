/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <boost/operators.hpp>

#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <ratio>
#include <type_traits>

NF_BEGIN_NAMESPACE

namespace detail {

/// @todo Extract DefaultTolerance for floating point comparison.
static constexpr double DefaultTolerance = 0.000001;

/* Comparison for integral types. */
template <typename tIntegral>
typename std::enable_if<std::is_integral<tIntegral>::value, bool>::type
equal(const tIntegral &a, const tIntegral &b) noexcept
{
    return a == b;
}

/* Comparison for floating point types. */
template <typename tFloating>
typename std::enable_if<std::is_floating_point<tFloating>::value, bool>::type
equal(const tFloating &a, const tFloating &b) noexcept
{
    return std::fabs(a - b) < DefaultTolerance;
}

/* This type trait checks that two provided unit types are related, e.g. both are data sized
 * or both are distances. Related units must also have the same underlying type. */
template <typename tUnit1, typename tUnit2>
struct IsRelatedUnit : public std::false_type
{
};

template <template <typename, typename> class tUnit, typename T, typename tRatio1, typename tRatio2>
struct IsRelatedUnit<tUnit<T, tRatio1>, tUnit<T, tRatio2>> : public std::true_type
{
};

template <typename tNewUnit, typename tOldUnit>
struct UnitCastTraits
{
    static_assert(IsRelatedUnit<tNewUnit, tOldUnit>::value, "Units are not related");

    using Type = typename tNewUnit::Type;
    using ToRatio = typename tNewUnit::Ratio;
    using FromRatio = typename tOldUnit::Ratio;
    using Quotient = std::ratio_divide<FromRatio, ToRatio>;

    /* For integer-based units it is unsafe to convert from a smaller unit, e.g. metres,
     * to a larger unit, e.g. kilometres. For example, 10m cannot be exactly represented
     * in integer kilometres. */
    static constexpr bool IsSafe = std::is_same<Type, typename tOldUnit::Type>::value
        && (std::is_floating_point<Type>::value || (Quotient::den == 1));

    using SafeUnit = std::conditional_t<IsSafe, tNewUnit, tOldUnit>;
};

template <typename tNewUnit, typename tOldUnit, bool OnlySafeCast = true>
constexpr tNewUnit abstractunit_cast(const tOldUnit &unit) noexcept
{
    using Traits = UnitCastTraits<tNewUnit, tOldUnit>;
    using Type = typename tNewUnit::Type;
    using Quotient = typename Traits::Quotient;

    static_assert(!OnlySafeCast || Traits::IsSafe, "Unsafe conversion to a larger unit");

    if (OnlySafeCast && !std::is_floating_point<Type>::value) {
        return tNewUnit{unit.value() * Quotient::num};
    }

    const double newValue =
        static_cast<double>(unit.value()) * (static_cast<double>(Quotient::num) / Quotient::den);

    return tNewUnit{static_cast<Type>(newValue)};
}

#define _nf_FIND_SAFE_UNIT() using SafeUnit = typename UnitCastTraits<Unit, xUnit>::SafeUnit
#define _nf_SAFE_VALUE(__unit) abstractunit_cast<SafeUnit>(__unit).value()
#define _nf_SAFE_THIS_VALUE() _nf_SAFE_VALUE(static_cast<const Unit &>(*this))
#define _nf_ENABLE_IF_RELATED_UNITS std::enable_if_t<IsRelatedUnit<Unit, xUnit>::value, int> = 0

#define NF_UNIT_OPERATOR(__retval, __op)                                                           \
    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>                                         \
    constexpr auto operator __op(const xUnit &rhs) const noexcept                                  \
    {                                                                                              \
        _nf_FIND_SAFE_UNIT();                                                                      \
        return __retval(_nf_SAFE_THIS_VALUE() __op _nf_SAFE_VALUE(rhs));                           \
    }

#define _nf_UNIT_LITERAL(__unit, __literal, __intype)                                              \
    constexpr __unit operator"" _##__literal(__intype value) noexcept                              \
    {                                                                                              \
        return __unit(static_cast<__unit::Type>(value));                                           \
    }

#define NF_UNIT_LITERAL_INT(__unit, __literal)                                                     \
    _nf_UNIT_LITERAL(__unit, __literal, unsigned long long)
#define NF_UNIT_LITERAL_DOUBLE(__unit, __literal) _nf_UNIT_LITERAL(__unit, __literal, long double)
#define NF_UNIT_LITERAL_BOTH(__unit, __literal)                                                    \
    NF_UNIT_LITERAL_INT(__unit, __literal)                                                         \
    NF_UNIT_LITERAL_DOUBLE(__unit, __literal)

/* This class is a base for type-safe representations of various units with any numerical
 * underlying type. */
template <template <typename, typename> class tUnit, typename T, class tRatio>
class AbstractUnit
{
public:
    using Type = T;
    using Ratio = tRatio;
    using Unit = tUnit<T, tRatio>;

    constexpr AbstractUnit() noexcept = default;

    explicit constexpr AbstractUnit(const T &value) noexcept
        : m_value(value)
    {
    }

    template <typename tRatio2, typename = std::enable_if_t<!std::is_same_v<tRatio, tRatio2>>>
    constexpr AbstractUnit( // NOLINT(google-explicit-constructor)
        const tUnit<T, tRatio2> &d) noexcept
        : m_value(abstractunit_cast<tUnit<T, tRatio>>(d).value())
    {
    }

    constexpr T value() const noexcept
    {
        return m_value;
    }

    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>
    constexpr bool operator==(const xUnit &rhs) const noexcept
    {
        _nf_FIND_SAFE_UNIT();
        return equal(_nf_SAFE_THIS_VALUE(), _nf_SAFE_VALUE(rhs));
    }

    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>
    constexpr bool operator!=(const xUnit &rhs) const noexcept
    {
        return !(*this == rhs);
    }

    NF_UNIT_OPERATOR(bool, <)
    NF_UNIT_OPERATOR(bool, <=)
    NF_UNIT_OPERATOR(bool, >)
    NF_UNIT_OPERATOR(bool, >=)
    NF_UNIT_OPERATOR(SafeUnit, +)
    NF_UNIT_OPERATOR(SafeUnit, -)

    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>
    constexpr auto operator/(const xUnit &rhs) const noexcept
    {
        _nf_FIND_SAFE_UNIT();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        return static_cast<double>(_nf_SAFE_THIS_VALUE()) / _nf_SAFE_VALUE(rhs);
#pragma GCC diagnostic pop
    }

    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>
    constexpr void operator+=(const xUnit &rhs) noexcept
    {
        m_value += abstractunit_cast<Unit>(rhs).value();
    }

    template <typename xUnit, _nf_ENABLE_IF_RELATED_UNITS>
    constexpr void operator-=(const xUnit &rhs) noexcept
    {
        m_value -= abstractunit_cast<Unit>(rhs).value();
    }

    constexpr Unit operator*(T factor) const noexcept
    {
        return Unit{m_value * factor};
    }

    constexpr friend Unit operator*(T factor, const Unit &rhs) noexcept
    {
        return Unit{rhs.value() * factor};
    }

    constexpr Unit operator/(T factor) const noexcept
    {
        return Unit{m_value / factor};
    }

    /// Boost serialization method.
    template <class Archive>
    void serialize(Archive &ar, unsigned char /*version*/)
    {
        // clang-format off
        ar & m_value;
        // clang-format on
    }

private:
    T m_value{};
};

} // namespace detail

NF_END_NAMESPACE
