//Ref:
// http://blog.think-async.com/search/label/asio
// https://www.geeksforgeeks.org/functors-in-cpp/
// https://stackoverflow.com/questions/356950/what-are-c-functors-and-their-uses
// https://www.geeksforgeeks.org/bind-function-placeholders-c/
// https://stackoverflow.com/questions/13238050/convert-stdbind-to-function-pointer

#include <iostream>
#include <string>
#include <functional>

//using namespace std::placeholders;


using namespace std;

void func(int a, int b, int c) {
    cout<<"func1\n";
    cout<<"a="<<a<<", b="<<b<<", c="<<c<<"\n";
}

void func2(int a, int b, int c) {
    cout<<"func2\n";
    cout<<"a="<<a<<", b="<<b<<", c="<<c<<"\n";
}

void func3(void(*fn)(int,int,int), int a, int b, int c) {
    fn(a,b,c);
} 

void func4(void(*fb)(int,int,int), int x) {
    fb(x);
}


int main() {
    // a=1, b=2, c=4
    auto fn = bind(func,1,3,4);
    // a=9, b=3, c=4
    auto fn2 = bind(func,std::placeholders::_1,3,4);
    // a=2, b=10, c=3
    auto fn3 = bind(func,2,std::placeholders::_1,3);
    // if fn has 2 parameters, use std::placeholders::_1, std::placeholders::_2
    // fn(a,b)
    // std::placeholders::_1: a, std::placeholders::_2: b

    fn();
    fn2(9);
    fn3(10);
    func3(func,1,2,3);

    func4(fn3,9);

    return 0;
}