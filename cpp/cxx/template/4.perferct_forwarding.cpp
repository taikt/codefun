#include <iostream>
#include <typeinfo>
#include <cstring>
using namespace std;

class Integer {
public:

    Integer() {
        cout<<"default constructor\n";
        mInt = new int(0);
    }
    Integer(int x) {
        cout<<"parametered constructor\n";
        mInt = new int(x);
    }

    // copy constructor, se dc chay de deep copy cho object
    // copy cho temporary object neu khong explicit khai bao move constructor
    // copy constructor dc call khi
    // 1. pass object to function by value
    // 2. function return object by value
    // 3. manually create copy of variable i.e Integer i2(i)
    Integer(const Integer& a) {
        cout<<"copy constructor\n"; // copy constructor apply khi khoi tao object moi, i.e Integer a=b;
        mInt = new int(*a.mInt);
    }

    // if user not declare, compiler create default assignment operator using shallow copy, which should be avoided
    Integer& operator=(const Integer& a) {
        cout<<"assignment copy operator\n";
        if (this != &a) {
            // copy assignment duoc goi khi gan object da duoc khoi tao truoc do, nen can delete old memory. i.e
            // Integer a;
            // Integer b;
            // a = b;
            delete mInt;
            mInt = new int(*a.mInt);
        }
        return *this;
    }
    // tu dong move temporary object, su dung 'shawdow copy', khong can deep copy nhu copy constructor
    Integer(Integer&& a) {
        cout<<"move constructor\n";
        mInt = a.mInt; // move constructor apply khi khoi tao object moi, nen khong can delete mInt i.e Interger a = std::move(b);
        a.mInt = nullptr;
    }
    Integer& operator=(Integer&& a) {
        cout<<"move assignment operator\n";
        if (this != &a) {
            // move assignment duoc goi khi gan object da duoc khoi tao truoc do, nen can delete old memory. i.e
            // Integer a;
            // Integer b;
            // a = std:move(b);
            delete mInt;
            mInt = a.mInt;
            a.mInt = nullptr;
        }
        return *this;
    }
public:
    int *mInt;
};

class Employee {
public:
    template<typename T>
    Employee(T&& mId) :
    id{std:forward<T>(mId)} { // preserve mId type: bao toan kieu cua mId khi forward to id
        cout<<"perfect forwarding\n";
    }
public:
    Integer id;
};

int main() {
    /*
    Integer a(5);
    Integer b(std::move(a)); // invoke move constructor
    Integer b= std::move(a); // invoke move constructor
    Integer b(a); // invoke copy constructor
    Integer b=a; // invoke copy constructor
    b = a; // invoke assignment copy
    b = std::move(a); // invoke move assignment
    */
    Integer x(3);
    Employee ee(Integer(2)); // invoke move constructor inside Integer
    //Employee ee(x); // invoke copy constructor inside Integer
    /*
    muc dich perfect forwarding la bao toan reference type
    i.e neu passing L-value (i.e x) => passing L-value to id => invoke copy constructor inside Integer class
    neu passing R-value (i.e Integer(2)) => passing R-value to id => invoke move constructor inside Integer class
    */

    return 0;
}
