#include <stdio.h>
#include <stdlib.h>
// https://www.youtube.com/watch?v=_8-ht2AKyH4


int main() {
    int a; //allocate memory in stack
    int *p;
    p = (int*) malloc(sizeof(int)); // allocate a block of 4 bytes in heap memory
    // p store the base address of the block
    *p = 10; //content of the block is 10
    free(p); // free the block, if not free, the block exists in heap ==> leak memory

    p = (int*)malloc(20*sizeof(int)); // allocate a block of 20*4= 80bytes in heap
    // this is example to store an array in heap
    p[0] = 2; //*p
    p[1] = 3; //*(p+1)
    printf("%d %d\n",p[1],p[2]);
    free(p);

    return 0;
}
