#include <iostream>
#include <string>
#include <functional>
#include <boost/function.hpp>
#include <boost/bind.hpp>
using namespace std;

// https://www.boost.org/doc/libs/1_42_0/libs/bind/bind.html#with_member_pointers
// g++ binder_template.cpp -std=c++11
// or g++ binder_template2.cpp -lboost_system

class X
{
public:
    bool f(int a) {
        cout<<"a="<<a<<"\n";
    }
};

template<typename Handler>
void testFunc(Handler handler) {
    handler(18);
}

int main() {
    X x;

    //shared_ptr<X> p(new X);

    int i = 5;

    //boost::bind(&X::f, ref(x), _1)(i);		// x.f(i)
    //boost::bind(&X::f, &x, _1)(i);			//(&x)->f(i)
    //boost::bind(&X::f, x, _1)(i);			// (internal copy of x).f(i)
    //bind(&X::f, p, _1)(i);			// (internal copy of p)->f(i)

    // can use boost::bind or std::bind, both are fine
    //testFunc(boost::bind(&X::f, &x, _1));
    testFunc(std::bind(&X::f, &x, std::placeholders::_1));

    return 0;
}

