#include <iostream>
#include <string>
#include <functional>
using namespace std;
// g++ .\binder_with_argument.cpp -std=c++11


#define MEM_FUNC_BINDING 0

#if MEM_FUNC_BINDING

// binding member function with arguments supplied at point of use
class Acceptor
{
public:
    Acceptor() {}
    void handlerAcceptor(int count, int err_code, int length)
    {
        cout << "handlerAcceptor called: count=" << count << ", err_code=" << err_code << ", length=" << length << "\n";
    }
};

class Binder
{
public:
    Binder(void (Acceptor::*fn)(int, int, int), Acceptor *v, int count, const std::_Placeholder<1> &err_code, const std::_Placeholder<2> &length)
        : fn_(fn), v1(v), count_(count) {}
    void operator()(int err_code, int length)
    {
        (v1->*fn_)(count_, err_code, length);
    }

public:
    void (Acceptor::*fn_)(int, int, int);
    int count_;
    Acceptor *v1;
};

// bder is a functor
void testCallback(Binder &bder)
{
    cout << "callback is invoked\n";
    bder(1, 16);
}
int main()
{
    Acceptor ac;
    Binder bd(&Acceptor::handlerAcceptor, &ac, 10, std::placeholders::_1, std::placeholders::_2);
    //bd(1, 12);
    testCallback(bd);
}



#else

// binding non-member function with arguments supplied at point of use
void handlerAcceptor(int count, int err_code, int length)
{
    cout << "handlerAcceptor called: count=" << count << ", err_code=" << err_code << ", length=" << length << "\n";
}

class Binder
{
public:
    Binder(void (*fn)(int, int, int), int count, const std::_Placeholder<1> &err_code, const std::_Placeholder<2> &length) : fn_(fn), count_(count) {}
    void operator()(int err_code, int length)
    {
        fn_(count_, err_code, length);
    }

public:
    void (*fn_)(int, int, int);
    int count_;
};

int main()
{
    Binder bd(&handlerAcceptor, 11, std::placeholders::_1, std::placeholders::_2);
    bd(1, 16);
}


#endif

