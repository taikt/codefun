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

#include <nf/detail/AsyncSharedState.h>

NF_BEGIN_NAMESPACE

template <typename tValue>
class Future;

template <>
class Future<void>;

/**
 * @ingroup nf_core_Async
 * @brief Asynchronous promise handle.
 *
 * Allows to fulfill a promise with a certain value, which is then sent to a continuation.
 * The Future can be obtained from the promise to attach the continuation to.
 *
 * @tparam tValue Type of value returned by this promise
 */
template <typename tValue>
class Promise
{
    static_assert(!std::is_reference_v<tValue>, "Cannot construct Promise for a reference");

public:
    /**
     * @brief Construct an unfulfilled promise.
     */
    Promise() noexcept;

public:
    /**
     * @brief Fulfill promise with value
     *
     * When the promise is fulfilled, the attached continuation is executed via the event loop.
     * The value passed to this function is sent to the continuation function as argument.
     *
     * @param value Value
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     */
    void fulfill(tValue value) const
    {
        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        m_state->setValue(std::move(value));
    }

#ifdef DOXYGEN
    /**
     * @brief Fulfill the promise.
     *
     * @note Only available when @c tValue=void
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     */
    void fulfill() const noexcept;
#endif

    /**
     * @brief Get the future for this promise
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     *
     * @return future
     */
    [[nodiscard]] Future<tValue> future() const noexcept
    {
        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        return Future<tValue>{*this};
    }

    /**
     * @brief Call operator overload which fulfills the promise with a value. See @ref fulfill() for
     * more details.
     *
     * @param value Value
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     */
    void operator()(tValue value) const
    {
        fulfill(std::move(value));
    }

#ifdef DOXYGEN
    /**
     * @brief Call operator overload which fulfills the promise. See @ref fulfill() for
     * more details.
     *
     * @note Only available when @c tValue=void
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     */
    void operator()() const noexcept;
#endif

private:
    friend class Future<tValue>;
    using SharedState = detail::AsyncSharedState<tValue>;

private:
    std::shared_ptr<SharedState> m_state;
};

template <typename tValue>
Promise<tValue>::Promise() noexcept
    : m_state(std::make_shared<SharedState>())
{
}

template <>
class Promise<void>
{
public:
    void fulfill() const noexcept
    {
        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        m_state->setValue({});
    }

    template <typename T = void>
    [[nodiscard]] Future<T> future() const noexcept
    {
        // Happens if object used after move
        assertThat(m_state, "Promise is not usable");
        return Future<T>{*this};
    }

    void operator()() const noexcept
    {
        fulfill();
    }

private:
    friend class Future<void>;
    using SharedState = detail::AsyncSharedState<std::monostate>;

private:
    std::shared_ptr<SharedState> m_state = std::make_shared<SharedState>();
};

NF_END_NAMESPACE
