#include <bits/stdc++.h>
using namespace std;
// dinh nghia lop co so template: base class

template<typename Derived>
class Printable {
public:
    void print() const{
        // call phuong thuc print cu the cua child class
        static_cast<const Derived*>(this)->print_impl();
    }
};

//child class
class Myclass: public Printable<Myclass> {
public:
    void print_impl() const {
        cout<< "hello my child class"<<endl;
    }
};

int main(){
    Myclass obj;
    obj.print();
    return 0;
}