/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#pragma once

#include <nf/Global.h>

#include <boost/core/noncopyable.hpp>

#include <functional>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_ExecutionFlow
 * @brief The RAII token.
 *
 * This class represents a RAII token which holds an action to release some resource or to reverse
 * some changes if anything breaks (i.e exception is thrown). When the object goes out of scope or
 * is explicitly \ref reset(), the action is executed. The \ref dismiss() method can be called to
 * prevent the action execution. The token can be returned from a function where the resource is
 * acquired or can be used in place where something can fail and the reversal is needed. This class
 * can be used to tackle a wide array of problems, with the most common one described in the
 * following section.
 *
 * @par Release the resources at scope end.
 *
 * There could be a method that allocates some resources it returns a token. The token is destroyed
 * on the scope end, releasing the resources.
 *
 * @code
 * {
 *     auto allocate = [&](auto &handle) {
 *         handle = 1;
 *         return RaiiToken::nonDismissible([&] { handle = 0; });
 *     };
 *
 *     int handle = 0;
 *     auto token = allocate(handle);
 * }
 * //The handle will be released and set to 0 at the scope end.
 * @endcode
 *
 * @par Reverse the changes on failure.
 *
 * Assume we must execute two actions, and in case the second one throws, we must revert the first
 * action. To handle this properly, we could use try-catch block:
 *
 * @code
 * firstAction();
 * try {
 *     secondAction();
 * } catch (...) {
 *     revertFirstAction();
 * }
 * @endcode
 *
 * The problem with this approach is that it is clumsy and does not scale well in case of more
 * complicated examples. For example, imagine having a third operation here.
 *
 * A solution to this problem is to have a class that can encapsulate the reversal of the first
 * action from the previous example. This reversal will be executed automatically when the guard
 * object goes @e out-of-scope if an exception is thrown. In case the exception is not thrown, the
 * reversal action can be canceled by invoking @ref dismiss() on the guard object.
 *
 * @code
 * auto token = RaiiToken::dismissible([]() noexcept { revertFirstAction(); });
 * firstAction();
 * secondAction();
 * token.dismiss();
 * @endcode
 *
 * @par Dismissibility of token
 *
 * The dismissibility already appeared in above examples. The difference between the dismmisible and
 * non-dismissible token is that the first's action excution can be avoided with a call of @ref
 * dismiss().
 *
 * The non-dismisible is handy in situations when the action shall be always executed (i.e.
 * resource allocation and release), regardless what happened. When the scope exits the resources
 * shall be always released.
 *
 * The dismissible token has to be used in scenarios when the actions are risky and could cause
 * premature scope exit thus causing the action in the token to be executed. Caution is needed when
 * using move-assign operator, because the action has to be dismissed firstly before assigment.
 *
 * @par Dismiss before move-assign operation.
 * @code
 * DataBaseTransaction transaction = db.createTransaction();
 * try {
 *     transaction.create(createData);
 *     auto token = RaiiToken::dismissible(
 *                       [&transaction]() { transaction.rollback(); });
 *
 *     transaction.insert(insertData);
 *     transaction.set(setData[0]);
 *
 *     token.dismiss(); //dismiss to avoid execution of functor on move-assignment
 *
 *     transaction.set(setData[1]);
 *
 *     token = RaiiToken::dismissible([&transaction]() {
 *                       transaction.drop(dropData);
 *                       transaction.rollback(); });
 *
 *     transaction.set(setData[3]);
 *
 *     token.dismiss();
 * } catch (...) {
 *      handleException();
 * }
 * @endcode
 *
 * @since 5.0
 */
class RaiiToken : private boost::noncopyable
{
public:
    /**
     * @brief Construct a RAII token with an empty action.
     */
    RaiiToken() noexcept;

    /**
     * @brief Move-construct a RAII token from another RAII token.
     *
     * This constructs a new token and transfers an ownership of an action and behavior from @p rhs
     * to this token. The @p rhs token becomes empty.
     */
    RaiiToken(RaiiToken &&rhs) noexcept;

    /**
     * @brief Destroy the RAII token.
     *
     * This will invoke @ref reset() to release the acquired resource.
     */
    ~RaiiToken();

    /**
     * @brief Move-assign a RAII token.
     *
     * This function invokes @ref reset() to release the acquired resource and transfers an
     * ownership of an action from @p rhs to this token. The @p rhs token becomes empty.
     */
    RaiiToken &operator=(RaiiToken &&rhs) noexcept;

    /**
     * @brief Reset the RAII token.
     *
     * If this RAII token holds a non-empty action, this function will invoke the action to
     * release the resource. If the action throws an exception, the application will be terminated.
     */
    void reset() noexcept;

    /**
     * @brief Create a dismissible token.
     * @param action An action that is to be executed when RAII token goes out of scope. The action
     * will be not executed if @ref dismiss() would be called firstly.
     * @return @c RaiiToken.
     */
    [[nodiscard]] static RaiiToken dismissible(std::function<void()> &&action) noexcept;

    /**
     * @brief Create a non-dismissible RAII token.
     * @param action An action that is to be executed when RAII token goes out of scope.
     * @return @c RaiiToken.
     */
    [[nodiscard]] static RaiiToken nonDismissible(std::function<void()> &&action) noexcept;

    /**
     * @brief Dismiss the reset of the token.
     *
     * When called it prevents the @ref reset() from being called.
     */
    void dismiss();

private:
    enum class Behavior
    {
        Dismissible,
        NonDismissible
    };
    RaiiToken(std::function<void()> &&action, Behavior) noexcept;

private:
    std::function<void()> m_action;
    Behavior m_behavior{Behavior::NonDismissible};
};

NF_END_NAMESPACE
