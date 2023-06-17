/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <memory>

NF_BEGIN_NAMESPACE
namespace detail {

class CallbackScope
{
public:
    CallbackScope() noexcept;
    ~CallbackScope();

    CallbackScope(const CallbackScope &) = delete;
    CallbackScope &operator=(const CallbackScope &) = delete;
    CallbackScope(CallbackScope &&) = default;
    CallbackScope &operator=(CallbackScope &&) = default;

public:
    template <typename tCallable>
    auto bind(tCallable &&callable) const
    {
        std::weak_ptr weakGuard{m_guard};
        return [callable = std::forward<tCallable>(callable), weakGuard](auto &&...args) mutable {
            auto guard = weakGuard.lock();
            if (guard) {
                return callable(std::forward<decltype(args)>(args)...);
            }

            /* If the guard has been deleted and the callback returns a non-void, we
             * will return a default-constructed value of the given type. */
            using ReturnType = decltype(callable(std::forward<decltype(args)>(args)...));
            return ReturnType();
        };
    }

    void reset() noexcept;

private:
    struct Guard
    {
    };
    std::shared_ptr<Guard> m_guard;
};

} // namespace detail
NF_END_NAMESPACE
