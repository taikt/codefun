#include <stdio.h>

# API
# memset, memcmp, memcpy

// display values of array
void displayMemory(const char* str, char* arr, int size) {
    int i;
    printf("%s %p\n",str,arr);

    for (int i=size-1; i>= 0; i--) {
        printf("%x ",arr[i]);
    }
    printf("\n");
}



int main() {

    // test memset
    char[6] a

    return 0;
}
