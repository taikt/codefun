#include <cstdio>
#include <iostream>

void leak_file_descriptor() {
    FILE* f = fopen("test.txt", "w");
    if (f) {
        fprintf(f, "Resource leak example\n");
        // Quên không gọi fclose(f); => rò rỉ file descriptor
    }
}

void leak_fd_in_loop() {
    for (int i = 0; i < 10; ++i) {
        FILE* f = fopen("test_loop.txt", "w");
        if (f) {
            fprintf(f, "Loop %d\n", i);
            // Không đóng file => rò rỉ nhiều file descriptor
        }
    }
}

int main() {
    leak_file_descriptor();
    leak_fd_in_loop();
    return 0;
}