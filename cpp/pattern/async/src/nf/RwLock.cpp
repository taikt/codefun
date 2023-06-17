/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/RwLock.h"

#include <nf/Executor.h>
#include <nf/Logging.h>
#include <nf/RaiiToken.h>

using namespace nf;

#define RWLOCK_DEBUG(__msg) RWLOCK_DEBUG_F(__msg, NULL)
#define RWLOCK_DEBUG_F(__msgFormat, ...) nf::info("{}: " __msgFormat, *this, __VA_ARGS__)

NF_BEGIN_NAMESPACE

namespace detail {

struct RwLockWaiter
{
    bool isReader{true};
    RwLock::OnLockAcquired cb;
};

std::ostream &operator<<(std::ostream &os, const RwLockWaiter &waiter) noexcept
{
    return os << (waiter.isReader ? "Reader" : "Writer") << "@" << &waiter;
}

} // namespace detail

std::ostream &operator<<(std::ostream &os, const RwLock &rwLock) noexcept
{
    os << "RwLock(";
    if (rwLock.m_hasWriter) {
        os << "W)";
    } else {
        os << rwLock.m_cntReaders << ")";
    }
    return os << "@" << &rwLock;
}

NF_END_NAMESPACE

RwLock::RwLock() noexcept
{
    RWLOCK_DEBUG("Construct");
}

RwLock::~RwLock()
{
    RWLOCK_DEBUG("Destroy");
    std::unique_lock lock(m_mutex);
    m_waiters.clear();
}

RaiiToken RwLock::lockRead(OnLockAcquired cb) noexcept
{
    return addLock(true, cb);
}

RaiiToken RwLock::lockWrite(OnLockAcquired cb) noexcept
{
    return addLock(false, cb);
}

void RwLock::wakeUp(Waiter &waiter) noexcept
{
    if (waiter.isReader) {
        m_cntReaders++;
    } else {
        m_hasWriter = true;
    }
    RWLOCK_DEBUG_F("Schedule {}", waiter);
    waiter.cb();
    waiter.cb = nullptr;
}

RaiiToken RwLock::addLock(bool isReader, OnLockAcquired &cb) noexcept
{
    auto waiter = std::make_shared<Waiter>();
    waiter->isReader = isReader;
    waiter->cb = [cb = std::move(cb), executor = Executor::thisThread(),
                  weakWaiter = std::weak_ptr(waiter)]() mutable {
        executor->post([cb = std::move(cb), weakWaiter = std::move(weakWaiter)]() mutable {
            if (auto waiter = weakWaiter.lock()) {
                cb();
            }
        });
    };

    RWLOCK_DEBUG_F("Add {}", *waiter);

    std::unique_lock lock(m_mutex);
    m_waiters.push_back(waiter);

    if (!m_hasWriter && (isReader || m_cntReaders == 0)) {
        wakeUp(*waiter);
    }

    return RaiiToken::nonDismissible(
        m_context.bind([this, it = std::prev(m_waiters.end())] { removeLock(it); }));
}

void RwLock::removeLock(Waiters::iterator it) noexcept
{
    const auto *waiter = it->get();
    const bool isWokenUp = waiter->cb == nullptr;
    const bool isReader = waiter->isReader;

    std::unique_lock lock(m_mutex);

    RWLOCK_DEBUG_F("Remove {}", *waiter);
    m_waiters.erase(it);

    if (isWokenUp) {
        if (isReader) {
            if (--m_cntReaders == 0) {
                wakeUpNext();
            }
        } else {
            m_hasWriter = false;
            wakeUpNext();
        }
    }
}

void RwLock::wakeUpNext() noexcept
{
    if (m_waiters.empty()) {
        RWLOCK_DEBUG("Exhausted");
        return;
    }

    RWLOCK_DEBUG("About to wake up more waiters");
    auto &waiter = m_waiters.front();
    if (waiter->isReader) {
        for (auto &waiter : m_waiters) {
            if (waiter->isReader) {
                wakeUp(*waiter);
            }
        }
    } else {
        wakeUp(*waiter);
    }
}
