/*
 * Basic Memory Leak Examples
 * Các ví dụ cơ bản về memory leak
 */

#include <iostream>
#include <string>

class User {
private:
    std::string* name;
    int* age;
    
public:
    User(const std::string& userName, int userAge) {
        // Memory allocation without proper cleanup
        name = new std::string(userName);
        age = new int(userAge);
    }
    
    // MEMORY LEAK: Missing destructor!
    // Destructor should delete name and age
    
    void displayInfo() {
        std::cout << "User: " << *name << ", Age: " << *age << std::endl;
    }
    
    void updateName(const std::string& newName) {
        // MEMORY LEAK: Old name pointer is lost!
        name = new std::string(newName);
    }
};

void createTemporaryBuffer() {
    // MEMORY LEAK: malloc without free
    char* buffer = (char*)malloc(1024);
    strcpy(buffer, "Some data");
    
    // Function returns without freeing buffer!
    if (strlen(buffer) > 100) {
        return; // Early return causes memory leak
    }
    
    // Even if we reach here, still no free()!
}

int* createArray(int size) {
    // MEMORY LEAK: Caller might not know to delete[]
    int* arr = new int[size];
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }
    return arr; // Caller responsible for cleanup, but not documented!
}

void processData() {
    User* user1 = new User("Alice", 25);
    User* user2 = new User("Bob", 30);
    
    user1->displayInfo();
    user2->displayInfo();
    
    // MEMORY LEAK: users never deleted!
    
    // Create array but forget to delete
    int* numbers = createArray(100);
    
    // Process numbers...
    std::cout << "First number: " << numbers[0] << std::endl;
    
    // MEMORY LEAK: numbers array never deleted!
}

int main() {
    std::cout << "Running memory leak examples..." << std::endl;
    
    // Multiple function calls creating leaks
    for (int i = 0; i < 5; i++) {
        createTemporaryBuffer();
        processData();
    }
    
    std::cout << "Program finished (with memory leaks!)" << std::endl;
    return 0;
}
