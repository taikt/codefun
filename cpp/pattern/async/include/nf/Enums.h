/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Jan Ehrenberger <jan.ehrenberger@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#include <array>
#include <cstdint>

/**
 * @addtogroup nf_core_Enums
 * @{
 */

/**
 * @hideinitializer
 * @brief Define a scoped enumeration and its utility functions.
 * @param __typename Enumeration typename.
 * @param ... Enumeration values.
 *
 * This macro defines a @cppdoc{scoped enumeration,language/enum} with the provided
 * typename together with utility functions to validate the given input and to convert
 * to/from a string.
 *
 * @b Usage
 *
 * @code
 * NF_ENUM(Color, Red, Green, Blue)
 * @endcode
 * defines a scoped enumeration called @c Color, which has three values: @c Color::Red,
 * @c Color::Green and @c Color::Blue. It also defines these free utility functions:
 *
 * - @code bool isValid(Color color); @endcode
 *   Check if the provided value is within an enumeration's range.
 *
 * - @code const char *toString(Color color); @endcode
 *   Convert the provided value to its textual representation. If the value is outside
 *   of the enumeration's range, '\<undefined\>' is returned.
 *
 * - @code Color fromString(const char *str, Color defaultValue); @endcode
 *   Convert the provided textual representation to a corresponding enumeration value. If
 *   there is no such value, the provided default value is returned.
 *
 * @b Limitations
 *
 * This macro can be used only to define free enumerations. It cannot be used inside classes.
 * See @ref NF_ENUM_CLASS.
 *
 * It is not possible to assign specific integer values to enumeration values.
 */
#define NF_ENUM(__typename, ...)                                                                   \
    enum class __typename                                                                          \
    {                                                                                              \
        __VA_ARGS__                                                                                \
    };                                                                                             \
    NF_ENUM_DEFINE_UTILS(__typename, __VA_ARGS__)

/**
 * @hideinitializer
 * @brief Define a scoped enumeration and its utility functions inside class definition.
 * @param __typename Enumeration typename.
 * @param ... Enumeration values.
 *
 * This is a version of macro @ref NF_ENUM that can be used inside class definition.
 *
 * @b Limitations
 *
 * - This macro can be used only to define enumeration in class scope. To define free standing
 *   enumeration, use @ref NF_ENUM.
 *
 * - It is not possible to assign specific integer values to enumeration values.
 */
#define NF_ENUM_CLASS(__typename, ...)                                                             \
    enum class __typename                                                                          \
    {                                                                                              \
        __VA_ARGS__                                                                                \
    };                                                                                             \
    _nf_ENUM_DEFINE_UTILS(__typename, __typename, friend, __VA_ARGS__)

#define _nf_ENUM_MKNAME(_1, _2, _3, __name) BOOST_PP_STRINGIZE(__name),
#define _nf_ENUM_VALUE(_1, __type, _3, __name) __type::__name,
#define _nf_ENUM_CASE(_1, __type, _3, __name) case __type::__name:
#define _nf_ENUM_CASE_RETURN_IDX(_1, __type, __idx, __name)                                        \
    case __type::__name:                                                                           \
        return __idx;
#define _nf_ENUM_FOREACH(__macro, __arg, ...)                                                      \
    BOOST_PP_SEQ_FOR_EACH_I(__macro, __arg, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define NF_ENUM_DEFINE_UTILS(__typename, ...)                                                      \
    _nf_ENUM_DEFINE_UTILS(__typename, __typename, , __VA_ARGS__)

/**
 * @hideinitializer
 * @brief Define utility functions for an enumeration in a class.
 */
#define NF_ENUM_DEFINE_UTILS_C(__class, __typename, ...)                                           \
    _nf_ENUM_DEFINE_UTILS(__class::__typename, __class##_##__typename, , __VA_ARGS__)

/**
 * @hideinitializer
 * @brief Define meta-information and utility classes for a given enumeration.
 * @param __typename Enumeration typename.
 * @param __scope Name of scope (struct) that is used to hold enum's meta-information.
 * @param ... Enumeration values.
 *
 * This macro defines:
 * - A helper struct called @c \_nf_enum_\<@e __scope\>, with static functions and members:
 *   - @c strings() to access array of textual representations of the provided enumeration values.
 *   - @c idx(v) to access index of enumeration value @c v in the list of enumeration values. First
 *     enumeration value has index 0.
 *   - @c size contains number of enumeration values. This is also size of the array returned by
 *     @c strings().
 * - Free functions @c isValid(), @c toString() and @c fromString(). See @ref NF_ENUM for what these
 *   functions do.
 *
 * @c isValid() uses a @e switch-statement to ensure that all enumeration values of @c __typename
 * are listed.
 */
#define NF_ENUM_DEFINE_UTILS_NS(__typename, __scope, ...)                                          \
    _nf_ENUM_DEFINE_UTILS(__typename, __scope, , __VA_ARGS__)


#define _nf_ENUM_DEFINE_UTILS(__typename, __scope, __function_prefix, ...)                         \
    struct _nf_enum_##__scope                                                                      \
    {                                                                                              \
        static constexpr auto strings() noexcept                                                   \
        {                                                                                          \
            constexpr std::array<const char *, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)> data{          \
                _nf_ENUM_FOREACH(_nf_ENUM_MKNAME, , __VA_ARGS__)};                                 \
            return data;                                                                           \
        }                                                                                          \
        static constexpr unsigned idx(__typename v) noexcept                                       \
        {                                                                                          \
            switch (v) {                                                                           \
                _nf_ENUM_FOREACH(_nf_ENUM_CASE_RETURN_IDX, __typename, __VA_ARGS__)                \
            }                                                                                      \
            return 0;                                                                              \
        }                                                                                          \
        static constexpr int size = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);                           \
    };                                                                                             \
    [[gnu::unused]] __function_prefix inline constexpr bool isValid(__typename v) noexcept         \
    {                                                                                              \
        switch (v) {                                                                               \
            _nf_ENUM_FOREACH(_nf_ENUM_CASE, __typename, __VA_ARGS__) return true;                  \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
    [[gnu::unused]] __function_prefix inline constexpr const char *toString(__typename v) noexcept \
    {                                                                                              \
        if (!isValid(v)) {                                                                         \
            return "<undefined>";                                                                  \
        }                                                                                          \
        return _nf_enum_##__scope::strings()[_nf_enum_##__scope::idx(v)];                          \
    }                                                                                              \
    [[gnu::unused]] __function_prefix inline bool fromString(__typename &vout,                     \
                                                             const char *s) noexcept               \
    {                                                                                              \
        constexpr std::array<__typename, _nf_enum_##__scope::size> values = {                      \
            _nf_ENUM_FOREACH(_nf_ENUM_VALUE, __typename, __VA_ARGS__)};                            \
        for (uint32_t i = 0; i < _nf_enum_##__scope::size; i++) {                                  \
            if (__builtin_strcmp(s, _nf_enum_##__scope::strings()[i]) == 0) {                      \
                vout = values[i];                                                                  \
                return true;                                                                       \
            }                                                                                      \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
    [[gnu::unused]] __function_prefix inline __typename fromString(const char *s,                  \
                                                                   __typename def) noexcept        \
    {                                                                                              \
        fromString(def, s);                                                                        \
        return def;                                                                                \
    }

/// @}
