// http://stackoverflow.com/questions/19167201/copy-constructor-of-template-class

#include "stdio.h"
#include <iostream>
using namespace std;


class A {
private:
	int* i;

public:
	
	A() {
		i = new int;
		*i = 10;
	};
		
	//A(A const & rhs) :i(new int) {
		//*i = *rhs->i;
	//} 
	
	// note: compiler never calll copy constructor from a template
	// compiler uses default calling 
    
    
	template <typename T>
	A(T const & rhs) 
		:i(new int) {
		//*i = *rhs->i;
		*i = 5;
	}

	void addValue(int a) {
   	    *i = a;
   	}

   	int getValue(){
   		return *i;
   	}
};

void f(A const & a1) {
	A a2(a1);
	//A a2;
	printf("i of a2=%d\n",a2.getValue());
}

int main() {
	//A *a = new A();
	A a1;
	a1.addValue(2);
	printf("i of a1=%d\n",a1.getValue());

	f(a1);
    

	return 0;
}
