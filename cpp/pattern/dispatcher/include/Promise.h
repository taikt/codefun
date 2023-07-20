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

class Dispatcher;

using namespace std;

template <typename tValue>
class Continuation 
{
public:
	template <typename F>
	Continuation(F&& func) {
		f = std::forward<F>(func);
	}

public:
	//std::function<void (const tValue&)> f;
    std::function<void (tValue)> f;
};



template <typename tValue>
class SharedState 
{
public:
    
    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void setValue(tValue &&value)
    {
        //m_value = std::move(value);
		if (m_continuation) {
			//m_continuation->f(*value);
            //if (m_dispatcher != nullptr)
                //m_dispatcher->deliverTask([this, value]{ m_continuation->f(value);});

            //ops(m_continuation->f, value);
            if (m_dispatcher != nullptr)
                m_dispatcher->deliverTask([this, value]{ops(m_continuation->f, value);});
		} else cout<<"m_continuation is empty\n";
		
    }

	template <typename F>
    void setContinuation(std::shared_ptr<Dispatcher>& dispatch_, F&& func)
    {
        //f = std::forward<F>(func);
        m_dispatcher = dispatch_;
        m_continuation = std::make_shared<Continuation<tValue>>(func);
    }

private:
 
    //tValue m_value;
    std::optional<tValue> m_value;

	std::shared_ptr<Continuation<tValue>> m_continuation;
    std::shared_ptr<Dispatcher> m_dispatcher;
};

template <typename tValue>
class Promise;

template <typename tValue>
class Future
{
    
public:
   	Future(Promise<tValue> &p)
        : m_state(p.m_state)
    {
    }
	
    template <typename F>
    void then(std::shared_ptr<Dispatcher>& dispatch_, F func) 
    {    
        m_state->setContinuation(dispatch_, std::move(func));
        
    }
 
    
protected:
	friend Promise<tValue>;
    std::shared_ptr<SharedState<tValue>> m_state;
};


template <typename tValue>
class Promise
{

public:
    void set_value(tValue value) 
    {
        m_state->setValue(std::move(value));
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
        set_value(std::move(value));
    }

private:
    friend class Future<tValue>;

private:
    std::shared_ptr<SharedState<tValue>> m_state;
};
#endif