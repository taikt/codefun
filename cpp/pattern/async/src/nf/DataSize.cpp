/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/DataSize.h"

#include <iostream>

NF_BEGIN_NAMESPACE

std::ostream &operator<<(std::ostream &os, const Bytes &bytes) noexcept
{
    return os << bytes.value() << " B";
}

std::ostream &operator<<(std::ostream &os, const Kibibytes &kib) noexcept
{
    return os << kib.value() << " KiB";
}

std::ostream &operator<<(std::ostream &os, const Mebibytes &mib) noexcept
{
    return os << mib.value() << " MiB";
}

std::ostream &operator<<(std::ostream &os, const Gibibytes &gib) noexcept
{
    return os << gib.value() << " GiB";
}

NF_END_NAMESPACE
