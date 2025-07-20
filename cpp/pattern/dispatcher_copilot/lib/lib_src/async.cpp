#include <functional>
#include <memory>
#include <exception>
#include <type_traits>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <future>

// Simple EventLoop for async execution
class EventLoop {
public:
    static EventLoop& getInstance() {
        static EventLoop instance;
        return instance;
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cond_.notify_one();
    }

    // Không còn pending_count_, không tự stop nữa
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cond_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

    ~EventLoop() {
        stop(); // Đảm bảo join thread khi app exit
    }

private:
    EventLoop() : stopped_(false) {
        worker_ = std::thread([this]() { this->run(); });
    }

    void run() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_.wait(lock, [this] { return stopped_ || !tasks_.empty(); });
                if (stopped_ && tasks_.empty()) break;
                if (tasks_.empty()) continue;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            try {
                task();
            } catch (const std::exception& ex) {
                std::cerr << "Uncaught exception in EventLoop: " << ex.what() << std::endl;
            } catch (...) {
                std::cerr << "Uncaught unknown exception in EventLoop!" << std::endl;
            }
        }
    }

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread worker_;
    bool stopped_;
};

// Forward declaration of Promise
template <typename T> class Promise;

// Type trait to check if a type is std::shared_ptr<Promise<V>>
template <typename X>
struct is_nested_promise : std::false_type {};
template <typename V>
struct is_nested_promise<std::shared_ptr<Promise<V>>> : std::true_type {};

// Helper trait to safely extract value_type from U::element_type
template <typename U, bool IsNested = is_nested_promise<U>::value>
struct promise_value_type {
    using type = U;
};
template <typename U>
struct promise_value_type<U, true> {
    using type = typename U::element_type::value_type;
};

// Promise class
template <typename T>
class Promise : public std::enable_shared_from_this<Promise<T>> {
public:
    using value_type = T;
    using Resolver = std::function<void(T)>;
    using Rejecter = std::function<void(std::exception_ptr)>;

private:
    enum class State { Pending, Resolved, Rejected };
    State state_ = State::Pending;
    T value_;
    std::exception_ptr exception_;
    std::vector<std::function<void()>> then_callbacks_;
    std::vector<std::function<void()>> catch_callbacks_;
    std::weak_ptr<Promise<T>> next_promise_;

public:
    Promise() = default;
    ~Promise() = default;

    void resolve(T value) {
        if (state_ != State::Pending) return;
        auto self = this->shared_from_this();
        EventLoop::getInstance().enqueue([self, value = std::move(value)]() mutable {
            if (self->state_ != State::Pending) return;
            self->state_ = State::Resolved;
            self->value_ = std::move(value);
            auto callbacks = std::move(self->then_callbacks_);
            self->then_callbacks_.clear();
            for (auto& cb : callbacks) {
                EventLoop::getInstance().enqueue(cb);
            }
        });
    }

    void reject(std::exception_ptr e) {
        if (state_ != State::Pending) return;
        auto self = this->shared_from_this();
        EventLoop::getInstance().enqueue([self, e]() {
            if (self->state_ != State::Pending) return;
            self->state_ = State::Rejected;
            self->exception_ = e;
            auto catch_cbs = std::move(self->catch_callbacks_);
            self->catch_callbacks_.clear();
            auto then_cbs = std::move(self->then_callbacks_);
            self->then_callbacks_.clear();
            if (catch_cbs.empty() && then_cbs.empty()) {
                self->propagate_rejection();
            } else {
                for (auto& cb : catch_cbs) {
                    EventLoop::getInstance().enqueue(cb);
                }
                // Đảm bảo finally luôn chạy kể cả khi không có catchError
                for (auto& cb : then_cbs) {
                    EventLoop::getInstance().enqueue(cb);
                }
            }
        });
    }

    template <typename F>
    auto then(F cb) {
        using U = typename std::invoke_result<F, T>::type;
        using ReturnType = typename promise_value_type<U>::type;

        std::cout << "[Promise<void>::then] About to create new Promise<void> (for chaining)" << std::endl;
        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();
        std::cout << "[then] Attach then-callback: this=" << this << ", newPromise=" << newPromise.get() << std::endl;
        // Chỉ gán next_promise_ nếu kiểu giống nhau
        if constexpr (std::is_same<Promise<ReturnType>, Promise<T>>::value) {
            this->next_promise_ = newPromise;
        }
        auto exec = [self, cb, newPromise]() {
            std::cout << "[then] Execute then-callback: self=" << self.get() << ", newPromise=" << newPromise.get() << std::endl;
            if (self->state_ == State::Rejected) {
                newPromise->reject(self->exception_);
                return;
            }
            if (self->state_ != State::Resolved) return;
            try {
                if constexpr (std::is_void<U>::value) {
                    cb(self->value_);
                    if constexpr (std::is_void<ReturnType>::value) {
                        newPromise->resolve();
                    }
                } else if constexpr (is_nested_promise<U>::value) {
                    auto nestedPromise = cb(self->value_);
                    using NestedValue = typename promise_value_type<U>::type;
                    if constexpr (std::is_void<NestedValue>::value) {
                        nestedPromise->then([newPromise]() {
                            newPromise->resolve();
                        });
                    } else {
                        nestedPromise->then([newPromise](auto&& nextValue) {
                            newPromise->resolve(std::forward<decltype(nextValue)>(nextValue));
                        });
                    }
                    nestedPromise->catchError([newPromise](const std::exception_ptr& e) {
                        newPromise->reject(e);
                    });
                } else {
                    auto result = cb(self->value_);
                    newPromise->resolve(result);
                }
            } catch (...) {
                newPromise->reject(std::current_exception());
            }
        };
        if (state_ == State::Resolved || state_ == State::Rejected) {
            EventLoop::getInstance().enqueue(exec);
        } else {
            then_callbacks_.push_back(exec);
        }
        return newPromise;
    }

    // catchError cho callback trả về giá trị hoặc Promise (chuẩn Promise A+)
    template <typename F>
    auto catchError(F cb) {
        using U = std::invoke_result_t<F, std::exception_ptr>;
        using ReturnType = typename promise_value_type<U>::type;

        std::cout << "[Promise<void>::catchError] About to create new Promise<void> (for chaining)" << std::endl;
        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();
        std::cout << "[catchError] Attach catch-callback: this=" << this << ", newPromise=" << newPromise.get() << std::endl;
        // Chỉ gán next_promise_ nếu kiểu giống nhau
        if constexpr (std::is_same<Promise<ReturnType>, Promise<T>>::value) {
            this->next_promise_ = newPromise;
        }
        auto exec = [self, cb, newPromise]() {
            std::cout << "[catchError] Execute catch-callback: self=" << self.get() << ", newPromise=" << newPromise.get() << std::endl;
            if (self->state_ != State::Rejected) {
                if (self->state_ == State::Resolved) {
                    if constexpr (!std::is_void<T>::value && !std::is_void<ReturnType>::value) {
                        newPromise->resolve(self->value_);
                    } else if constexpr (std::is_void<ReturnType>::value) {
                        newPromise->resolve();
                    }
                }
                return;
            }
            try {
                if constexpr (std::is_void<U>::value) {
                    cb(self->exception_);
                    if constexpr (std::is_void<ReturnType>::value) {
                        newPromise->resolve();
                    }
                } else if constexpr (is_nested_promise<U>::value) {
                    auto nestedPromise = cb(self->exception_);
                    using NestedValue = typename promise_value_type<U>::type;
                    if constexpr (std::is_void<NestedValue>::value) {
                        nestedPromise->then([newPromise]() {
                            newPromise->resolve();
                        });
                    } else {
                        nestedPromise->then([newPromise](auto&& nextValue) {
                            newPromise->resolve(std::forward<decltype(nextValue)>(nextValue));
                        });
                    }
                    nestedPromise->catchError([newPromise](const std::exception_ptr& e) {
                        newPromise->reject(e);
                    });
                } else {
                    auto result = cb(self->exception_);
                    newPromise->resolve(result);
                }
            } catch (...) {
                newPromise->reject(std::current_exception());
            }
        };
        if (state_ == State::Resolved || state_ == State::Rejected) {
            EventLoop::getInstance().enqueue(exec);
        } else {
            catch_callbacks_.push_back(exec);
        }
        return newPromise;
    }

    // finally() cho Promise<T>
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
        }
        return newPromise;
    }

    // Propagate rejection to next promise in chain if not handled
    void propagate_rejection() {
        if (auto next = next_promise_.lock()) {
            next->reject(exception_);
        }
    }
};

// Specialization for Promise<void>
// Promise<void> (cleaned, concise, correct Promise/A+)
template <>
class Promise<void> : public std::enable_shared_from_this<Promise<void>> {
public:
    using value_type = void;
    using Resolver = std::function<void()>;
    using Rejecter = std::function<void(std::exception_ptr)>;
private:
    enum class State { Pending, Resolved, Rejected };
    State state_ = State::Pending;
    std::exception_ptr exception_;
    std::vector<std::function<void()>> then_callbacks_;
    std::vector<std::function<void()>> catch_callbacks_;
    std::mutex callback_mutex_;
    bool callbacks_moved_ = false;
public:
    Promise() = default;
    ~Promise() = default;
    void resolve() {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (state_ != State::Pending) return;
        state_ = State::Resolved;
        callbacks_moved_ = true;
        auto thens = std::move(then_callbacks_);
        auto catchs = std::move(catch_callbacks_);
        then_callbacks_.clear();
        catch_callbacks_.clear();
        for (auto& cb : thens) EventLoop::getInstance().enqueue(cb);
        for (auto& cb : catchs) EventLoop::getInstance().enqueue(cb);
    }
    void reject(std::exception_ptr e) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (state_ != State::Pending) return;
        state_ = State::Rejected;
        exception_ = e;
        callbacks_moved_ = true;
        auto thens = std::move(then_callbacks_);
        auto catchs = std::move(catch_callbacks_);
        then_callbacks_.clear();
        catch_callbacks_.clear();
        for (auto& cb : catchs) EventLoop::getInstance().enqueue(cb);
        for (auto& cb : thens) EventLoop::getInstance().enqueue(cb);
    }
    template <typename F>
    auto then(F cb) {
        using U = std::invoke_result_t<F>;
        using ReturnType = typename promise_value_type<U>::type;
        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();
        auto exec = [self, cb, newPromise]() {
            if (self->state_ == State::Rejected) { newPromise->reject(self->exception_); return; }
            if (self->state_ != State::Resolved) return;
            try {
                if constexpr (std::is_void<U>::value) { cb(); newPromise->resolve(); }
                else if constexpr (is_nested_promise<U>::value) {
                    auto nested = cb();
                    nested->then([newPromise]() { newPromise->resolve(); });
                    nested->catchError([newPromise](const std::exception_ptr& e) { newPromise->reject(e); });
                } else { newPromise->resolve(cb()); }
            } catch (...) { newPromise->reject(std::current_exception()); }
        };
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (state_ == State::Pending && !callbacks_moved_) then_callbacks_.push_back(exec);
            else EventLoop::getInstance().enqueue(exec);
        }
        return newPromise;
    }
    template <typename F>
    auto catchError(F cb) {
        using U = std::invoke_result_t<F, std::exception_ptr>;
        using ReturnType = typename promise_value_type<U>::type;
        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();
        auto exec = [self, cb, newPromise]() {
            if (self->state_ != State::Rejected) { if (self->state_ == State::Resolved) newPromise->resolve(); return; }
            try {
                if constexpr (std::is_void<U>::value) { cb(self->exception_); newPromise->resolve(); }
                else if constexpr (is_nested_promise<U>::value) {
                    auto nested = cb(self->exception_);
                    nested->then([newPromise]() { newPromise->resolve(); });
                    nested->catchError([newPromise](const std::exception_ptr& e) { newPromise->reject(e); });
                } else { newPromise->resolve(cb(self->exception_)); }
            } catch (...) { newPromise->reject(std::current_exception()); }
        };
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (state_ == State::Pending && !callbacks_moved_) {
                catch_callbacks_.push_back(exec);
            } else EventLoop::getInstance().enqueue(exec);
        }
        return newPromise;
    }
    template <typename F>
    auto finally(F cb) {
        auto self = this->shared_from_this();
        auto newPromise = std::make_shared<Promise<void>>();
        auto exec = [self, cb, newPromise]() {
            try { cb(); } catch (...) {}
            if (self->state_ == State::Resolved) newPromise->resolve();
            else if (self->state_ == State::Rejected) newPromise->reject(self->exception_);
        };
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (state_ == State::Pending && !callbacks_moved_) {
                then_callbacks_.push_back(exec);
            } else EventLoop::getInstance().enqueue(exec);
        }
        return newPromise;
    }
};

// Helper function giống fetchData trong JS
std::shared_ptr<Promise<std::string>> fetchData() {
    auto p = std::make_shared<Promise<std::string>>();
    std::thread([p]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        // Mô phỏng lấy dữ liệu: random thành công/thất bại
        //bool succeed = (std::rand() % 2 == 0); // 50% thành công, 50% thất bại
        bool succeed = true; // Giả sử thành công
        if (succeed) {
            std::string data = "Dữ liệu lấy thành công!";
            std::cout << "[fetchData] Output: " << data << std::endl;
            p->resolve(data);
        } else {
            std::string err = "FETCH_ERROR: Không lấy được dữ liệu từ server!";
            std::cout << "[fetchData] Output: " << err << std::endl;
            p->reject(std::make_exception_ptr(std::runtime_error(err)));
        }
    }).detach();
    return p;
}

// Main for testing
int main() {
    std::cout << "Bắt đầu lấy dữ liệu..." << std::endl;
    /*
    fetchData()
        ->then([](const std::string& result) {
            std::cout << "Kết quả: " << result << std::endl;
            throw std::runtime_error("Lỗi phát sinh trong then!");
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
    */
    fetchData()
        ->then([](const std::string& result) {
            std::cout << "Kết quả: " << result << std::endl;
            // Test cả resolve và reject:
            // throw std::runtime_error("Lỗi phát sinh trong then!"); // Bỏ comment để test reject
            return static_cast<int>(result.length());
        })
        ->then([](int charCount) {
            std::cout << "Bước 2: Đếm được " << charCount << " ký tự" << std::endl;
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
    std::cout << "Main running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5)); // Đợi cho các promise hoàn thành
    return 0;
}