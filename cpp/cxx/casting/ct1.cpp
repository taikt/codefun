// https://www.youtube.com/watch?v=lglGKxNrLgM&t=496s
// static vs dynamic cast

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
	//virtual void test1() = 0; // pure virtual function
	virtual void test2() { // virtual function
		printf("base test2\n");
	}

};

class derived1 : public base {
public:
	derived1():_iderived1Value(5) {
		printf("derived1 constructor\n");
	}
	void derived1Function() {
		printf("derived1 value:%d\n", _iderived1Value);
	}
	//virtual void test1() { // override
	//	printf("derived1 test\n");
	//}

	//virtual void test2() { // override
	//	printf("derived1 test2\n");
	//}

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
	// static cast

	// a. convert object from one type to another
	int i=9;
	float f = static_cast<float> (i);

	// b. convert pointer/reference from one type to a related type (down/up cast)
	base* ptrBase = new derived1();
	derived1* dp = static_cast<derived1*>(ptrBase); // down cast
	dp->derived1Function();
	//ptrBase->derived1Function(); // wrong

	base* ptrBase1 = static_cast<base*>(new derived1); // up cast


	// dynamic cast
	base* ptrBase2 = new derived1();
	// a. down cast only --> cast from base class to derived class
	// b. run-time type check. if succeed, dp2==ptrBase2; if fail, dp2==0
	// c. require 2 types to be polymorphic (have at least on virtual function)
	derived1* dp2 = dynamic_cast<derived1*>(ptrBase2); 
	dp2->derived1Function();

	//base* ptrBase3 = dynamic_cast<base*>(new derived1); // up cast, check..


	// const_cast
	//const char* str ="hello"; // only work on pointer and same type
	//char* modifiable = const_cast<char*> (str);

	// reinterpret_cast
	//long p = 0x523423;
	//base* dp = reinterpret_cast<base*>(p); // re-interpret the bits of object

	return 0;
}
