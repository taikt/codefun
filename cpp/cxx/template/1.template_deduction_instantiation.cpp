#include <iostream>
using namespace std;
/*
int max(int x, int y) {
    //if (x>y) return x;
    //else return y;
    return (x>y)?x:y;
}
*/

template<typename T>
T Max(T x, T y) {
    return (x>y)?x:y;
}

int main() {
    auto n = Max(1,4);
    cout<<n<<"\n";
    cout<<Max(1,static_cast<int>(4.5))<<"\n";
    cout<<Max(static_cast<float>(1),4.5f)<<"\n";
    return 0;
}
