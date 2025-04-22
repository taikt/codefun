// https://grok.com/chat/1c8ab45e-10f9-457b-a5ce-de8bbdc53789
#include <iostream>
#include <vector>

// Hàm in các phần tử bằng fold expression
template <typename... Args>
void print_elements(Args... args) {
    // Fold expression để in lần lượt các tham số, cách nhau bởi dấu phẩy
    ((std::cout << args << ", "), ...);
    std::cout << std::endl;
}

// Hàm in truc tiep cac thanh phan
template <typename T>
void print_vector(const std::vector<T>& vec) {
    // Kiểm tra vector rỗng
    if (vec.empty()) {
        std::cout << "Vector is empty" << std::endl;
        return;
    }

    // Tạo tuple từ vector (cần một cách để mở vector thành tham số)
    // Ở đây, sử dụng vòng lặp đơn giản để gọi print_elements trực tiếp
    std::cout << "Vector elements: ";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i == vec.size() - 1) {
            std::cout << vec[i] << std::endl; // Không in dấu phẩy ở phần tử cuối
        } else {
            std::cout << vec[i] << ", ";
        }
    }
}



// Hàm chính để kiểm tra
int main() {
    // Tạo một số vector để kiểm tra
    std::vector<int> vec1 = {1, 2, 3, 4, 5};
    std::vector<double> vec2 = {1.1, 2.2, 3.3};
    std::vector<std::string> vec3 = {"Hello", "World"};
    std::vector<int> vec4 = {};

    // In các vector
    print_vector(vec1);
    print_vector(vec2);
    print_vector(vec3);
    print_vector(vec4);

    // Ví dụ gọi trực tiếp print_elements với fold expression
    std::cout << "\nDirect fold expression example:" << std::endl;
    print_elements(1, 2, 3, 4, 5); // Gọi trực tiếp để minh họa fold expression

    

    return 0;
}
