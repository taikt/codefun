**State pattern** là một design pattern thuộc nhóm hành vi (behavioral pattern), cho phép một đối tượng thay đổi hành vi của nó khi trạng thái nội bộ thay đổi. Đối tượng sẽ xuất hiện như thể nó đã thay đổi class của mình.

### Đặc điểm chính:
- Mỗi trạng thái là một class riêng, kế thừa từ một interface chung (ví dụ: `State`).
- Đối tượng context (ví dụ: `Context`) giữ một con trỏ tới trạng thái hiện tại (`State*`).
- Khi trạng thái thay đổi, context sẽ chuyển con trỏ sang một đối tượng trạng thái khác.
- Mỗi trạng thái có thể xử lý các hành động khác nhau, hoặc quyết định chuyển sang trạng thái khác.

---

## Ví dụ State Pattern trong C++

Giả sử bạn có một máy bán hàng tự động (Vending Machine) với các trạng thái: Không có tiền (`NoCoinState`), Có tiền (`HasCoinState`), Đang bán hàng (`SoldState`).

```cpp
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
```

**Tóm lại:**  
- State pattern giúp đóng gói từng trạng thái thành class riêng, dễ mở rộng, bảo trì, và thay đổi hành vi động theo trạng thái.
- Trong ví dụ trên, mỗi trạng thái của máy bán hàng là một class riêng, và máy sẽ chuyển trạng thái khi có sự kiện phù hợp.