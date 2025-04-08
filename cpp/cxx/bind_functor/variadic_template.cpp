#include <iostream>
#include <string>
using namespace std;

/*
template<typename... ArgTypes> 
void f_(ArgTypes... Args)
{
   f_(Args)...;
}
*/

template<typename Func, typename... Args>
int ops(Func&& f, Args&&... args) {
    return std::forward<Func>(f)(std::forward<Args>(args)...);
}

template<typename tFunc, typename... tArg>
class Invocator {
public:
    Invocator(tFunc&& func)
    :m_func(std::move(func)) {}

    void operator()(tArg... value) {
        m_func(std::move(value)...);
    }

public:
    std::decay_t<tFunc> m_func;
};



int fuction_(int a, int b) {
    cout<<"a="<<a<<", b="<<b<<"\n";
    return a+b;
}

int main() {
    //Invocator m_invok;
    //Invocator<fFunc> m_invok(fuction_);

    cout<<"sumz:"<<ops(fuction_, 1,2)<<"\n";

    return 0;
}

