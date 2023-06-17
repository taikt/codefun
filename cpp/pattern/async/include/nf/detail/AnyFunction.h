/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <memory>
#include <tuple>

NF_BEGIN_NAMESPACE

namespace detail {

// NOTE: This class works as a handle, e.g. it is copyable and every copy refers to the same
// function. This enables the use of non-copyable callables for multiple invocations.
class AnyFunction final
{
public:
    template <typename tFunc, typename = std::enable_if_t<!std::is_same_v<tFunc, AnyFunction>>>
    explicit AnyFunction(tFunc &&f) noexcept
        : m_f(std::make_shared<Wrapper<tFunc>>(std::forward<tFunc>(f)))
    {
    }

    template <typename tFunc, typename... tArgs>
    auto invoke(tArgs &&...args) const
    {
        return func<tFunc>()(std::forward<tArgs>(args)...);
    }

    template <typename tFunc, typename... tArgs>
    auto apply(std::tuple<tArgs...> &&args) const
    {
        return std::apply(func<tFunc>(), std::move(args));
    }

    AnyFunction(const AnyFunction &) = default;
    AnyFunction(AnyFunction &&) noexcept = default;
    AnyFunction &operator=(const AnyFunction &) = default;
    AnyFunction &operator=(AnyFunction &&) noexcept = default;

private:
    struct Base
    {
    };

    template <typename tFunc>
    struct Wrapper : public Base
    {
        template <typename xFunc, typename = std::enable_if_t<!std::is_same_v<xFunc, Wrapper>>>
        explicit Wrapper(xFunc &&f) noexcept
            : f(std::forward<xFunc>(f))
        {
        }
        std::decay_t<tFunc> f;
    };

private:
    template <typename tFunc>
    auto &func() const
    {
        return static_cast<Wrapper<tFunc> *>(m_f.get())->f;
    }

private:
    std::shared_ptr<Base> m_f;
};

} // namespace detail

NF_END_NAMESPACE
