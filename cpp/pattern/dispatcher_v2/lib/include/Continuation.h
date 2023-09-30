/*
* tai2.tran
*/

#ifndef CONTINUATION_HPP
#define CONTINUATION_HPP

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

#include "State.h"
#include "Invocator.h"
#include "Dispatcher.h"
#include "Promise.h"
#include "Log.h"

using namespace std;


namespace kt {

class Dispatcher;

template<typename F, typename tReturn, typename... tArgument>
class Invocation;

template <typename tReturn>
class Promise;

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

    ~Continuation() {
		//cout<<"destroy continatio object\n";
	}

    Promise<tReturn> getNextPromise() {
        return m_Invocation->getNextPromise();
    }

    template<typename Func, typename... Args>
    void ops(Func&& f, Args&&... args) {
        return std::forward<Func>(f)(std::forward<Args>(args)...);
    }

    void handle(tValue&& value) {
        auto fn = std::bind(*m_Invocation.get(),value);
        dispatch->deliverTask(fn);
        //dispatch->deliverTask([this,value]{ops(*m_Invocation.get(),value);});
    }

public:
    std::shared_ptr<Invocation<F, tReturn, tValue>> m_Invocation;
    std::shared_ptr<Dispatcher> dispatch;
};

}

#endif
