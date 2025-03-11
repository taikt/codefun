/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include "detail/AbstractUnit.h"

NF_BEGIN_NAMESPACE

template <typename T, class tRatio>
class DataSize : public detail::AbstractUnit<DataSize, T, tRatio>
{
    using detail::AbstractUnit<DataSize, T, tRatio>::AbstractUnit;
};

/**
 * @addtogroup nf_core_Units
 * @{
 */

/**
 * @name Data Sizes
 * @{
 *
 * Type-safe representations of various data sizes with binary prefixes, i.e. 1024-based
 * instead of 1000-based. Data size types use internally a signed 64-bit integer to store
 * their values.
 *
 * @par Literals
 *
 * These literals are defined for data sizes: @c _B for bytes, @c _KiB for kibibytes, @c
 * _MiB for mebibyte and @c _GiB for gibibytes.
 *
 * @code
 * using nf::unit_literals;
 * foo(10_GiB); // equivalent to foo(nf::Gibibytes{10})
 * @endcode
 *
 * @par Conversions
 *
 * Data sizes can be implicitly converted to data sizes of a smaller unit, but not vice-versa.
 * For example, mebibytes are implicitly convertible to kibibytes or bytes. If the opposite
 * is needed, then data sizes must be explicitly converted with @ref datasize_cast().
 *
 * If an expression contains different data size units, they will be implicitly converted to
 * the smallest unit and a result of such expression will be of this type. For example, @c
 * a in the following snippet will be of type @ref Bytes and will have a value of 1025.
 * @code
 * auto a = 1_KiB + 1_B;
 * @endcode
 *
 * @par Streaming
 *
 * Data sizes can be streamed to @c std::ostream.
 *
 * @par Operations
 *
 * Data sizes can be:
 * - Added to other data sizes.
 * - Subtracted from other data sizes.
 * - Compared with other data sizes.
 * - Divided by other data sizes (to find the ratio).
 * - Multiplied by an integer factor.
 * - Divided by an integer factor.
 *
 * Some examples:
 * @code
 * 3_MiB + 1_MiB  // 4_MiB
 * 3_MiB - 1_MiB  // 2_MiB
 * 3_MiB < 1_MiB  // false
 * 3_MiB == 1_MiB // false
 * 3_MiB > 1_MiB  // true
 * 3_MiB / 1_MiB  // 3.0
 * 3_MiB * 2      // 6_MiB
 * 3_MiB / 2      // 1_MiB (integer division)
 * @endcode
 *
 * --
 */

/// Bytes (B).
using Bytes = DataSize<std::int64_t, std::ratio<1>>;
/// Kibibytes (KiB, 1024 bytes).
using Kibibytes = DataSize<std::int64_t, std::ratio<1024>>;
/// Mebibytes (MiB, 1024 KiB).
using Mebibytes = DataSize<std::int64_t, std::ratio<1048576>>;
/// Gibibytes (GiB, 1024 MiB).
using Gibibytes = DataSize<std::int64_t, std::ratio<1073741824>>;

/**
 * @brief Explicit cast to a different data size unit.
 *
 * This function must be used to cast smaller data sizes to larger data sizes, e.g. bytes
 * to kibibytes. Usually such cast will result in loss of information because, e.g. a number
 * of bytes cannot always be represented as an integer number of kibibytes. In such cases
 * the result will be rounded down to the closest value of the larger type.
 *
 * @code
 * datasize_cast<Kibibytes>(520_B) == 0_KiB; // rounded down
 * @endcode
 */
template <typename tNewUnit, typename tOldUnit>
constexpr tNewUnit datasize_cast(const tOldUnit &unit) noexcept
{
    return detail::abstractunit_cast<tNewUnit, tOldUnit, false>(unit);
}

/// @} // data sizes

std::ostream &operator<<(std::ostream &os, const Bytes &bytes) noexcept;
std::ostream &operator<<(std::ostream &os, const Kibibytes &kib) noexcept;
std::ostream &operator<<(std::ostream &os, const Mebibytes &mib) noexcept;
std::ostream &operator<<(std::ostream &os, const Gibibytes &gib) noexcept;

namespace unit_literals {

NF_UNIT_LITERAL_INT(Bytes, B)
NF_UNIT_LITERAL_INT(Kibibytes, KiB)
NF_UNIT_LITERAL_INT(Mebibytes, MiB)
NF_UNIT_LITERAL_INT(Gibibytes, GiB)

} // namespace unit_literals

NF_END_NAMESPACE
