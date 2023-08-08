#ifndef STATE_HPP
#define STATE_HPP


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
#include "Invocator.h"

using namespace std;

template <typename tValue>
class State 
{
public:
    State() {
		//cout<<"creat shared state object\n";
	}
   	~State(){
		//cout<<"shared state is destroy\n";
	}

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

#endif
