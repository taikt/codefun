# 🛡️ LGEDV Analysis Report

**File:** `line_alignment_test.cpp`  
**Path:** `/home/worker/src/code_test/line_alignment_test.cpp`  
**Analyzed:** 6/27/2025, 8:22:06 PM  
**Rule Type:** LGEDV Coding Standards  
**Model:** GPT 4  
**Lines of Code:** 94  
**Code Size:** 3162 characters  
**Total Issues:** 6 violations found

---

## 📊 Summary Dashboard

| Severity | Count | Description |
|----------|-------|-------------|
| 🔴 Critical | 1 | Must fix immediately |
| 🟠 High | 1 | Fix before release |
| 🟡 Medium | 3 | Improve code quality |
| 🔵 Low | 1 | Best practice recommendations |

---

## 📋 Analysis Results

## 🚨 Issue 1: Use of magic numbers in loops and initialization
**Rule Violated:** LGEDV_CRCL_0001 - Use definition/enumerations instead of magic number.  
**Line:** 17, 28, 72  
**Severity:** Medium  
**Current Code:**
```cpp
for (int i = 0; i < 100; ++i) { // Line 17
    data.push_back(i * 2);
    lookup[std::to_string(i)] = i * 2;
}

for (int i = 0; i < 20; ++i) { // Line 72
    std::cout << "Line " << i << ": " << longString.substr(0, 50) << std::endl;
}
```
**Fixed Code:**
```cpp
constexpr int MAX_DATA_SIZE = 100;
constexpr int MAX_LINES = 20;

for (int i = 0; i < MAX_DATA_SIZE; ++i) {
    data.push_back(i * 2);
    lookup[std::to_string(i)] = i * 2;
}

for (int i = 0; i < MAX_LINES; ++i) {
    std::cout << "Line " << i << ": " << longString.substr(0, 50) << std::endl;
}
```
**Explanation:** Magic numbers were replaced with named constants to improve readability and maintainability.

---

## 🚨 Issue 2: Memory leak due to raw pointer allocation
**Rule Violated:** LGEDV_CRCL_0020 - Every allocated resource needs to be released properly.  
**Line:** 39  
**Severity:** Critical  
**Current Code:**
```cpp
int* rawPtr = new int(42);  // Memory leak potential
```
**Fixed Code:**
```cpp
std::unique_ptr<int> rawPtr = std::make_unique<int>(42);
```
**Explanation:** Replaced raw pointer with `std::unique_ptr` to ensure proper memory management and avoid leaks.

---

## 🚨 Issue 3: Use of C-style cast
**Rule Violated:** LGEDV_CRCL_0019 - Use meaningful return values and avoid unsafe practices.  
**Line:** 42  
**Severity:** High  
**Current Code:**
```cpp
double value = (double)*rawPtr;
```
**Fixed Code:**
```cpp
double value = static_cast<double>(*rawPtr);
```
**Explanation:** Replaced C-style cast with `static_cast` for type safety and compliance with modern C++ practices.

---

## 🚨 Issue 4: Unused variable
**Rule Violated:** LGEDV_CRCL_0005 - Use meaningful variable names and avoid unused variables.  
**Line:** 45  
**Severity:** Low  
**Current Code:**
```cpp
int unusedVar = 123;
```
**Fixed Code:**
```cpp
// Removed unused variable
```
**Explanation:** Removed the unused variable to avoid clutter and potential confusion.

---

## 🚨 Issue 5: Missing log at the beginning of functions
**Rule Violated:** LGEDV_CRCL_0011 - Add at least one log at the beginning of each function.  
**Line:** 25, 36, 66  
**Severity:** Medium  
**Current Code:**
```cpp
void processData() {
    std::sort(data.begin(), data.end());
    // ...
}

void demonstratePotentialIssues() {
    int* rawPtr = new int(42);
    // ...
}

void thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay() {
    std::string longString = "This is a very long string...";
    // ...
}
```
**Fixed Code:**
```cpp
void processData() {
    std::cout << "Entering processData()" << std::endl;
    std::sort(data.begin(), data.end());
    // ...
}

void demonstratePotentialIssues() {
    std::cout << "Entering demonstratePotentialIssues()" << std::endl;
    int* rawPtr = new int(42);
    // ...
}

void thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay() {
    std::cout << "Entering thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay()" << std::endl;
    std::string longString = "This is a very long string...";
    // ...
}
```
**Explanation:** Added logs at the beginning of each function for traceability.

---

## 🚨 Issue 6: Range-based for loop not used
**Rule Violated:** LGEDV_CRCL_0009 - Recommended Cyclomatic Complexity is lower than 10.  
**Line:** 30  
**Severity:** Medium  
**Current Code:**
```cpp
for (auto it = data.begin(); it != data.end(); ++it) {
    if (*it > 50) {
        std::cout << "Processing: " << *it << std::endl;
    }
}
```
**Fixed Code:**
```cpp
for (const auto& value : data) {
    if (value > 50) {
        std::cout << "Processing: " << value << std::endl;
    }
}
```
**Explanation:** Replaced iterator-based loop with a range-based for loop to simplify the code and reduce complexity.

---

## 🔧 Complete Fixed Code
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <map>
#include <set>
#include <unordered_map>

namespace TestNamespace {

    class ComplexClass {
    private:
        std::vector<int> data;
        std::unordered_map<std::string, int> lookup;
        std::shared_ptr<std::string> name;

    public:
        ComplexClass(const std::string& className) 
            : name(std::make_shared<std::string>(className)) {
            constexpr int MAX_DATA_SIZE = 100;
            for (int i = 0; i < MAX_DATA_SIZE; ++i) {
                data.push_back(i * 2);
                lookup[std::to_string(i)] = i * 2;
            }
        }

        void processData() {
            std::cout << "Entering processData()" << std::endl;
            std::sort(data.begin(), data.end());
            for (const auto& value : data) {
                if (value > 50) {
                    std::cout << "Processing: " << value << std::endl;
                }
            }
        }

        void demonstratePotentialIssues() {
            std::cout << "Entering demonstratePotentialIssues()" << std::endl;
            std::unique_ptr<int> rawPtr = std::make_unique<int>(42);
            double value = static_cast<double>(*rawPtr);
            std::cout << "Value: " << value << std::endl;
        }

        void thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay() {
            std::cout << "Entering thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay()" << std::endl;
            std::string longString = "This is a very long string that extends beyond normal line width to test horizontal scrolling functionality in the code analysis report display";
            constexpr int MAX_LINES = 20;
            for (int i = 0; i < MAX_LINES; ++i) {
                std::cout << "Line " << i << ": " << longString.substr(0, 50) << std::endl;
            }
        }

        ~ComplexClass() {
            std::cout << "Destructor called for: " << *name << std::endl;
        }
    };

    template<typename T, typename U, typename V>
    auto processMultipleTypes(const T& first, const U& second, const V& third) -> decltype(first + second + third) {
        return first + second + third;
    }

} // namespace TestNamespace

int main() {
    TestNamespace::ComplexClass obj("TestObject");

    obj.processData();
    obj.demonstratePotentialIssues();
    obj.thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay();

    auto result = TestNamespace::processMultipleTypes(1, 2.5, 3.14f);
    std::cout << "Template result: " << result << std::endl;

    return 0;
}
```

---

## 📄 Source Code Analyzed

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <map>
#include <set>
#include <unordered_map>

// This is a test file to verify line number alignment
// and scrollbar functionality in code analysis reports

namespace TestNamespace {
    
    class ComplexClass {
    private:
        std::vector<int> data;
        std::unordered_map<std::string, int> lookup;
        std::shared_ptr<std::string> name;
        
    public:
        ComplexClass(const std::string& className) 
            : name(std::make_shared<std::string>(className)) {
            // Initialize with some default data
            for (int i = 0; i < 100; ++i) {
                data.push_back(i * 2);
                lookup[std::to_string(i)] = i * 2;
            }
        }
        
        void processData() {
            std::sort(data.begin(), data.end());
            
            // This loop violates MISRA rules - should use range-based for
            for (auto it = data.begin(); it != data.end(); ++it) {
                if (*it > 50) {
                    std::cout << "Processing: " << *it << std::endl;
                }
            }
        }
        
        void demonstratePotentialIssues() {
            // Multiple potential violations here
            int* rawPtr = new int(42);  // Memory leak potential
            
            // Use of C-style cast (should use static_cast)
            double value = (double)*rawPtr;
            
            // Unused variable
            int unusedVar = 123;
            
            // Missing delete for rawPtr - memory leak!
            std::cout << "Value: " << value << std::endl;
        }
        
        // Long method name to test horizontal scrolling
        void thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay() {
            std::string longString = "This is a very long string that extends beyond normal line width to test horizontal scrolling functionality in the code analysis report display";
            
            std::cout << longString << std::endl;
            
            // More code to increase vertical height
            for (int i = 0; i < 20; ++i) {
                std::cout << "Line " << i << ": " << longString.substr(0, 50) << std::endl;
            }
        }
        
        ~ComplexClass() {
            std::cout << "Destructor called for: " << *name << std::endl;
        }
    };
    
    // Function with multiple parameters to test formatting
    template<typename T, typename U, typename V>
    auto processMultipleTypes(const T& first, const U& second, const V& third) -> decltype(first + second + third) {
        return first + second + third;
    }
    
} // namespace TestNamespace

int main() {
    TestNamespace::ComplexClass obj("TestObject");
    
    obj.processData();
    obj.demonstratePotentialIssues();
    obj.thisIsAVeryLongMethodNameThatShouldTriggerHorizontalScrollingInTheCodeDisplay();
    
    // Test template function
    auto result = TestNamespace::processMultipleTypes(1, 2.5, 3.14f);
    std::cout << "Template result: " << result << std::endl;
    
    return 0;
}

```

---

## 🚀 Quick Actions

To view the interactive HTML report through the configured web server:
```bash
cd /home/worker/src/codeguard/web_srv
./start_server.sh start
# Then open: http://localhost:8888
```

Or view directly:
```bash
cd /home/worker/src/codeguard/web_srv/defect_report
python3 -m http.server 8081
# Then open: http://localhost:8081/line_alignment_test-lgedv-report.html
```

**📂 All reports are saved to:** `/home/worker/src/codeguard/web_srv/defect_report`

---

*Generated by LGEDV CodeGuard v1.0.0 - LGEDV Analysis*
