#include <iostream>
#include <string>
#include <functional>
using namespace std;
#define MEM_FUNC_BINDING 0

#if MEM_FUNC_BINDING
// binding member function

class Acceptor
{
public:
    Acceptor() {}
    void handlerAcceptor(int x)
    {
        cout << "[binding member] handlerAcceptor called:" << x << "\n";
    }
};

class Binder
{
public:
    Binder(void (Acceptor::*fn)(int), Acceptor *v, int x) : fn_(fn), v1(v), x_(x) {}
    void operator()()
    {
        (v1->*fn_)(x_);
    }

public:
    void (Acceptor::*fn_)(int);
    int x_;
    Acceptor *v1;
};

int main()
{
    Acceptor ac;
    Binder bd(&Acceptor::handlerAcceptor, &ac, 10);
    bd();
}



#else

// binding non-member function
void handlerAcceptor(int x)
{
    cout << "[binding non-member] handlerAcceptor called:" << x << "\n";
}

class Binder
{
public:
    Binder(void (*fn)(int), int x) : fn_(fn), x_(x) {}
    void operator()()
    {
        fn_(x_);
    }

public:
    void (*fn_)(int);
    int x_;
};

int main()
{
    Binder bd(&handlerAcceptor, 11);
    bd();
}

#endif
