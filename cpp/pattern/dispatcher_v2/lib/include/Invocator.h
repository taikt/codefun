/*
* tai2.tran
*/

#ifndef INVOCATOR_HPP
#define INVOCATOR_HPP


#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/hana.hpp>
#include <variant>
#include <type_traits>
#include <iostream>
#include <string>


#include "Dispatcher.h"
#include "Promise.h"
#include "Continuation.h"

using namespace std;


namespace kt {

class Dispatcher;
template <typename tReturn>
class Promise;

template<typename F, typename tReturn, typename... tArgument>
class Invocation 
{
public:
    Invocation(F&& func):
		m_func(std::move(func)) {
            //m_promise = std::make_shared<Promise<tReturn>>();
        }

    auto getNextPromise() {
        return m_promise;
    }
    
    static constexpr bool argumentsVoid()
    {
        constexpr auto kArgumentTypes =
            boost::hana::make_basic_tuple(boost::hana::type_c<tArgument>...);

        if constexpr (boost::hana::is_empty(kArgumentTypes)) {
            return true;
        } else {
            return boost::hana::if_(boost::hana::front(kArgumentTypes)
                                        == boost::hana::type_c<std::monostate>,
                                    true, false);
        }
    }

    void operator()(tArgument... value){
        if constexpr (argumentsVoid() && std::is_void_v<tReturn>) {
            m_func();
            m_promise.set_value();
        } else if constexpr (argumentsVoid() && !std::is_void_v<tReturn>) {
            m_promise.set_value(m_func());
        } else if constexpr (!argumentsVoid() && std::is_void_v<tReturn>) {
            m_func(std::move(value)...);
            m_promise.set_value();
        } else {
            m_promise.set_value(m_func(std::move(value)...));
        }
    }

public:

    Promise<tReturn> m_promise;
    std::decay_t<F> m_func;
};

}

#endif