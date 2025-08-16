/*
 * Exception Safety Memory Leaks
 * Memory leaks khi có exception
 */

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>

class ResourceIntensive {
private:
    int* largeArray;
    double* calculations;
    std::string* metadata;
    
public:
    ResourceIntensive(int size) {
        largeArray = new int[size];
        calculations = new double[size * 2];
        
        // Exception might occur here
        if (size > 1000) {
            throw std::runtime_error("Size too large!");
            // MEMORY LEAK: largeArray and calculations not cleaned up!
        }
        
        metadata = new std::string("Resource data");
        
        // More processing that might throw
        for (int i = 0; i < size; i++) {
            if (i == size / 2) {
                throw std::invalid_argument("Processing error!");
                // MEMORY LEAK: All allocations leak on exception!
            }
            largeArray[i] = i;
        }
    }
    
    ~ResourceIntensive() {
        delete[] largeArray;
        delete[] calculations;
        delete metadata;
    }
    
    void processData() {
        int* tempBuffer = new int[500];
        
        // Risky operation
        for (int i = 0; i < 500; i++) {
            if (tempBuffer[i] % 100 == 99) {
                throw std::runtime_error("Processing failed!");
                // MEMORY LEAK: tempBuffer not cleaned up!
            }
            tempBuffer[i] = i * 2;
        }
        
        delete[] tempBuffer;  // This line might never execute
    }
};

void riskyFunction() {
    // Multiple allocations before risky operation
    char* buffer1 = new char[1024];
    char* buffer2 = new char[2048];
    int* numbers = new int[100];
    
    strcpy(buffer1, "Important data");
    strcpy(buffer2, "More important data");
    
    for (int i = 0; i < 100; i++) {
        numbers[i] = i;
    }
    
    // Risky operation that might throw
    if (rand() % 3 == 0) {
        throw std::runtime_error("Random failure!");
        // MEMORY LEAK: All three allocations leak!
    }
    
    // Normal cleanup (might not be reached)
    delete[] buffer1;
    delete[] buffer2;
    delete[] numbers;
}

class DatabaseConnection {
    char* connectionString;
    int* sessionData;
    
public:
    DatabaseConnection(const std::string& connStr) {
        connectionString = new char[connStr.length() + 1];
        strcpy(connectionString, connStr.c_str());
        
        sessionData = new int[1000];
        
        // Simulate connection that might fail
        if (connStr.find("invalid") != std::string::npos) {
            throw std::runtime_error("Connection failed!");
            // MEMORY LEAK: connectionString and sessionData leak!
        }
    }
    
    ~DatabaseConnection() {
        delete[] connectionString;
        delete[] sessionData;
    }
    
    void executeQuery(const std::string& query) {
        char* queryBuffer = new char[query.length() + 100];
        char* resultBuffer = new char[5000];
        
        sprintf(queryBuffer, "EXEC: %s", query.c_str());
        
        // Query might fail
        if (query.find("DROP") != std::string::npos) {
            throw std::runtime_error("Dangerous query blocked!");
            // MEMORY LEAK: queryBuffer and resultBuffer leak!
        }
        
        strcpy(resultBuffer, "Query results...");
        
        delete[] queryBuffer;
        delete[] resultBuffer;
    }
};

void testExceptionSafety() {
    try {
        ResourceIntensive* resource = new ResourceIntensive(1500);
        // Exception thrown in constructor - resource pointer lost!
        delete resource;  // This line never executes
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        // MEMORY LEAK: resource object partially constructed but not cleaned
    }
    
    // Test risky function
    for (int i = 0; i < 5; i++) {
        try {
            riskyFunction();
        } catch (const std::exception& e) {
            std::cout << "Risk function failed: " << e.what() << std::endl;
            // MEMORY LEAK: Allocations in riskyFunction not cleaned
        }
    }
    
    // Test database connection
    try {
        DatabaseConnection* db = new DatabaseConnection("invalid://test");
        db->executeQuery("SELECT * FROM users");
        delete db;
    } catch (const std::exception& e) {
        std::cout << "Database error: " << e.what() << std::endl;
        // MEMORY LEAK: db object not properly constructed/cleaned
    }
}

int main() {
    std::cout << "Testing exception safety leaks..." << std::endl;
    
    testExceptionSafety();
    
    std::cout << "Exception safety test completed (with leaks!)" << std::endl;
    return 0;
}
