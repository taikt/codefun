// https://www.tutorialspoint.com/cplusplus/cpp_polymorphism.htm

#include <iostream>
#include <cstdio>
using namespace std;

class base {
public:
	base() {
		printf("base constructor\n");
	}
	void function() {}
	// https://stackoverflow.com/questions/2652198/difference-between-a-virtual-function-and-a-pure-virtual-function
	virtual void test1() = 0; // pure virtual function: make base class as abstract. 
	//the derived class 'has to' implement the inherited pure virtual function.

	virtual void test2() { // virtual function
		printf("base test2\n");
	}
    
    int m_base;
};

class derived1 : public base {
public:
	derived1():_iderived1Value(5) {
		printf("derived1 constructor\n");
        m_base = 2;
	}
	void derived1Function() {
		printf("derived1 value:%d\n", _iderived1Value);
	}
	virtual void test1() { // override
		printf("derived1 test1\n");
	}
#if 1
	virtual void test2() { // override
		printf("derived1 test2\n");
	}
#endif
private:
	int _iderived1Value;
};


class derived2 : public base {
public:
	derived2():_iderived2Value(7) {
		printf("derived2 constructor\n");
	}
	void derived2Function() {
		printf("derived1 value:%d\n", _iderived2Value);
	}
private:
	int _iderived2Value;
};

int main() {
	base* ptrBase = new derived1();

	ptrBase->test1();
	ptrBase->test2();
    
    printf("m_base=%d\n",ptrBase->m_base);

	return 0;
}
