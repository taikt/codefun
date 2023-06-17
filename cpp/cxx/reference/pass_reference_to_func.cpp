// passing by pointer is ok both C++, C
// passing by reference is only ok on C++ 
// http://www.geeksforgeeks.org/passing-by-pointer-vs-passing-by-reference-in-c/
// https://www3.ntu.edu.sg/home/ehchua/programming/cpp/cp4_PointerReference.html ==> must read
// https://stackoverflow.com/questions/3796181/c-passing-pointer-to-function-howto-c-pointer-manipulation
// https://stackoverflow.com/questions/57483/what-are-the-differences-between-a-pointer-variable-and-a-reference-variable-in


#include <stdio.h>
#include <stdlib.h>

// pass a reference
void foo(int &x) {
	x=2;
}

// pass a pointer
void foo_p(int* x) {
	*x = 9;
}

// pass by value
void foo_v(int x) {
	x = 7;
}

int main() {
	int x = 4;
	foo(x);
	printf("x=%d\n",x); // x=2
	foo_p(&x);
	printf("x=%d\n",x); // x=9
	foo_v(x);
	printf("x=%d\n",x); // x=9

	return 0;
}
