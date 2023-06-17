// https://www.tutorialspoint.com/cplusplus/cpp_overloading.htm
//
#include <iostream>
#include <stdio.h>
using namespace std;

class printData {
    public:
        printData() {
            printf("object is created\n");
        }
        
        ~printData() {
            printf("object is removed\n");
        }

        void print(int i) {
            printf("i=%d\n",i);
        }
        
        void print(char *c) {
            printf("c=%s\n",c);
        }
        
};

int main() {
    printData pd;

    pd.print(5);
    pd.print("hello");

    return 0;
}
