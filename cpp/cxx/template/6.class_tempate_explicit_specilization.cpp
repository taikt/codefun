#include <iostream>
using namespace std;

template<typename T>
class Printer {
    T *mData;
public:
    Printer(T* data): mData(data)
    {
    }
    Print() {
        cout<<*mData<<"\n";
    }
    T* getData() {
        return mData;
    }

};

int main() {
    int a=3;
    Printer<int> x(&a);
    x.Print();

    return 0;
}
