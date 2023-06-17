/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Demangle.h>

#include <boost/core/noncopyable.hpp>

#include <memory>
#include <set>

NF_BEGIN_NAMESPACE

namespace detail {

struct SingletonEntryBase : boost::noncopyable
{
    virtual ~SingletonEntryBase() = default;
};

template <typename tImpl>
struct SingletonEntry : SingletonEntryBase
{
    template <typename... tArgs>
    explicit SingletonEntry(tArgs &&...args) noexcept
        : instance(std::make_shared<tImpl>(std::forward<tArgs>(args)...))
    {
    }

    std::shared_ptr<tImpl> instance;
};

class SingletonHolderBase : boost::noncopyable
{
protected:
    /**
     * @brief Log an error message if an instance doesn't exist.
     *
     * Log the message with the object name @c objName when
     * SingletonHolder::lock() is called and the object
     * is destroyed already.
     */
    static void logNoInstance(const std::string &objName) noexcept;
};

} // namespace detail

/**
 * @addtogroup nf_core_Singleton
 * @{
 */

/**
 * @brief The singleton registry.
 *
 * This class holds @cppdoc{shared pointers,memory/shared_ptr} to singleton objects and
 * allows to release them on request. This is necessary so singleton objects can be manually
 * deleted before the program execution leaves @c main().
 *
 * @see @ref SingletonHolder
 */
class SingletonRegistry : boost::noncopyable
{
public:
    using Entry = std::unique_ptr<detail::SingletonEntryBase>;
    SingletonRegistry() = delete;

public:
    /**
     * @brief Construct a singleton instance.
     *
     * The method constructs a shared instance of @c tImpl in the registry and returns it.
     */
    template <typename tImpl, typename... tArgs>
    static std::shared_ptr<tImpl> construct(tArgs &&...args) noexcept
    {
        auto entry = std::make_unique<detail::SingletonEntry<tImpl>>(std::forward<tArgs>(args)...);
        auto instance = entry->instance;
        insert(std::move(entry));
        return instance;
    }

    /**
     * @brief Release all stored singleton instances.
     *
     * This method clears the registry and releases all stored shared references to singleton
     * instances. This might result in destroying singleton instances. In this case @ref
     * SingletonHolder::lock() will start returning @e nullptr.
     */
    static void releaseAll() noexcept;

private:
    static void insert(std::unique_ptr<detail::SingletonEntryBase> &&entry) noexcept;
};

/**
 * @brief The singleton holder.
 *
 * This is a helper class for implementing @c instance() methods for singletons:
 * @code
 * std::shared_ptr<MyClass> MyClass::instance() noexcept
 * {
 *     static nf::SingletonHolder<MyClass> s_instance(someParameter, MyDependency::instance());
 *     return s_instance.lock();
 * }
 * @endcode
 *
 * @see @ref SingletonRegistry
 */
template <typename tImpl>
class SingletonHolder : public detail::SingletonHolderBase
{
public:
    /**
     * @brief Construct a singleton holder.
     *
     * This method constructs a shared instance of @c tImpl using the provided arguments
     * in the @ref SingletonRegistry "singleton registry" and holds a @cppdoc{weak reference,
     * memory/weak_ptr} to that instance.
     */
    template <typename... tArgs>
    explicit SingletonHolder(tArgs &&...args) noexcept
        : m_instance(SingletonRegistry::construct<tImpl>(std::forward<tArgs>(args)...))
    {
    }

    /**
     * @brief Lock a weak reference to the held singleton instance.
     *
     * This method returns a shared pointer to the held singleton instance, if it still
     * exists. Otherwise, it returns @e nullptr.
     *
     * @see @ref SingletonRegistry::releaseAll()
     */
    std::shared_ptr<tImpl> lock() const noexcept
    {
        auto instance = m_instance.lock();
        if (!instance) {
            this->logNoInstance(nf::demangle<tImpl>());
        }
        return instance;
    }

private:
    std::weak_ptr<tImpl> m_instance;
};

/// @}

NF_END_NAMESPACE
