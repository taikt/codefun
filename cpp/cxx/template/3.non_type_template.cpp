#include <iostream>
#include <typeinfo>
#include <cstring>
using namespace std;

// pass non-type parameter voi constant value
// int size la non-type template argument
template<int size>
void Print() {
    cout<<size<<"\n";
}

template<typename T>
T Sum(T *arr, int size) { // int size: o day khong phai la non-type template argument
    T sum{};
    for (int i=0;i<size;i++) {
        sum += arr[i];
    }
    return sum;
}

template<typename T, int size> // int size: day la non-type template argument
T Sum2(T (&arr)[size]) { // pass vao array reference
    T sum{};
    for (int i=0;i<size;i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    int i=6;
    Print<3>(); // Print<i>() -> wrong, i khong phai constant

    int arr[]{2,3,5,1};
    cout<<Sum(arr,4)<<"\n"; // pass size of array

    // int(&ref)[4] = arr //ref la array reference cua arr
    cout<<Sum2(arr)<<"\n"; // khong can pass size of array
    return 0;
}
