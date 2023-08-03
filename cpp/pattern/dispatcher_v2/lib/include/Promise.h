

#ifndef PROMISE_HPP
#define PROMISE_HPP


#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <memory>
#include "Dispatcher.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/hana.hpp>
#include <variant>
#include <type_traits>

#include <iostream>
#include <string>
using namespace std;

//namespace TPromise {

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


template<typename tValue>
class ContinuationBase 
{
public:
    
    void process(tValue&& value) {
        handle(std::move(value));
    }

    virtual void handle(tValue&& value) = 0;
};

template <typename tValue, typename F, typename tReturn>
class Continuation : public ContinuationBase<tValue>
{
public:
	Continuation(F&& func, std::shared_ptr<Dispatcher>& dispatch_) 
    :dispatch(dispatch_)
    {
        m_Invocation = std::make_shared<Invocation<F, tReturn, tValue>>(std::move(func));
	}

    Promise<tReturn> getNextPromise() {
        return m_Invocation->getNextPromise();
    }

    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void handle(tValue&& value) {
        dispatch->deliverTask([this,value]{ops(*m_Invocation.get(),value);});
    }

public:
    std::shared_ptr<Invocation<F, tReturn, tValue>> m_Invocation;
    std::shared_ptr<Dispatcher> dispatch;
};


template <typename tValue>
class State 
{
public:
   
    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void setValue(tValue &&value)
    {
        m_value = std::move(value);
		if (m_continuation) {
            m_continuation->process(std::move(*m_value));
         
		} else cout<<"m_continuation is empty\n";
		
    }

    void setContinuation(std::shared_ptr<Dispatcher>& dispatch_, std::shared_ptr<ContinuationBase<tValue>>&& continuation)
    {
        m_dispatcher = dispatch_;        
        m_continuation = std::move(continuation);
        
    }

private: 
    std::optional<tValue> m_value;
	std::shared_ptr<ContinuationBase<tValue>> m_continuation;
    std::shared_ptr<Dispatcher> m_dispatcher;
};


template <typename tValue>
class Promise
{

public:
    void set_value(tValue value) 
    {
        m_state->setValue(std::move(value));
    }

    void set_value() {
        //cout<<"promise set void argument\n";
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
        //cout<<"why call me\n";
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
       // cout<<"promise set void argument\n";
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

