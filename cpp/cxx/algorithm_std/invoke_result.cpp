#include <iostream>
#include <type_traits>
#include <iostream>
#include <string>
using namespace std;
// only for c++17
// g++ invoke_result.cpp -std=c++17

// return type of callable at compile time
 
struct S
{
    int operator()(char, int&);
    //float operator()(int) { return 1.0; }
};

double ff(int a, int b, double c){
    cout <<"testing\n";
    return 0.4;
}

int main() {
    std::invoke_result<S,char,int&>::type b = 3;
    static_assert(std::is_same<decltype(b), int>::value, "");

    // khai bao x kieu double, lay kieu la gia gtri return cua ff 
    std::invoke_result<decltype(ff),int,int,double>::type x = 3.2;
    static_assert(std::is_same<decltype(x), double>::value, "");
    // static_assert(std::is_same<decltype(x), int>::value, ""); => error, kieu la double, not int
    cout<<"done\n";
}