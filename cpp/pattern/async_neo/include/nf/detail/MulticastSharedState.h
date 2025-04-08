/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Promise.h>

#include <memory>
#include <optional>
#include <variant>
#include <vector>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename tResult>
class MulticastSharedState
{
public:
    using StorageType = std::conditional_t<std::is_void_v<tResult>, std::monostate, tResult>;
    using Ptr = std::shared_ptr<MulticastSharedState>;

public:
    void setValue(StorageType value)
    {
        std::unique_lock lock(m_mutex);
        assertThat(!m_value, "Value is already set");
        m_value = std::move(value);

        decltype(m_pending) localPending;
        m_pending.swap(localPending);
        for (auto &&promise : localPending) {
            if constexpr (std::is_void_v<tResult>) {
                promise.fulfill();
            } else {
                promise.fulfill(*m_value);
            }
        }
    }

    [[nodiscard]] Future<tResult> future() noexcept
    {
        std::unique_lock lock(m_mutex);
        if (m_value) {
            if constexpr (std::is_void_v<tResult>) {
                return {};
            } else {
                return {*m_value};
            }
        }

        Promise<tResult> promise{};
        auto future = promise.future();
        m_pending.emplace_back(std::move(promise));
        return future;
    }

private:
    std::mutex m_mutex;
    std::optional<StorageType> m_value;
    std::vector<Promise<tResult>> m_pending{};
};

} // namespace detail

NF_END_NAMESPACE
