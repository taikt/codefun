// https://refactoring.guru/design-patterns/factory-method

#include <string>
#include <iostream>
using namespace std;

// khai bao interface common cho cac product
class Product {
    public:
        virtual ~Product() {}
        virtual string Operation() const=0;
};

class Product1: public Product {
    public:
        string Operation() const override {
            return "product1 result";
        }
};

class Product2: public Product {
    public:
        string Operation() const override {
            return "product1";
        }
};

// tao creator class
// class nay chua factory method interface
// subclass imlement this factory method
class Creator {
public:
    virtual ~Creator(){};
    virtual Product* FactoryMethod() const = 0;

    string otherOps() const {
        // call factory method
        Product* p = FactoryMethod();
        // su dung result
        string res="creator:" + p->Operation();
        delete p;
        return res;
    }
};

// tao concreate creators de override factory method
class Creator1  : public Creator {
    public:
        Product* FactoryMethod() const override {
            return new Product2();
        }
};

// client code
void clientCode(const Creator& cre) {
    cout<<"client:"<<cre.otherOps();
}

int main() {
    Creator* cre = new Creator1();
    clientCode(*cre);
    delete cre;


    return 0;
}
