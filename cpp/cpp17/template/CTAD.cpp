// https://grok.com/chat/ff59af38-7913-4fe5-93bc-b1d8771a2dc3
// Class Template Argument Deduction (CTAD)
#include <iostream>
#include <string>

// Định nghĩa một lớp mẫu đơn giản
template <typename T>
class Box {
public:
    T value;
    Box(T v) : value(v) {}
    void print() const {
        std::cout << "Value: " << value << std::endl;
    }
};

int main() {
    // Không cần chỉ định rõ ràng kiểu Box<int> hay Box<std::string>
    Box box1(42);           // CTAD suy ra T = int
    Box box2(std::string("Hello")); // CTAD suy ra T = std::string

    box1.print(); // In ra: Value: 42
    box2.print(); // In ra: Value: Hello

    // CTAD cũng hoạt động với các container
    std::pair p(1, 2.5); // Suy ra std::pair<int, double>
    std::cout << "Pair: " << p.first << ", " << p.second << std::endl;

    return 0;
}