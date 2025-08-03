"Boost không always async" nghĩa là:  
**Callback của `boost::future::then` không luôn luôn được thực thi bất đồng bộ (async) trên event loop hoặc thread khác.**

Cụ thể:
- Nếu bạn dùng `boost::future::then` mà **không chỉ định executor**, callback có thể được gọi **ngay lập tức** (đồng bộ, trên thread hiện tại) nếu future đã sẵn sàng.
- Điều này **khác với Promise/A+** (JS), nơi callback `.then()` **luôn luôn** được gọi bất đồng bộ (luôn chạy sau tick hiện tại, trên event loop).

**Hệ quả:**  
- Với Boost, nếu bạn chaining nhiều `.then()`, có thể một số callback sẽ chạy ngay trên thread gọi `.then()`, số khác lại chạy trên thread fulfill promise, dẫn đến hành vi không nhất quán và khó kiểm soát về concurrency.

**Tóm lại:**  
- "Not always async" = Không đảm bảo callback luôn chạy bất đồng bộ trên event loop/thread riêng, khác với chuẩn Promise/A+.


toi tuong trong boost neu khai bao executor chi 1 thread event duy nhat, thi ngam dinh moi then() deu phai chay tren executor thread do?

Không, trong **Boost** (cụ thể là `boost::future::then`), **callback chỉ chạy trên executor thread nếu bạn chỉ định rõ executor cho từng lần gọi `.then()`**. Nếu bạn không chỉ định executor, callback có thể chạy trên thread hiện tại, thread fulfill promise, hoặc thread khác—không đảm bảo luôn chạy trên executor thread, dù executor chỉ có 1 thread.

**Ví dụ:**
```cpp
boost::asio::io_context io;
auto fut = boost::async(boost::launch::async, [] { return 42; });

fut.then(boost::asio::bind_executor(io.get_executor()), [](auto f) {
    // Callback này chắc chắn chạy trên thread của io_context
});
```
- Nếu **không dùng `bind_executor`**, callback có thể chạy trên bất kỳ thread nào.

**Tóm lại:**  
- **Boost không mặc định** cho callback chạy trên executor thread, dù executor chỉ có 1 thread.
- **Bạn phải chỉ định executor** cho từng `.then()` nếu muốn đảm bảo callback luôn chạy trên thread đó.  
- Đây là điểm khác biệt với Promise/A+ (JS), nơi callback luôn chạy trên event loop.
