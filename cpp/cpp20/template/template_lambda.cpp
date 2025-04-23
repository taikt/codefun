// https://grok.com/chat/932d3320-6516-42a7-be76-2163de660623
// sử dụng cú pháp template trong lambda expression, giúp lambda trở nên 
// linh hoạt hơn khi làm việc với các kiểu dữ liệu khác nhau.
#include <iostream>
using namespace std;

int main() {
    auto add_lambda = []<typename T, typename U>(T x, U y){
        return x+y;
    };
    cout<<add_lambda(2,3.14)<<endl;

    return 0;
}