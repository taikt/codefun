/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Wawrzyniec Kaczmarowski <wawrzyniec.kaczmarowski@partner.bmw.de>
 */

#include "nf/Version.h"

#include <boost/algorithm/string.hpp>

#include <sstream>
#include <tuple>
#include <vector>

using namespace nf;

Version::Version(uint16_t major, uint16_t minor, uint16_t patch) noexcept
    : m_majorVersion(major)
    , m_minorVersion(minor)
    , m_patch(patch)
{
}

Version::Version(uint16_t major, uint16_t minor, uint16_t patch, std::string preRelease,
                 std::string metaData) noexcept

    : m_majorVersion(major)
    , m_minorVersion(minor)
    , m_patch(patch)
    , m_preRelease(std::move(preRelease))
    , m_metaData(std::move(metaData))
{
}

uint16_t Version::majorVersion() const noexcept
{
    return m_majorVersion;
}

uint16_t Version::minorVersion() const noexcept
{
    return m_minorVersion;
}

uint16_t Version::patch() const noexcept
{
    return m_patch;
}

std::optional<std::string> Version::preRelease() const noexcept
{
    return m_preRelease;
}

std::optional<std::string> Version::metaData() const noexcept
{
    return m_metaData;
}

bool Version::operator<(const Version &rhs) const noexcept
{
    std::vector<std::string> lhsPreReleaseParts;
    std::vector<std::string> rhsPreReleaseParts;

    if (this->m_preRelease.has_value()) {
        boost::split(lhsPreReleaseParts, this->m_preRelease.value(), boost::is_any_of("."));
    }

    if (rhs.m_preRelease.has_value()) {
        boost::split(rhsPreReleaseParts, rhs.m_preRelease.value(), boost::is_any_of("."));
    }

    return std::tie(this->m_majorVersion, this->m_minorVersion, this->m_patch, lhsPreReleaseParts)
        < std::tie(rhs.m_majorVersion, rhs.m_minorVersion, rhs.m_patch, rhsPreReleaseParts);
}

std::ostream &nf::operator<<(std::ostream &os, const Version &version) noexcept
{
    os << 'v' << version.majorVersion() << "." << version.minorVersion() << "." << version.patch();

    if (version.preRelease().has_value()) {
        os << '-' << version.preRelease().value();
    }

    if (version.metaData().has_value()) {
        os << '+' << version.metaData().value();
    }

    return os;
}
