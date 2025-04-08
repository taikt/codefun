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
#include <cassert>

#include <iostream>
#include <string>
using namespace std;

class Dispatcher;
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
			//cout<<"create invocation object\n";
            m_promise = std::make_shared<Promise<tReturn>>();
        }
	~Invocation(){
		//cout<<"Invocation is destroy\n";
	}
	
    auto get_future() noexcept{
        Future<tReturn> m_future;
        //return m_promise.get_future();
        if (m_promise != nullptr) {
            m_future = m_promise->get_future();
        }
        else cout<<"m_promise is null\n";

        //cout << "return next future\n";
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

    void operator()(tArgument... value) noexcept{
        if constexpr (argumentsVoid() && std::is_void_v<tReturn>) {
            //cout<<"promise set value1\n";
            m_func();
            m_promise->set_value();
        } else if constexpr (argumentsVoid() && !std::is_void_v<tReturn>) {
            //cout<<"promise set value2\n";
            m_promise->set_value(m_func());
        } else if constexpr (!argumentsVoid() && std::is_void_v<tReturn>) {
            //cout<<"promise set value3\n";
            m_func(std::move(value)...);
			//cout<<"promise set value3: func call done, next set promise\n";
            m_promise->set_value();
			//cout<<"exit invocation call\n";
        } else { //not argument void and return
            //cout<<"promise set value4\n";
            m_promise->set_value(m_func(std::move(value)...));
        }
    }

public:

    std::shared_ptr<Promise<tReturn>> m_promise;
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
    	//cout<<"creat continatio object\n";
        m_invoker = std::make_shared<Invocation<F, tReturn, tValue>>(std::move(func));
	}

	~Continuation() {
		//cout<<"destroy continatio object\n";
	}

    Future<tReturn> get_future() {
        return m_invoker->get_future();   
    }

    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void handle(tValue&& value) {
        
        /*
        wrong method:
        dispatch->deliverTask(
            [this,value]{
                ops(*m_invoker.get(),value);
            }
        );
        */
        auto fn = std::bind(*m_invoker.get(),value);
        dispatch->deliverTask(fn);
        //(*m_invoker.get())(value);
    }

public:
    std::shared_ptr<Invocation<F, tReturn, tValue>> m_invoker;
    std::shared_ptr<Dispatcher> dispatch;
};



template <typename tValue>
class SharedState 
{
public:
    SharedState() = default;
    SharedState(const SharedState &) = delete;
    SharedState(SharedState &&) = delete;
    SharedState &operator=(const SharedState &) = delete;
    SharedState &operator=(SharedState &&) = delete;

    /*
	SharedState() {
		cout<<"creat shared state object\n";
	}
    */
   	~SharedState(){
		//cout<<"shared state is destroy\n";
	}
    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void setValue(tValue &&value) noexcept
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
    //void setContinuation(std::shared_ptr<Dispatcher>& dispatch_, std::shared_ptr<ContinuationBase<tValue>>&& continuation) 
    void setContinuation(std::shared_ptr<Dispatcher>& dispatch_, std::shared_ptr<ContinuationBase<tValue>> continuation) noexcept // update
    {
        //f = std::forward<F>(func);
        m_dispatcher = dispatch_;        
        m_continuation = continuation; //std::move(continuation); update
        
       // cout<< "set continuation success\n";
    }


private:
 
    //tValue m_value;
    std::optional<tValue> m_value;

	std::shared_ptr<ContinuationBase<tValue>> m_continuation;
    std::shared_ptr<Dispatcher> m_dispatcher;
};


template <typename tValue>
class Future
{ 
public:
    
    Future() {
		//cout<<"create future object no para\n";
    }
	~Future(){
		//cout<<"Future is destroy\n";
	}
   	//Future(Promise<tValue> &p)
	Future(const Promise<tValue>& p) noexcept // change rererenc to copy
        : m_state(p.m_state) // new update
    {
    	//cout<<"create future object with para\n";
    }
	
    template <typename F>
    auto then(std::shared_ptr<Dispatcher>& dispatch_, F func) noexcept
    {   
        
        if constexpr (std::is_void_v<tValue>) {
            static_assert(std::is_invocable_v<F>, "Continuation must be invocable with 'void'");
        } else {
            static_assert(std::is_invocable_v<F, tValue>,
                          "Continuation must be invocable with 'TValue'");
        }
        
        //using tReturn = typename std::conditional_t<std::is_void_v<tValue>, std::invoke_result<F>,std::invoke_result<F, tValue>>::type;
        using tReturn = typename std::invoke_result<F,tValue>::type;

        std::shared_ptr<Continuation<tValue,F,tReturn>> m_continuation = std::make_shared<Continuation<tValue,F,tReturn>>(std::move(func),dispatch_);
        Future<tReturn> future = m_continuation->get_future();
     
        m_state->setContinuation(dispatch_, std::move(m_continuation));

        //cout<<"m_state counter:"<<m_state.use_count()<<"\n";
      
        return future;
        
    }

    Future(tValue value) noexcept
    {
        this->m_state->setValue(std::move(value));
    }
 
protected:
	friend Promise<tValue>;
    std::shared_ptr<SharedState<tValue>> m_state = std::make_shared<SharedState<tValue>>(); //new update

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
static_assert(!std::is_reference_v<tValue>, "Cannot construct Promise for a reference");

public:
    void set_value(tValue value) noexcept
    {
        m_state->setValue(std::move(value));
    }

    void set_value() {
        //cout<<"promise set void argument\n";
    }

	Future<tValue> get_future() const noexcept
    {
        //static_assert(m_state, "Promise is not usable");
        return Future<tValue>{*this};
    }

	Promise() noexcept
    : m_state(std::make_shared<SharedState<tValue>>())
	{
		//cout<<"create a promise object\n";
	}

    ~Promise() {
        //cout<<"distroy promise\n";
    }

    void operator()(tValue value) 
    {
        //cout<<"why call me\n";
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
   
    void set_value() noexcept {
       // cout<<"promise set void argument\n";
    }

	Future<void> get_future() noexcept
    {
        return Future<void>{*this};
    }

	Promise() 
    : m_state(std::make_shared<SharedState<std::monostate>>())
	{
	}

    void operator()() 
    {
        //cout<<"why call me\n";
        set_value();
    }

private:
    friend class Future<void>;

private:
    std::shared_ptr<SharedState<std::monostate>> m_state;
};
#endif