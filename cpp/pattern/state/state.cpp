#include <iostream>
#include <memory>

// Interface cho State
class State {
public:
    virtual ~State() {}
    virtual void insertCoin() = 0;
    virtual void dispense() = 0;
};

// Forward declaration
class VendingMachine;

// State cụ thể: Không có tiền
class NoCoinState : public State {
    VendingMachine* machine;
public:
    NoCoinState(VendingMachine* m) : machine(m) {}
    void insertCoin() override;
    void dispense() override {
        std::cout << "Bạn cần cho tiền vào trước!\n";
    }
};

// State cụ thể: Có tiền
class HasCoinState : public State {
    VendingMachine* machine;
public:
    HasCoinState(VendingMachine* m) : machine(m) {}
    void insertCoin() override {
        std::cout << "Đã có tiền rồi!\n";
    }
    void dispense() override;
};

// Context
class VendingMachine {
    std::unique_ptr<State> state;
public:
    VendingMachine();
    void setState(State* s) { state.reset(s); }
    void insertCoin() { state->insertCoin(); }
    void dispense() { state->dispense(); }
    // Truy cập cho State
    friend class NoCoinState;
    friend class HasCoinState;
};

void NoCoinState::insertCoin() {
    std::cout << "Đã nhận tiền!\n";
    machine->setState(new HasCoinState(machine));
}
void HasCoinState::dispense() {
    std::cout << "Đang trả hàng...\n";
    machine->setState(new NoCoinState(machine));
}

VendingMachine::VendingMachine() {
    state.reset(new NoCoinState(this));
}

// Demo
int main() {
    VendingMachine vm;
    vm.dispense();      // Bạn cần cho tiền vào trước!
    vm.insertCoin();    // Đã nhận tiền!
    vm.insertCoin();    // Đã có tiền rồi!
    vm.dispense();      // Đang trả hàng...
    vm.dispense();      // Bạn cần cho tiền vào trước!
    return 0;
}