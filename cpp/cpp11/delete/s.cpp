// https://grok.com/chat/3a36052a-bbf0-43e9-a921-295e0fd73051
// delete specifier được giới thiệu từ C++11, sử dụng với từ khóa = delete 
// để ngăn chặn việc sử dụng một hàm thành viên hoặc hàm cụ thể. 
// Điều này hữu ích khi bạn muốn vô hiệu hóa một số hành vi mặc định 
// (như copy constructor, copy assignment operator) 
// hoặc ngăn chặn việc gọi hàm với các tham số không mong muốn.
#include <iostream>

class Config {
public:
    // Constructor yêu cầu tham số
    Config(int id) : configId(id) {
        std::cout << "Config created with ID: " << configId << std::endl;
    }

    // Vô hiệu hóa default constructor
    Config() = delete;

    void print() const {
        std::cout << "Config ID: " << configId << std::endl;
    }

private:
    int configId;
};

int main() {
    Config cfg(42); // OK
    cfg.print();

    // Dòng sau sẽ gây lỗi biên dịch vì default constructor bị xóa
    // Config cfg2;

    return 0;
}