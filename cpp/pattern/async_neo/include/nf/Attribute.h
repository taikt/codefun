/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#pragma once

#include "Signal.h"

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Signals
 * @brief The attribute.
 */
template <typename tValue>
class Attribute : boost::noncopyable
{
public:
    template <typename xValue = tValue,
              std::enable_if_t<std::is_default_constructible_v<xValue>, bool> = true>
    Attribute() noexcept
    {
    }

    explicit Attribute(const tValue &val) noexcept
        : m_value{val}
    {
    }

    explicit Attribute(tValue &&val) noexcept
        : m_value{std::move(val)}
    {
    }

public:
    /// Signals that the value changed.
    Signal<tValue> changed;

public:
    /// Allows updating the value.
    void set(const tValue &value) noexcept
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        if (m_value != value) {
#pragma GCC diagnostic pop
            m_value = value;
            changed.notify(m_value);
        }
    }

    /// Allows read-only access to the value.
    const tValue &value() const noexcept
    {
        return m_value;
    }

private:
    tValue m_value{};
};

NF_END_NAMESPACE
