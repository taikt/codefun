#include <stdio.h>
/**
  * address of x=0061FF2F, address of y=0061FF2E
  * refer x from address of x=21, refer y from address of y=31
  * value of a=0061FF2F, address of a=0061FF10, refer value from a=00000021
  * value of b=0061FF2E, address of b=0061FF14, refer value from b=00000031
  * x=31, y=21
  */


void swap(char *a, char *b) {
    char temp;
    printf("value of a=%p, address of a=%p, refer value from a=%p\n",a,&a,*a);
    printf("value of b=%p, address of b=%p, refer value from b=%p\n",b,&b,*b);
    temp = *a;
    *a = *b;
    *b = temp;
}

int main() {
    char x = 0x21, y = 0x31;
    printf("address of x=%p, address of y=%p\n",&x,&y);
    printf("refer x from address of x=%x, refer y from address of y=%x\n",*(&x),*(&y));
    swap(&x,&y);

    printf("x=%x, y=%x\n",x,y);

    return 0;
}
