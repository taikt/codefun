#include <bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl
using namespace std;

// https://grok.com/chat/3a36052a-bbf0-43e9-a921-295e0fd73051
// suy ra kiểu dữ liệu của một biểu thức tại thời điểm 
// biên dịch (compile-time). Nó rất hữu ích trong lập 
// trình template hoặc khi cần xác định kiểu của một
//  biểu thức mà không cần tính toán giá trị của biểu 
//  thức đó.

template<typename T1, typename T2>
auto add(T1 a, T2 b) -> decltype(a+b) {
    return a+b;
}

int main() {
    int x=5;
    double y=3.5;
    auto result=add(x,y); // decltype(a+b) suy ra kieu douple
    db(result);
    return 0;
}