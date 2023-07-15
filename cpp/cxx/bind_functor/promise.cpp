#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
#include <functional>

using namespace std;

struct ResultResponse {
	int id;
	string name;
};

template <typename Value>
class Promise
{
private:
    std::function<void (Value)> f;
  
public:
	/*
    template <typename F>
    Promise(F&& func)
        : f(std::forward<F>(func))
    {}
    */

	template <typename F>
	void then(F&& func) {
		f = std::forward<F>(func);
	}

	//template <typename Value>
    void fullfill(Value val)
    {
        f(val);
    }
};

int main() {
	//Promise<int> m_promise([](int x) {cout << x<<"\n";});


	//Promise<ResultResponse> m_promise2([](ResultResponse r) {cout << "id:" <<r.id<< ", name:"<< r.name<<"\n";});
	Promise<ResultResponse> m_promise2;
	m_promise2.then([](ResultResponse r) {cout << "id:" <<r.id<< ", name:"<< r.name<<"\n";});

	ResultResponse ret;
	ret.id = 2;
	ret.name = "mr Tai";
	

	m_promise2.fullfill(ret);

	/*
	int result{};

    auto promise = Promise<int>{};
    promise.future().then(context, &twice).then(context, [&](int val) { result = val; });

    promise.fulfill(2);
	*/
}

