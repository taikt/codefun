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

class Dispatcher;

using namespace std;

template <typename tReturn>
class Promise;

template <typename tValue>
class Future;

template<typename F, typename tReturn, typename... tArgument>
class Invocation 
{
public:
    Invocation(F&& func):
		m_func(std::move(func)) {
        
            m_promise = std::make_shared<Promise<tReturn>>();
            //cout<<"create promise object: type is "<<typeid(m_promise.get()).name()<<"\n";
            //auto test = m_promise->get_future();
            cout<<"create promise object done\n";
        }

    auto get_future() {
        cout << "Invocation: get future\n";
        Future<tReturn> m_future;
        //return m_promise.get_future();
        if (m_promise != nullptr) {
            m_future = m_promise->get_future();
        }
        else cout<<"m_promise is null\n";

        cout << "return future\n";
        return m_future;
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
        //m_promise.set_value(m_func(std::move(value)...));
        cout<<"invocation: call me now\n";
        if constexpr (argumentsVoid() && std::is_void_v<tReturn>) {
            cout<<"arugment is void\n";
            m_func();
            m_promise->set_value();
        } else if constexpr (argumentsVoid() && !std::is_void_v<tReturn>) {
            m_promise->set_value(m_func());
        } else if constexpr (!argumentsVoid() && std::is_void_v<tReturn>) {
            m_func(std::move(value)...);
            m_promise->set_value();
        } else {
            m_promise->set_value(m_func(std::move(value)...));
        }
    }

public:
    //Promise<tReturn> m_promise;
    std::shared_ptr<Promise<tReturn>> m_promise;
    std::decay_t<F> m_func;
};

/*
template<typename tValue>
class ContinuationBase 
{
public:
    
    void process(tValue&& value) {
        handle(std::move(value));
    }

    virtual void handle(tValue&& value) = 0;
};
*/
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
    
    //template <typename F>
	Continuation(F&& func)
    //:m_invoker(std::move(func))
    {
        m_invoker = std::make_shared<Invocation<F, tReturn, tValue>>(std::move(func));
        cout<<"invocation address1:"<<m_invoker<<"\n";
	}

    //auto get_future() {
    Future<tReturn> get_future() {
        cout<<"get future from continuation\n";
        //if (m_invoker == nullptr) {
        //    cout<<"invocation is null\n";
        //}
        //cout<<"invocation address2:"<<m_invoker<<"\n";
        return m_invoker->get_future();
       
        //return Future<tReturn>();
    }

    void handle(tValue&& value) {
        (*m_invoker.get())(value);
    }

public:
    std::shared_ptr<Invocation<F, tReturn, tValue>> m_invoker;
};



template <typename tValue>
class SharedState 
{
public:
    //using tReturn = typename std::conditional_t<std::is_void_v<tValue>, std::invoke_result<F>,
    //                                                  std::invoke_result<F, tValue>>::type;
    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void setValue(tValue &&value)
    {
        m_value = std::move(value);
		if (m_continuation) {
			//m_continuation->f(*value);
            //if (m_dispatcher != nullptr)
                //m_dispatcher->deliverTask([this, value]{ m_continuation->f(value);});

            //ops(m_continuation->f, value);
            //if (m_dispatcher != nullptr)
            //    m_dispatcher->deliverTask([this, value]{ops(m_continuation->f, value);});
            m_continuation->process(std::move(*m_value));
            //m_dispatcher->deliverTask([this]{ops(m_continuation->process, std::move(*m_value));});
		} else cout<<"m_continuation is empty\n";
		
    }

	//template <typename F>
    void setContinuation(std::shared_ptr<Dispatcher>& dispatch_, std::shared_ptr<ContinuationBase<tValue>>&& continuation)
    {
        //f = std::forward<F>(func);
        m_dispatcher = dispatch_;
        
        m_continuation = std::move(continuation);

        
        cout<< "set continuation success\n";
    }

  

private:
 
    //tValue m_value;
    std::optional<tValue> m_value;

	std::shared_ptr<ContinuationBase<tValue>> m_continuation;
    std::shared_ptr<Dispatcher> m_dispatcher;
};

//template <typename tValue>
//class Promise;

template <typename tValue>
class Future
{
    
public:
    
    Future() {
    }
   	Future(Promise<tValue> &p)
        : m_state(p.m_state)
    {
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

        //using tReturn = typename std::conditional_t<std::is_void_v<tValue>, std::invoke_result<F>,std::invoke_result<F, tValue>>::type;
        using tReturn = typename std::invoke_result<F,tValue>::type;
        //using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;

        std::shared_ptr<Continuation<tValue,F,tReturn>> m_continuation = std::make_shared<Continuation<tValue,F,tReturn>>(std::move(func));
        Future<tReturn> future = m_continuation->get_future();
        cout << "return future\n";

        m_state->setContinuation(dispatch_, std::move(m_continuation));
        //auto future = m_continuation->get_future();
        
        return future;
        
    }
 
protected:
	friend Promise<tValue>;
    std::shared_ptr<SharedState<tValue>> m_state;
};

template<>
class Future<void>
{
public:
    Future() {}
    Future(Promise<void> &p)
    {}
};


template <typename tValue>
class Promise
{

public:
    void set_value(tValue value) 
    {
        cout<<"promise set value\n";
        m_state->setValue(std::move(value));
    }

    void set_value() {
        cout<<"promise set void argument\n";
    }

	Future<tValue> get_future() 
    {
        return Future<tValue>{*this};
    }

	Promise() 
    : m_state(std::make_shared<SharedState<tValue>>())
	{
	}

    void operator()(tValue value) 
    {
        cout<<"why call me\n";
        set_value(std::move(value));
    }

private:
    friend class Future<tValue>;

private:
    std::shared_ptr<SharedState<tValue>> m_state;
};


template <>
class Promise<void>
{

public:
   
    void set_value() {
        cout<<"promise set void argument\n";
    }

	Future<void> get_future() 
    {
        return Future<void>{*this};
    }

	Promise() 
    : m_state(std::make_shared<SharedState<std::monostate>>())
	{
	}

    void operator()() 
    {
        cout<<"why call me\n";
        set_value();
    }

private:
    friend class Future<void>;

private:
    std::shared_ptr<SharedState<std::monostate>> m_state;
};
#endif