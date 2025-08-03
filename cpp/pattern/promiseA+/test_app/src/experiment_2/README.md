Bạn nhận định đúng:  
- Theo chuẩn Promise A+, khi resolve thì chuyển trạng thái thành fulfilled (resolved), khi có lỗi/exception thì chuyển sang rejected (reject).
- Promise A+ hỗ trợ `.then()` để xử lý thành công, `.catch()` để bắt lỗi, và `.finally()` để thực thi bất kể thành công hay thất bại.

**Đánh giá code hiện tại:**
1. **Resolve/Reject:**  
   - Hàm `resolve` và `reject` đã thực hiện đúng: chỉ chuyển trạng thái khi còn pending, và callback được enqueue bất đồng bộ.
   - Nếu callback trong `then` throw exception, promise tiếp theo sẽ bị reject (đúng chuẩn A+).

2. **Chaining:**  
   - Hàm `then` trả về promise mới, callback có thể trả về giá trị hoặc promise khác (đúng chuẩn A+).
   - Nếu callback trả về promise, promise tiếp theo sẽ chờ resolve/reject của promise đó (đúng chuẩn A+).

3. **Bắt lỗi:**  
   - Hàm `catchError` tương đương `.catch()` trong JS, cho phép bắt lỗi ở bất kỳ đâu trong chain (đúng chuẩn A+).

4. **finally():**  
   - **Chưa có** hỗ trợ `.finally()` như Promise JS. Promise A+ không bắt buộc finally, nhưng JS Promise hiện đại có.

**Đề xuất refactor để bổ sung finally():**

Thêm vào class Promise<T> và Promise<void>:
```cpp
template <typename F>
auto finally(F cb) {
    auto self = this->shared_from_this();
    auto newPromise = std::make_shared<Promise<T>>();
    auto exec = [self, cb, newPromise]() {
        try {
            cb();
        } catch (...) {
            // ignore exception in finally
        }
        if (self->state_ == State::Resolved) {
            if constexpr (!std::is_void<T>::value)
                newPromise->resolve(self->value_);
            else
                newPromise->resolve();
        } else if (self->state_ == State::Rejected) {
            newPromise->reject(self->exception_);
        }
    };
    if (state_ == State::Resolved || state_ == State::Rejected) {
        EventLoop::getInstance().enqueue(exec);
    } else {
        then_callbacks_.push_back(exec);
        catch_callbacks_.push_back(exec);
    }
    return newPromise;
}
```
- Đặt vào cả Promise<T> và Promise<void>.
- Hàm này sẽ luôn chạy callback `cb` khi promise settle (resolve hoặc reject), và truyền trạng thái tiếp cho promise tiếp theo.

**Tóm lại:**  
- Code hiện tại đã tuân thủ Promise A+ về then/catch và chain.
- Nên bổ sung finally() để tương đương Promise JS hiện đại.

Bạn có muốn tôi tự động thêm finally() vào code không?


####
```cpp
int main() {
    std::cout << "Bắt đầu lấy dữ liệu..." << std::endl;
    fetchData(true)
        ->then([](const std::string& result) {
            std::cout << result << std::endl;
            return fetchData(false); // Gọi tiếp Promise thất bại
        })
        ->then([](const std::string& result) {
            std::cout << result << std::endl; // Không chạy vì Promise trước bị reject
            return std::string("Should not print");
        })
        ->catchError([](std::exception_ptr e) {
            try {
                if (e) std::rethrow_exception(e);
            } catch (const std::exception& ex) {
                std::cerr << "Đã xảy ra lỗi: " << ex.what() << std::endl;
            }
        })
        ->finally([]() {
            std::cout << "Kết thúc quá trình xử lý." << std::endl;
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EventLoop::getInstance().stop();
    return 0;
}

Kết quả này là **đúng** và hoàn toàn giống hành vi Promise trong JavaScript:

### Giải thích luồng thực thi:
1. **fetchData(true)** trả về Promise resolve, nên:
   - `then` đầu tiên chạy, in ra `"Dữ liệu lấy thành công!"` và trả về `fetchData(false)` (Promise bị reject).
2. **Promise trả về từ then** là một Promise bị reject, nên:
   - `then` tiếp theo **không chạy** (vì Promise trước bị reject).
   - Chuyển sang `catchError`, in ra `"Đã xảy ra lỗi: Lỗi lấy dữ liệu!"`.
3. **finally** luôn chạy, in ra `"Kết thúc quá trình xử lý."`.

### Vì sao lại có cả `"Dữ liệu lấy thành công!"` và `"Đã xảy ra lỗi: ..."`?
- Promise đầu tiên thành công, nên in `"Dữ liệu lấy thành công!"`.
- Promise tiếp theo bị reject, nên in `"Đã xảy ra lỗi: ..."`.
- finally luôn chạy.

### Đúng với JS Promise:
```js
fetchData(true)
  .then(...) // in ra thành công
  .then(...) // không chạy vì promise trước bị reject
  .catch(...) // in ra lỗi
  .finally(...) // luôn chạy
```

**Tóm lại:**  
Output của bạn là hoàn toàn đúng với logic Promise chain như JavaScript. Không có lỗi gì cả!

###
Q: toi tuong 1 promise thi co 1 then. vay chi can 1 ham callback gan voi 1 promise la du, tai sao can 1 vector then_callbacks cho 1 promise

A: Trong chuẩn Promise/A+ (và cả JavaScript), **mỗi Promise có thể có nhiều then() được gọi trên nó** – tức là bạn có thể attach nhiều callback vào cùng một promise object, ví dụ:

```js
let p = new Promise(...);
p.then(cb1);
p.then(cb2);
p.then(cb3);
```

Tất cả các callback cb1, cb2, cb3 đều phải được gọi khi promise settle, **dù promise đã resolve trước hay sau khi attach**.

**Vì vậy, cần 1 vector (danh sách) then_callbacks_ cho mỗi promise** để lưu trữ tất cả các callback được attach qua then(). Khi promise resolve, nó sẽ duyệt và gọi tất cả các callback này.

Nếu chỉ dùng 1 callback duy nhất:
- Chỉ callback cuối cùng được attach sẽ được gọi, các callback trước đó sẽ bị ghi đè (sai chuẩn).
- Không hỗ trợ trường hợp nhiều then() trên cùng một promise.

**Tóm lại:**
- then_callbacks_ là vector để đảm bảo mọi callback then đều được gọi đúng chuẩn Promise/A+.
- Mỗi promise object có vector riêng, không dùng chung.
- Việc này cũng áp dụng cho catch_callbacks_ và finally (nếu muốn nhiều finally).

**Tham khảo:**  
- [Promise/A+ 2.2.6](https://promisesaplus.com/#point-36): "then may be called multiple times on the same promise."
- JavaScript: `p.then(...); p.then(...);` đều chạy.

**Kết luận:**  
Cần vector then_callbacks_ để hỗ trợ nhiều callback then trên cùng một promise, đúng chuẩn Promise/A+.

###
Q: khi add finally trong code hien tai, thi finally callback duoc attach vao promise nao. Promise khoi tao dau tien hay cuoi cung, lam sao debug duoc finally callback duoc them vao promise nao

A: 
Trong code Promise hiện tại của bạn, **mỗi lần gọi then/catchError/finally đều tạo ra một promise mới** (giống chuẩn Promise/A+ và JavaScript). Khi bạn gọi:

```cpp
fetchData()
    ->then(...)
    ->then(...)
    ->catchError(...)
    ->finally(...);
```

- Mỗi then/catchError/finally trả về một promise mới (newPromise).
- Callback finally **được attach vào promise cuối cùng trong chuỗi** (tức là promise trả về từ catchError, hoặc then nếu không có catchError).

**Cụ thể:**
- Khi bạn gọi `->finally(cb)`, callback cb sẽ được attach vào promise object mà bạn gọi finally trên nó (promise cuối cùng của chuỗi chain).
- Nó KHÔNG attach vào promise đầu tiên (promise gốc từ fetchData), mà vào promise cuối cùng của chain.

---

### Cách debug: finally callback được attach vào promise nào?

**Cách 1: In địa chỉ this**
- Thêm log in ra địa chỉ của promise hiện tại trong finally:
```cpp
std::cout << "[finally] Attach to promise at: " << this << std::endl;
```

**Cách 2: In địa chỉ promise trong từng then/catch/finally**
- Trong then/catchError/finally, in ra địa chỉ promise hiện tại:
```cpp
std::cout << "[then] this=" << this << std::endl;
std::cout << "[catchError] this=" << this << std::endl;
std::cout << "[finally] this=" << this << std::endl;
```

**Cách 3: In địa chỉ newPromise**
- Khi tạo newPromise trong then/catchError/finally, in ra địa chỉ:
```cpp
std::cout << "[finally] newPromise=" << newPromise.get() << std::endl;
```

---

### Kết luận

- **finally callback luôn được attach vào promise cuối cùng trong chain** (promise trả về từ catchError hoặc then cuối cùng).
- Để debug, chỉ cần in ra địa chỉ `this` trong finally và so sánh với các promise trong chain.
- Nếu bạn muốn kiểm tra callback được attach vào promise nào, hãy thêm log địa chỉ như trên vào hàm finally.

Nếu bạn muốn mình tự động chèn log này vào file, hãy xác nhận!

------------------------------
Q:
ban bao finally luon gan vao promise cuoi cung. vay trong code hien tai co phai promise cuoi la khi goi catchError khong?
dieu gi xay ra neu catchError khong duoc goi? vi du promise resolve thanh cong

A:

Trong code Promise hiện tại của bạn:

- finally luôn được gắn vào promise trả về từ lệnh gọi cuối cùng trong chuỗi (có thể là then, catchError, hoặc finally trước đó).
- Nếu bạn gọi catchError, thì finally sẽ gắn vào promise trả về từ catchError (tức là promise cuối cùng trong chuỗi).
- Nếu bạn KHÔNG gọi catchError, finally sẽ gắn vào promise trả về từ then cuối cùng.

Ví dụ:
```cpp
fetchData()
    ->then(...A...)
    ->then(...B...)
    ->finally(...F...);
```
Ở đây, finally sẽ gắn vào promise trả về từ then(...B...), tức là promise cuối cùng trong chuỗi.

Nếu bạn có:
```cpp
fetchData()
    ->then(...A...)
    ->catchError(...C...)
    ->finally(...F...);
```
finally sẽ gắn vào promise trả về từ catchError(...C...).

**Nếu catchError không được gọi và promise resolve thành công:**  
- finally vẫn sẽ được gọi khi promise settle (resolve).
- finally sẽ chạy sau tất cả các then, và chỉ chạy một lần, đúng chuẩn Promise/A+.

**Tóm lại:**  
- finally luôn gắn vào promise cuối cùng trong chuỗi gọi (dù là then hay catchError).
- finally luôn được thực thi khi promise settle (resolve hoặc reject), không phụ thuộc vào việc có gọi catchError hay không.

Bạn có thể xác nhận điều này bằng log địa chỉ promise trong các hàm then, catchError, finally mà tôi đã thêm.

Trong code hiện tại, điều kiện "finally luôn được thực thi khi promise settle, không phụ thuộc vào việc có gọi catchError hay không" được thể hiện rõ nhất ở phần cài đặt hàm `finally` của lớp `Promise<T>` (và tương tự với `Promise<void>`):

```cpp
template <typename F>
auto finally(F cb) {
    auto self = this->shared_from_this();
    auto newPromise = std::make_shared<Promise<T>>();
    std::cout << "[finally] Attach finally-callback: this=" << this << ", newPromise=" << newPromise.get() << std::endl;
    auto exec = [self, cb, newPromise]() {
        std::cout << "[finally] Execute finally-callback: self=" << self.get() << ", newPromise=" << newPromise.get() << ", state=" << (int)self->state_ << std::endl;
        try { cb(); } catch (...) { /* ignore */ }
        if (self->state_ == State::Resolved) {
            if constexpr (!std::is_void<T>::value)
                newPromise->resolve(self->value_);
            else
                newPromise->resolve();
        } else if (self->state_ == State::Rejected) {
            newPromise->reject(self->exception_);
        }
    };
    if (state_ == State::Resolved || state_ == State::Rejected) {
        std::cout << "[finally] Promise already settled, enqueue finally-callback immediately, state=" << (int)state_ << std::endl;
        EventLoop::getInstance().enqueue(exec);
    } else {
        std::cout << "[finally] Promise pending, push finally-callback to then/catch, state=" << (int)state_ << std::endl;
        then_callbacks_.push_back(exec);
        catch_callbacks_.push_back(exec);
    }
    return newPromise;
}
```

**Giải thích:**
- Nếu promise đã settle (Resolved hoặc Rejected), callback finally sẽ được enqueue thực thi ngay lập tức, bất kể có catchError hay không.
- Nếu promise chưa settle, callback finally sẽ được push vào cả then_callbacks_ và catch_callbacks_. Điều này đảm bảo finally sẽ được gọi khi promise settle theo bất kỳ hướng nào (resolve hoặc reject).
- Việc này hoàn toàn độc lập với việc bạn có gọi catchError hay không.

**Tóm lại:**  
Chính việc always enqueue hoặc push finally-callback vào cả then_callbacks_ và catch_callbacks_ là cơ chế đảm bảo finally luôn được thực thi khi promise settle, không phụ thuộc vào catchError.


--------------------------------------
Q: 
A: (fixed solution)
Dựa vào log và code, promise `0x55f540cc4ac0` là promise trả về từ `catchError` trong chuỗi:
```
... ->then(...) ->then(...) ->catchError(...) ->finally(...)
```
Và callback của `finally` được gắn vào promise này (`this=0x55f540cc4ac0`).

**Vấn đề:**  
Trong log, không có bất kỳ dòng nào cho thấy promise `0x55f540cc4ac0` được resolve/reject (`[Promise<void>::resolve] (INVOKED) on: ...` hoặc `[Promise<void>::reject] (INVOKED) on: ...`), nên callback của finally không bao giờ được thực thi.

---

## Phân tích nguyên nhân

### 1. Callback exec của catchError không được thực thi
- Khi promise trước catchError (`0x55f540cc49b0`) được resolve, callback exec của catchError (gắn vào `catch_callbacks_` của `0x55f540cc49b0`) chỉ được gọi nếu promise bị reject.
- Nhưng khi promise trước được resolve, trong exec của catchError:
  ```cpp
  if (self->state_ != State::Rejected) {
      if (self->state_ == State::Resolved) {
          if constexpr (std::is_void<ReturnType>::value) {
              newPromise->resolve();
          }
      }
      return;
  }
  ```
  => newPromise (tức là `0x55f540cc4ac0`) sẽ được resolve.

### 2. Tuy nhiên, **exec này chỉ được enqueue nếu promise trước đã settled khi gọi catchError**  
- Nếu promise trước đã settled (resolved) khi gọi catchError, exec sẽ được enqueue ngay lập tức.
- Nếu promise trước chưa settled, exec sẽ được push vào `catch_callbacks_` của promise trước, và chỉ được gọi khi promise trước bị reject.

### 3. **Vấn đề thực tế:**  
- Khi promise trước (`0x55f540cc49b0`) được resolve, nó chỉ thực thi các callback trong `then_callbacks_`, KHÔNG thực thi các callback trong `catch_callbacks_`.
- Do đó, callback exec của catchError (gắn vào `catch_callbacks_`) sẽ không bao giờ được gọi nếu promise trước resolve thành công.

---

## Kết luận:  
**Promise/A+ chuẩn:**  
- Khi promise trước resolve, catchError phải chuyển trạng thái cho promise sau (resolve luôn), để finally được gọi.
- Nhưng code hiện tại chỉ thực thi `then_callbacks_` khi resolve, và chỉ thực thi `catch_callbacks_` khi reject.

**=> Callback exec của catchError sẽ không bao giờ được gọi nếu promise trước resolve thành công!**

---

## Cách khắc phục

**Giải pháp:**  
- Khi resolve, ngoài việc thực thi `then_callbacks_`, cần thực thi luôn các callback trong `catch_callbacks_` (với promise<void>).
- Hoặc, khi gọi catchError, nếu promise trước đã resolve, phải enqueue exec ngay lập tức (đã làm đúng).
- Nhưng nếu promise trước chưa settled, callback exec phải được push vào cả `then_callbacks_` và `catch_callbacks_` (giống như finally), để đảm bảo luôn được gọi khi promise settle.

**=> Sửa catchError cho Promise<void>:**
- Nếu promise chưa settled, push exec vào cả `then_callbacks_` và `catch_callbacks_`.

---

## Đề xuất sửa code

Sửa catchError cho Promise<void> như sau:
```cpp
bool settled = false;
{
    std::lock_guard<std::mutex> lock(callback_mutex_);
    settled = (state_ == State::Resolved || state_ == State::Rejected);
    if (!settled) {
        // Đảm bảo exec luôn được gọi khi promise settle
        then_callbacks_.push_back(exec);
        catch_callbacks_.push_back(exec);
        return newPromise;
    }
}
EventLoop::getInstance().enqueue(exec);
return newPromise;
```

**Như vậy, promise 0x55f540cc4ac0 sẽ luôn được resolve khi promise trước resolve, và finally sẽ luôn được gọi.**

