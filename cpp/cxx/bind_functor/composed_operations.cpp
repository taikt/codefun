#include <iostream>
#include <string>
#include <functional>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
using namespace std;

// g++ binder_template2.cpp -std=c++11
// or g++ binder_template2.cpp -lboost_system
class Session
{
public:
    void accepterHandler(int err) {
        cout<<"[f] err="<<err<<"\n";
    }

    void f2(int a,int b) {
        cout<<"[f2] a="<<a<<" ,b="<<b<<"\n";
    }
};



template<typename Handler>
class acceptOperation {
public:
    acceptOperation(Handler h)
        :handler_(h)
    {}

    void perform(int& err) {
        cout<<"perform\n";
    }

    void complete(int err) {
        cout<<"complete, callback handler\n";
        this->handler_(err);
    }
private:
    Handler handler_;
};


class Reactor {
public:
    Reactor() {}

    template<typename Operation>
    void startReadOp(Operation op) {
        op.complete(3);
    }
};

class Acceptor {
public:
    Acceptor(Reactor react)
        :react_(react)
        {}

    template<typename Handler>
    void asyncAccept(Handler h) {
        react_.startReadOp(acceptOperation<Handler>(h));
    }

private:
    Reactor react_;
};


int main() {
    Reactor reactor_;
    Session sess_;
    Acceptor acc(reactor_);
    acc.asyncAccept(std::bind(&Session::accepterHandler, &sess_, std::placeholders::_1));

    return 0;
}

