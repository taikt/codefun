
#ifndef PROMISE_HPP
#define PROMISE_HPP

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
#include "State.h"

using namespace std;


template <typename tValue>
class Promise
{

public:
    void set_value(tValue value) 
    {
        m_state->setValue(std::move(value));
    }

    void set_value() {
        cout<<"promise set void argument\n";
    }

    template <typename F>
    auto then(std::shared_ptr<Dispatcher>& dispatch_, F func) 
    {   
        
        if constexpr (std::is_void_v<tValue>) {
            static_assert(std::is_invocable_v<F>, "Continuation must be invocable with 'void'");
        } else {
            static_assert(std::is_invocable_v<F, tValue>,
                          "Continuation must be invocable with 'TValue'");
        }
        
        using tReturn = typename std::invoke_result<F,tValue>::type;

        std::shared_ptr<Continuation<tValue,F,tReturn>> m_continuation = std::make_shared<Continuation<tValue,F,tReturn>>(std::move(func),dispatch_);

        Promise<tReturn> next_promise = m_continuation->getNextPromise();
     
        m_state->setContinuation(dispatch_, std::move(m_continuation));
      
        return next_promise;
        
    }


	Promise() 
    : m_state(std::make_shared<State<tValue>>())
	{
	}

    void operator()(tValue value) 
    {
        set_value(std::move(value));
    }

private:
    std::shared_ptr<State<tValue>> m_state;
};


template <>
class Promise<void>
{

public:
   
    void set_value() {
       cout<<"promise set void argument\n";
    }

	Promise() 
    : m_state(std::make_shared<State<std::monostate>>())
	{
	}

    void operator()() 
    {
        set_value();
    }

private:
    std::shared_ptr<State<std::monostate>> m_state;
};

#endif

