#include <iostream>

class DataHolder {
public:
    DataHolder(size_t n) {
        data = new int[n]; // cấp phát động
        size = n;
    }
    void set(size_t idx, int value) {
        if (idx < size) data[idx] = value;
    }
    int get(size_t idx) const {
        if (idx < size) return data[idx];
        return -1;
    }
    ~DataHolder() {
    }
private:
    int* data;
    size_t size;
};

int main() {
    DataHolder* holder = new DataHolder(100);
    holder->set(0, 123);
    std::cout << "holder->get(0) = " << holder->get(0) << std::endl;
   
    return 0;
}