#include <iostream>

int main() {
    int arr[10];
    arr[20] = 5; // Vi phạm Rule 0-1-1 (truy cập ngoài giới hạn)
    //??= define FOO 1 // Vi phạm Rule 2-3-1 (sử dụng trigraph)
    return 0;
}