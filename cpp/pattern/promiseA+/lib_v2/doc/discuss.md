taikt: ban hay giai thich dona code then() chi tiet. ham exec de lam gi?

GitHub Copilot: Hàm `then()` trong lớp `Promise<T>` dùng để đăng ký một callback sẽ được thực thi khi Promise được resolve (hoặc reject, để chuyển lỗi sang Promise tiếp theo).

### Giải thích chi tiết:

- **Khai báo kiểu trả về của callback:**  
  ```cpp
  using U = typename std::invoke_result<F, T>::type;
  using ReturnType = typename promise_value_type<U>::type;
  ```
  - `F` là kiểu của callback truyền vào.
  - `U` là kiểu trả về của callback khi gọi với giá trị của Promise.
  - `ReturnType` là kiểu thực sự của giá trị Promise tiếp theo (nếu callback trả về Promise thì lấy kiểu value_type bên trong).

- **Tạo Promise mới cho chaining:**  
  ```cpp
  auto newPromise = std::make_shared<Promise<ReturnType>>();
  ```
  - Mỗi lần gọi `then()`, sẽ tạo một Promise mới để trả về, dùng cho chain tiếp theo.

- **Tạo lambda `exec`:**  
  ```cpp
  auto exec = [self, cb, newPromise]() { ... }
  ```
  - Đây là hàm sẽ thực thi callback khi Promise hiện tại được resolve hoặc reject.

#### Vai trò của `exec`:
- **Khi Promise bị reject:**  
  - Nếu trạng thái là `Rejected`, chuyển lỗi sang Promise mới bằng `newPromise->reject(self->exception_)`.
- **Khi Promise chưa resolve:**  
  - Nếu chưa resolve, không làm gì cả (chờ resolve).
- **Khi Promise đã resolve:**  
  - Thực thi callback với giá trị của Promise.
  - Nếu callback trả về void, resolve Promise mới luôn.
  - Nếu callback trả về một Promise khác (nested Promise), thì chain tiếp theo sẽ chờ Promise đó resolve/reject rồi mới resolve/reject Promise mới.
  - Nếu callback trả về giá trị thường, resolve Promise mới với giá trị đó.
- **Bắt lỗi:**  
  - Nếu callback ném exception, reject Promise mới với exception đó.

#### Đăng ký thực thi:
- Nếu Promise đã resolve/reject, enqueue `exec` vào EventLoop để thực thi ngay.
- Nếu Promise còn pending, lưu `exec` vào danh sách callback để thực thi sau khi resolve/reject.

---

**Tóm lại:**  
- Hàm `then()` tạo một Promise mới cho chain tiếp theo.
- Hàm `exec` là logic thực thi callback khi Promise hiện tại đã settle (resolve/reject).
- Đảm bảo xử lý đúng các trường hợp: trả về giá trị, trả về Promise, trả về void, và bắt lỗi.