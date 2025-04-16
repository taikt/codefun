#include <iostream>
#include <string>
/*
delegating constructors (constructor ủy quyền) là một tính năng 
được giới thiệu từ C++11, cho phép một constructor trong cùng một lớp 
gọi một constructor khác của chính lớp đó để thực hiện việc khởi tạo.
 Điều này giúp tránh lặp lại mã khởi tạo và làm cho mã dễ bảo trì hơn.
*/
// https://grok.com/chat/3a36052a-bbf0-43e9-a921-295e0fd73051

class Person {
public:
    // Constructor chính (target constructor)
    Person(const std::string& n, int a) : name(n), age(a) {
        std::cout << "Target constructor called\n";
    }

    // Delegating constructor (gọi constructor chính với giá trị mặc định)
    Person() : Person("Unknown", 0) {
        std::cout << "Delegating constructor called\n";
    }

    // Delegating constructor khác
    Person(const std::string& n) : Person(n, 0) {
        std::cout << "Delegating constructor with name called\n";
    }

    void print() const {
        std::cout << "Name: " << name << ", Age: " << age << std::endl;
    }

private:
    std::string name;
    int age;
};

int main() {
    Person p1; // Gọi delegating constructor Person()
    p1.print();

    Person p2("Alice"); // Gọi delegating constructor Person(const std::string&)
    p2.print();

    Person p3("Bob", 30); // Gọi target constructor trực tiếp
    p3.print();

    return 0;
}