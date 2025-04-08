/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include "Printable.h"

#include <optional>

namespace std { // NOLINT

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::optional<T> &optional) noexcept
{
    return os << nf::detail::exclusivePrintable(optional);
}

} // namespace std
