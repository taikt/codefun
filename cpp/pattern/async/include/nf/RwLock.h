/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Context.h>
#include <nf/RaiiToken.h>

#include <functional>
#include <list>
#include <mutex>

NF_BEGIN_NAMESPACE

namespace detail {
struct RwLockWaiter;
} // namespace detail

/**
 * @ingroup nf_core_Sync
 * @brief The readers-writer lock.
 *
 * This is an implementation of an asynchronous, event loop based readers-writer lock. It
 * allows to have multiple readers at the same time, while a writer requires an exclusive
 * access.
 *
 * @par Thread-safety
 *
 * All methods are thread-safe.
 *
 * @since 3.21, 4.1
 */
class RwLock
{
public:
    using OnLockAcquired = std::function<void()>;

public:
    /**
     * @brief Constructor.
     *
     * Construct an unlocked RW lock.
     */
    RwLock() noexcept;

    /**
     * @brief Destructor.
     *
     * Destroy the lock. All acquired locks are released, all pending locks are discarded.
     */
    ~RwLock();

public:
    /**
     * @brief Acquire a read-lock.
     *
     * This function starts acquiring a read-lock. It will asynchronously invoke the provided
     * @p cb in the context of the calling thread when the lock is acquired. The lock will
     * remain acquired until the returned RAII token is destroyed. If the RAII token is
     * destroyed before the callback is invoked, it will not be invoked.
     *
     * The read-lock can be acquired only if there is no write-lock acquired.
     */
    [[nodiscard]] RaiiToken lockRead(OnLockAcquired cb) noexcept;

    /**
     * @brief Acquire a write-lock.
     *
     * This function starts acquiring a write-lock. It will asynchronously invoke the provided
     * @p cb in the context of the calling thread when the lock is acquired. The lock will
     * remain acquired until the returned RAII token is destroyed. If the RAII token is
     * destroyed before the callback is invoked, it will not be invoked.
     *
     * The write-lock can be acquired only if there is no other write-lock or read-lock acquired.
     */
    [[nodiscard]] RaiiToken lockWrite(OnLockAcquired cb) noexcept;

private:
    using Waiter = detail::RwLockWaiter;
    using Waiters = std::list<std::shared_ptr<Waiter>>;

private:
    friend std::ostream &operator<<(std::ostream &os, const RwLock &rwLock) noexcept;
    void wakeUp(Waiter &waiter) noexcept;
    RaiiToken addLock(bool isReader, OnLockAcquired &cb) noexcept;
    void removeLock(Waiters::iterator it) noexcept;
    void wakeUpNext() noexcept;

private:
    Context m_context;
    std::mutex m_mutex;
    Waiters m_waiters;
    int m_cntReaders{0};
    bool m_hasWriter{false};
};

NF_END_NAMESPACE
