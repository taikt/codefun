// https://grok.com/chat/45e0195d-2439-4f9b-a7c6-e6062d77ea0f
#include <iostream>
#include <vector>
#include <string>

// Hàm template sử dụng generic lambda
template <typename Container, typename Lambda>
void process(Container& container, Lambda func) {
    for (auto& item : container) {
        func(item); // Gọi lambda với từng phần tử
    }
}

int main() {
    // Vector chứa các số nguyên
    std::vector<int> numbers = {1, 2, 3, 4, 5};

    // Generic lambda: nhân đôi giá trị của bất kỳ kiểu dữ liệu nào
    auto doubleValue = [](auto& value) {
        value *= 2;
    };

    // Gọi hàm process với vector số nguyên và lambda
    process(numbers, doubleValue);

    // In kết quả
    std::cout << "Doubled numbers: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Vector chứa các chuỗi
    std::vector<std::string> words = {"hello", "world"};

    // Generic lambda: thêm dấu chấm than vào chuỗi hoặc bất kỳ kiểu nào hỗ trợ += "!"
    auto addExclamation = [](auto& value) {
        value += "!";
    };

    // Gọi hàm process với vector chuỗi và lambda
    process(words, addExclamation);

    // In kết quả
    std::cout << "Modified words: ";
    for (const auto& word : words) {
        std::cout << word << " ";
    }
    std::cout << std::endl;

    return 0;
}