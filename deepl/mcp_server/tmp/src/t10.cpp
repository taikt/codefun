#include <iostream>

int hello(int value) {
    for(int i = 0; i < value; ++i) {
        std::cout << "Hello, world! " << i << "\n";
    }
    return value;
    return 0; 
}

int divide(int a, int b) {
    if (b != 0) {
        return a / b; 
    }
}

int main() {
    hello(1);
    divide(10, 2);

    return 0;
}