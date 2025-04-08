//more example: http://cc.byexamples.com/2007/10/11/simple-callback-function/

#include <stdio.h>
#include <stdlib.h>

void add(int a, int b)
{	
	printf("Implement add: %d + %d=%d\n", a, b,a+b);
}

void sub(int a, int b)
{	
	printf("Implement sub: %d - %d=%d\n",a,b, a-b);
}

//void (*fn) (int a, int b);
void apply(void (*fn)(), int a, int b)
{
	fn(a,b);
}

int main()
{
	printf("start call back..\n");
	// only refer function name at main()
	// not parse parameters
	apply(add,2, 3); // add -> *fn
	apply(sub,2, 3); // sub -> *fn
	
	return 0;
}