#include <stdio.h>

int main() {
    unsigned int a=0xabcd1234;

    printf("base address of a:%x %p\n",&a, &a);

    unsigned int *p;
    p = &a;
    printf("address of p:%x, value of p=%x, value of a refered from p=%x\n",&p, p,*p);

    unsigned int **p2;
    p2= &p;

    printf("address of p2:%x, value of p2=%x, value of p1 refered from p2=%x,"
           " value of a refered from p2=%x\n",&p2, p2, *p2, **p2);

    return 0;
}
