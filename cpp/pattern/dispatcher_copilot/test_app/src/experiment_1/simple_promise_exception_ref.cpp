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

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cond_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

private:
    EventLoop() : stopped_(false) {
        worker_ = std::thread([this]() { this->run(); });
        worker_.detach();
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
            auto callbacks = std::move(self->catch_callbacks_);
            self->catch_callbacks_.clear();
            if (callbacks.empty()) {
                // propagate rejection if no catch handler
                self->propagate_rejection();
            } else {
                for (auto& cb : callbacks) {
                    EventLoop::getInstance().enqueue(cb);
                }
            }
        });
    }

    template <typename F>
    auto then(F cb) {
        using U = typename std::invoke_result<F, T>::type;
        using ReturnType = typename promise_value_type<U>::type;

        auto newPromise = std::make_shared<Promise<ReturnType>>();
        newPromise->next_promise_ = std::weak_ptr<Promise<ReturnType>>(); // clear next for new
        auto self = this->shared_from_this();
        // Link next_promise_ for rejection propagation
        this->next_promise_ = newPromise;

        auto exec = [self, cb, newPromise]() {
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

        auto newPromise = std::make_shared<Promise<ReturnType>>();
        if constexpr (!std::is_void<ReturnType>::value) {
            newPromise->next_promise_ = std::weak_ptr<Promise<ReturnType>>();
            this->next_promise_ = newPromise;
        }
        auto self = this->shared_from_this();

        auto exec = [self, cb, newPromise]() {
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

    // Propagate rejection to next promise in chain if not handled
    void propagate_rejection() {
        if (auto next = next_promise_.lock()) {
            next->reject(exception_);
        }
    }
};

// Specialization for Promise<void>
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
    std::weak_ptr<Promise<void>> next_promise_; // Thêm biến này để đồng bộ với Promise<T>

public:
    Promise() = default;

    void resolve() {
        if (state_ != State::Pending) return;
        auto self = this->shared_from_this();
        EventLoop::getInstance().enqueue([self]() {
            if (self->state_ != State::Pending) return;
            self->state_ = State::Resolved;
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
            auto callbacks = std::move(self->catch_callbacks_);
            self->catch_callbacks_.clear();
            for (auto& cb : callbacks) {
                EventLoop::getInstance().enqueue(cb);
            }
        });
    }

    // then for callback with no argument
    template <typename F>
    auto then(F cb) {
        using U = std::invoke_result_t<F>;
        using ReturnType = typename promise_value_type<U>::type;

        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();

        auto exec = [self, cb, newPromise]() {
            if (self->state_ == State::Rejected) {
                newPromise->reject(self->exception_);
                return;
            }
            if (self->state_ != State::Resolved) return;
            try {
                if constexpr (std::is_void<U>::value) {
                    cb();
                    if constexpr (std::is_void<ReturnType>::value) {
                        newPromise->resolve();
                    }
                } else if constexpr (is_nested_promise<U>::value) {
                    auto nestedPromise = cb();
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
                    auto result = cb();
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

    // then for callback with argument (for nested promise)
    template <typename F, typename Arg>
    auto then(F cb, Arg&& arg) {
        using U = std::invoke_result_t<F, Arg>;
        using ReturnType = typename promise_value_type<U>::type;

        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();

        auto exec = [self, cb, newPromise, arg = std::forward<Arg>(arg)]() mutable {
            if (self->state_ == State::Rejected) {
                newPromise->reject(self->exception_);
                return;
            }
            if (self->state_ != State::Resolved) return;
            try {
                if constexpr (std::is_void<U>::value) {
                    cb(std::forward<Arg>(arg));
                    if constexpr (std::is_void<ReturnType>::value) {
                        newPromise->resolve();
                    }
                } else if constexpr (is_nested_promise<U>::value) {
                    auto nestedPromise = cb(std::forward<Arg>(arg));
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
                    auto result = cb(std::forward<Arg>(arg));
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

        auto newPromise = std::make_shared<Promise<ReturnType>>();
        auto self = this->shared_from_this();

        auto exec = [self, cb, newPromise]() {
            if (self->state_ != State::Rejected) {
                if (self->state_ == State::Resolved) {
                    if constexpr (std::is_void<ReturnType>::value) {
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

    // Propagate rejection to next promise in chain if not handled (đồng bộ với Promise<T>)
    void propagate_rejection() {
        if (auto next = next_promise_.lock()) {
            next->reject(exception_);
        }
    }
};

// Main for testing
int main() {
    auto promise = std::make_shared<Promise<int>>();

    auto chain = promise->then([](int value) {
        std::cout << "Step 1: value = " << value << std::endl;
        //throw std::runtime_error("Error in Step 1"); // Tạo exception để test
        return value + 10;
    })->then([](int value) {
        std::cout << "Step 2: value = " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        throw std::runtime_error("Error in Step 2"); // Tạo exception để test
        return value * 2;
    });

    chain->catchError([](std::exception_ptr e) {
        try {
            if (e) std::rethrow_exception(e);
        } catch (const std::exception& ex) {
            std::cout << "Caught exception: " << ex.what() << std::endl;
        }
    });

    promise->resolve(5);

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    EventLoop::getInstance().stop();
    return 0;
}