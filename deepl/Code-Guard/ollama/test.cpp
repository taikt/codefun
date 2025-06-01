#include <iostream>

int processNumber(int value) {
    switch (value) {
        case 1:
            std::cout << "Value is one\n"; 
        case 2:
            std::cout << "Value is two\n";
            break;
        case 3:
            std::cout << "Value is three\n";
            return 3; 
        default:
            std::cout << "Other value\n";
            break;
    }
    return value; 
}
void leakMemory() {
    int* ptr = new int(42); // Memory leak here, as we do not delete ptr
}

int main() {
    int result = processNumber(1);
    std::cout << "Result: " << result << "\n";
    return 0;
}