/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 */

#pragma once

#include <nf/Future.h>
#include <nf/Promise.h>
#include <nf/detail/MulticastSharedState.h>

#include <memory>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Async
 * @brief Asynchronous multicast promise handle.
 *
 * When this promise is fulfilled value is sent to multiple continuations.
 * Continuations can be attached to Futures obtained from future().
 *
 * @tparam tResult Type of result returned by this promise
 */
template <typename tResult>
class MulticastPromise
{
    friend class Future<tResult>;

    using tSharedState = detail::MulticastSharedState<tResult>;

    static_assert(std::is_void_v<tResult> || std::is_copy_constructible_v<tResult>,
                  "MulticastPromises can only be created for copy-constructible types");
    static_assert(!std::is_reference_v<tResult>,
                  "Cannot construct MulticastPromise for a reference");

public:
#ifdef DOXYGEN
    /**
     * Fulfill this promise and notify all futures that have been obtained from this promise
     * using future(). Note that the given value must be copyable because each pending
     * future will be fulfilled with a separate copy of the object.
     *
     * @warning Must be called exactly once. Calling it second time results in program termination.
     *
     * @param value The value to fulfill the promise with
     */
    template <typename T = tResult>
    void fulfill(T value) const;

    /**
     * Fulfill this promise and notify all futures that have been obtained using
     * future(). Overload for the void result type.
     *
     * @warning Must be called exactly once. Calling it second time results in program termination.
     */
    void fulfill() const noexcept;
#else
    template <typename T = tResult, typename std::enable_if_t<!std::is_void_v<T>> * = nullptr>
    void fulfill(T value) const
    {
        static_assert(std::is_constructible_v<tResult, T>,
                      "Unable to construct result from passed value");

        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        m_state->setValue(std::move(value));
    }

    template <typename T = tResult, typename std::enable_if_t<std::is_void_v<T>> * = nullptr>
    void fulfill() const noexcept
    {
        assertThat(m_state, "Promise is not usable");
        m_state->setValue(std::monostate{});
    }
#endif

    /**
     * Get a new future for this multicast promise. All futures obtained from this
     * promise will be completed with a copy of the result at the same time.
     *
     * @note Can be called multiple times to obtain more futures for this promise.
     *
     * @return Future
     */
    [[nodiscard]] Future<tResult> future() const noexcept
    {
        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        return m_state->future();
    }

private:
    // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
    typename tSharedState::Ptr m_state = std::make_shared<tSharedState>();
};

NF_END_NAMESPACE
