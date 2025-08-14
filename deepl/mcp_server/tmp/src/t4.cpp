/*
 * Mixed Allocation/Deallocation Types - Memory Leaks
 * Lỗi sử dụng sai kiểu allocation/deallocation
 */

#include <iostream>
#include <cstdlib>

class DataManager {
private:
    int* singleValue;
    double* arrayData;
    char* stringBuffer;
    
public:
    DataManager(int size) {
        // Mixed allocation types
        singleValue = (int*)malloc(sizeof(int));  // malloc
        arrayData = new double[size];              // new[]
        stringBuffer = new char[256];              // new[]
        
        *singleValue = 42;
        for (int i = 0; i < size; i++) {
            arrayData[i] = i * 1.5;
        }
        strcpy(stringBuffer, "Initial data");
    }
    
    ~DataManager() {
        // MEMORY LEAK: Wrong deallocation types!
        delete singleValue;    // Should be free()! 
        delete arrayData;      // Should be delete[]!
        free(stringBuffer);    // Should be delete[]!
    }
    
    void resetData() {
        // MEMORY LEAK: Reassignment without cleanup
        singleValue = (int*)malloc(sizeof(int));
        arrayData = new double[100];
        stringBuffer = new char[512];
        
        // Old pointers are lost forever!
    }
};

void mixedAllocationExample() {
    // C-style allocation
    int* cArray = (int*)calloc(50, sizeof(int));
    
    // C++ style allocation  
    std::string* cppString = new std::string("Hello");
    
    // Array allocation
    float* floatArray = new float[25];
    
    // Process data...
    for (int i = 0; i < 50; i++) {
        cArray[i] = i;
    }
    
    *cppString = "Modified string";
    
    for (int i = 0; i < 25; i++) {
        floatArray[i] = i * 2.5f;
    }
    
    // MEMORY LEAKS: Wrong cleanup!
    delete cArray;           // Should be free()!
    delete[] cppString;      // Should be delete!
    free(floatArray);        // Should be delete[]!
}

class ResourceHolder {
    int* data;
    
public:
    ResourceHolder(int* ptr) : data(ptr) {}
    
    // MEMORY LEAK: No destructor to clean up data!
    // Also no copy constructor/assignment operator
    
    ResourceHolder(const ResourceHolder& other) {
        // MEMORY LEAK: Shallow copy creates double-delete potential
        data = other.data;
    }
    
    ResourceHolder& operator=(const ResourceHolder& other) {
        // MEMORY LEAK: No cleanup of existing data
        data = other.data;
        return *this;
    }
    
    void replaceData(int* newData) {
        // MEMORY LEAK: Old data pointer lost
        data = newData;
    }
};

void objectLifecycleLeaks() {
    DataManager* dm1 = new DataManager(20);
    DataManager* dm2 = new DataManager(30);
    
    // Use objects
    dm1->resetData();  // Creates internal leaks
    dm2->resetData();  // Creates internal leaks
    
    // MEMORY LEAK: Objects never deleted
    // Even if deleted, destructors have wrong deallocation!
    
    // Test ResourceHolder
    int* testData = new int[10];
    ResourceHolder holder1(testData);
    ResourceHolder holder2 = holder1;  // Shallow copy
    
    // MEMORY LEAK: testData will be referenced by both holders
    // When they go out of scope, potential double-delete or no delete
}

int main() {
    std::cout << "Testing mixed allocation leaks..." << std::endl;
    
    for (int i = 0; i < 3; i++) {
        mixedAllocationExample();
        objectLifecycleLeaks();
    }
    
    std::cout << "Mixed allocation test completed (with leaks!)" << std::endl;
    return 0;
}
