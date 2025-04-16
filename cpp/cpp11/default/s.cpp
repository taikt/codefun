#include <iostream>
#include <string>

// https://grok.com/chat/3a36052a-bbf0-43e9-a921-295e0fd73051
// trình biên dịch tự động tạo ra triển khai mặc định cho các 
// hàm thành viên đặc biệt (special member functions)
// Default constructor
// Copy constructor
// Move constructor
// Copy assignment operator
// Move assignment operator
// Destructor

class Person {
public:
    std::string name;
    int age=0;

    // Tự định nghĩa constructor có tham số
    Person(const std::string& n, int a) : name(n), age(a) {}

    // Yêu cầu trình biên dịch tạo default constructor
    Person() = default;

    // Yêu cầu trình biên dịch tạo destructor mặc định
    ~Person() = default;

    // Yêu cầu trình biên dịch tạo copy constructor mặc định
    Person(const Person&) = default;

    // Hàm in thông tin
    void print() const {
        std::cout << "Name: " << name << ", Age: " << age << std::endl;
    }
};

int main() {
    // Sử dụng default constructor
    Person p1; // name = "", age = 0 (do triển khai mặc định)
    p1.print();

    // Sử dụng constructor có tham số
    Person p2("Alice", 25);
    p2.print();

    // Sử dụng copy constructor
    Person p3 = p2; // Sao chép p2
    p3.print();

    return 0;
}