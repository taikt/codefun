#include <iostream>
#include <string>
#include <functional>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
using namespace std;

// g++ binder_template2.cpp -std=c++11
// or g++ binder_template2.cpp -lboost_system

class X
{
public:
    bool f(int a) {
        cout<<"[f] a="<<a<<"\n";
    }

    void f2(int a,int b) {
        cout<<"[f2] a="<<a<<" ,b="<<b<<"\n";
    }
};

template<typename Handler>
class WrapperHandler {
public:
    WrapperHandler(Handler h)
        : handler_(h) {}

    template<typename Arg1>
    void operator()(Arg1 arg1) {
        handler_(arg1);
    }

    template<typename Arg1, typename Arg2>
    void operator()(Arg1 arg1, Arg2 arg2) {
        handler_(arg1,arg2);
    }
private:
    Handler handler_;
};

template<typename Handler>
inline WrapperHandler<Handler> makeWrapperHandler(Handler h) {
    return WrapperHandler<Handler>(h);
}

template<typename Handler>
void testFunc(Handler handler) {
    handler(12);
}

int main() {
    X x;
    //int i = 5;
    //testFunc(bind(&X::f, &x, _1));

    // call as a functor
    makeWrapperHandler(std::bind(&X::f, &x, std::placeholders::_1))(5);
    makeWrapperHandler(std::bind(&X::f2, &x, std::placeholders::_1,std::placeholders::_2))(9,10);

    return 0;
}

