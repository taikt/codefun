#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <memory>
#include <unistd.h>

#include <bits/stdc++.h>


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
	std::function<void (tValue)> f;
};

template <typename tValue>
class SharedState 
{
public:
    
    void setValue(tValue &&value)
    {
        m_value = std::move(value);
		if (m_continuation) {
			m_continuation->f(value);
		} else cout<<"m_continuation is empty\n";
		
    }

	template <typename F>
    void setContinuation(F&& func)
    {
        //f = std::forward<F>(func);
        m_continuation = std::make_shared<Continuation<tValue>>(func);
    }

private:
 
    tValue m_value;
	//std::function<void (tValue)> f;
	std::shared_ptr<Continuation<tValue>> m_continuation;
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
    void then(F func) 
    {       
        m_state->setContinuation(std::move(func));
        
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

void doAsyncJob(std::shared_ptr<Promise<int>> &promise) {
	sleep(2);
	promise->set_value(5);
}

std::shared_ptr<Promise<int>> mypromise;

Future<int> startRequest() {
	auto m_promise = std::make_shared<Promise<int>>();
	mypromise = m_promise;
	// send this job to a queue
	// doAsyncJob(m_promise);
	// note: future object must return before promise set value
	
	return m_promise->get_future();
}



int main() {
	// auto m_promise = std::make_shared<Promise<int>>();
	//m_promise->get_future().then([](int x) {cout << x<<"\n";});
	//m_promise->set_value(2);
		
	startRequest().then([](int x) {cout << x<<"\n";});
	doAsyncJob(mypromise);
}


