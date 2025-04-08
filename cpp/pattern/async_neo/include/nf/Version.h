/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Wawrzyniec Kaczmarowski <wawrzyniec.kaczmarowski@partner.bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Types
 * @brief Semantic versioning.
 *
 * This class implements a semantic versioning concept as in https://semver.org/spec/v2.0.0.html.
 * Given a version number MAJOR.MINOR.PATCH, increment the:
 *  - MAJOR version when you make incompatible API changes,
 *  - MINOR version when you add functionality in a backwards compatible manner, and
 *  - PATCH version when you make backwards compatible bug fixes.
 *
 * Additional labels for pre-release and build metadata are available as extensions to the
 * MAJOR.MINOR.PATCH format.
 * Examples:
 * - 1.0.0-alpha+001
 * - 1.0.0+20130313144700
 * - 1.0.0-beta+exp.sha.5114f85
 **/

class Version
{
public:
    Version(uint16_t major, uint16_t minor, uint16_t patch) noexcept;
    Version(uint16_t major, uint16_t minor, uint16_t patch, std::string preRelease,
            std::string metaData = std::string()) noexcept;

    /**
     * @brief Returns the major part of the software version.
     *
     * The major part of software version reffers to changes that make incompatible API code
     * changes.
     **/
    uint16_t majorVersion() const noexcept;

    /**
     * @brief Returns the minor part of the software version.
     *
     * The minor part of software version reffers to adding functionality in a backwards compatible
     * manner.
     **/
    uint16_t minorVersion() const noexcept;

    /**
     * @brief Returns the patch part of the software version.
     *
     * The patch part of software version reffers to backwards compatible bug fixes.
     **/
    uint16_t patch() const noexcept;

    /**
     * @brief Returns the pre-release part of the software version.
     *
     * The pre-release part of software version indicates that the version is unstable and
     * might not satisfy the intended compatibility requirements.
     **/
    std::optional<std::string> preRelease() const noexcept;

    /**
     * @brief Returns the metadata part of the software version.
     **/
    std::optional<std::string> metaData() const noexcept;

    /**
     * @brief Comparasion of two versions.
     *
     * Examples:
     *  - 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
     *  - 1.0.0-alpha < 1.0.0
     *  - 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11
     *    < 1.0.0-rc.1 < 1.0.0.
     **/
    bool operator<(const Version &rhs) const noexcept;

private:
    uint16_t m_majorVersion;
    uint16_t m_minorVersion;
    uint16_t m_patch;
    std::optional<std::string> m_preRelease = std::nullopt;
    std::optional<std::string> m_metaData = std::nullopt;
};

// The streaming operator for @e Version.
std::ostream &operator<<(std::ostream &os, const Version &version) noexcept;

NF_END_NAMESPACE
