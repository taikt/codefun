#include <stdio.h>

// Bit identification
#define isBitSet(x,n) ((x & (1<<n))!=0 ? 1 : 0)
// http://stackoverflow.com/questions/12989298/is-it-possible-to-use-a-if-statement-inside-define

int main()
{

    unsigned int a = 0x20; //32, 0010 0000
    printf("a = 0x%x %d\n",a,a);
    int n = 6;
    printf("is bit set at n=%d:%d\n",n,(isBitSet(a,n)==1 ? 1:0));


    return 0;
}
