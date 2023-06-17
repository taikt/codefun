#include <iostream>
#include <typeinfo>
#include <cstring>
using namespace std;

template<typename T>
T Max(T x, T y) {
    cout<<typeid(T).name()<<"\n";
    return x>y ? x:y;
}
// explicit instantiation
//template char Max(char x, char y);

// explicit specialization
// phai dinh nghia trong cpp file, not header
template<> const char * Max(const char* x, const char* y) {
    return strcmp(x,y)>0 ? x:y; // luu y cach implement khac trong template
}
int main() {
    /*
    char * const a; // constant pointer, pointer value cannot changed but value being pointed can be changed
    const char * a; // pointer point to a constant char. pointer value can be changed but value being pointed cannot be changed.
    */
    const char *b{"B"};
    const char *a{"A"};
    cout<<Max(a,b); // return A, wrong
    // pass vao templete theo dia chi, vi dia chi a lon hon b (a duoc khai bao sau),
    // nen tra lai A => wrong result
    // solution: tao explicit specialization

    return 0;
}
