/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Nikolay Kutyavin <nikolai.kutiavin@bmw.de>
 */

#include "nf/Printable.h"

#include <array>
#include <charconv>
#include <cstring>
#include <sstream>

NF_BEGIN_NAMESPACE

std::string detail::toHex(const std::byte *obj, std::size_t size)
{
    if (obj == nullptr || size == 0) {
        return {};
    }

    std::stringstream ss;

    ss << std::setfill('0') << std::uppercase << std::hex;
    for (const auto *it = obj; it != (obj + size); ++it) {
        ss << std::setw(2) << std::to_integer<int>(*it) << ' ';
    }

    auto ret = ss.str();
    ret.pop_back();
    return ret;
}

std::string strerror(int error) noexcept
{
    static constexpr std::size_t BufferSize = 256;
    std::array<char, BufferSize> buffer; // NOLINT
    /* Note, there are two versions of strerror_r. This will compile only if the GNU
     * version is being used. It returns char*. */
    return ::strerror_r(error, buffer.data(), buffer.size());
}

NF_END_NAMESPACE
