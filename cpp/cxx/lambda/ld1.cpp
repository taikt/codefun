#include <iostream>
#include <vector>
#include <functional>

class WorkingClass {
public:
    typedef const std::function<void (int)> handler_t;
    void AddHandler(handler_t& h) {
        handlerList.push_back(&h);
    }
    
    void DoStuff() {
        for (auto& handler : handlerList) {
            (*handler)(42);
            (*handler)(23);
        }
    }

private:
    std::vector<handler_t*> handlerList;
};

int main() {
    WorkingClass wc;
    
    wc.AddHandler([&](int num) {
    std::cout << "A: " << num << std::endl;
    });
    wc.AddHandler([&](int num) {
    std::cout << "B: " << num << std::endl;
    });
    
    wc.DoStuff();
}
