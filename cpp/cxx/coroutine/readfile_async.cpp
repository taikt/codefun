#include <coroutine>
#include <iostream>
#include <thread>

// https://cppuniverse.com/EverydayCpp20/CoroutinesAsync

struct AsyncFileIOLibrary {
	~AsyncFileIOLibrary() {
		done = true;
		thread.join();
	}

	auto async_open_file() {
		struct awaiter {
			AsyncFileIOLibrary& lib;
			bool await_ready() const noexcept { return false; }
			void await_resume() const noexcept { }
			void await_suspend(std::coroutine_handle<> handle) {
				std::lock_guard guard(lib.mutex);
				lib.work = [handle, &lib = lib]() mutable {
					lib.outfile.open("dummy.txt", std::fstream::app);
					handle.resume();
				};
			}
		};
		return awaiter { *this };
	}

	auto async_read_file() {
		struct awaiter {
			AsyncFileIOLibrary& lib;
			bool await_ready() const noexcept { return false; }
			void await_resume() const noexcept { }
			void await_suspend(std::coroutine_handle<> handle) {
				std::lock_guard guard(lib.mutex);
				lib.work = [handle, this]() mutable {
					std::string line;
					for (; std::getline(lib.outfile, line);) {
						lib.lines.push_back(line);
					}
					handle.resume();
				};
			}
		};
		return awaiter { *this };
	};

	std::fstream outfile;
	std::vector<std::string> lines;
	std::atomic<bool> done { false };
	std::function<void()> work;
	std::mutex mutex;

	std::thread thread { [this] {
		while (!done) {
			std::unique_lock lk(mutex);
			// Check if work is ready
			if (work) {
				auto copy = work;
				work = nullptr;
				lk.unlock();
				copy();
			}
			std::this_thread::yield();
		}
	} };
};

struct Task {
	struct promise_type {
		std::suspend_never initial_suspend() { return {}; };
		std::suspend_never final_suspend() noexcept { return {}; };
		void unhandled_exception() {};
		void return_void() { }
		auto get_return_object() { return Task {}; };
	};
};

std::atomic<bool> finished { false };

Task read_modify_write_async(AsyncFileIOLibrary& lib) {
	co_await lib.async_open_file();
	co_await lib.async_read_file();
	finished = true;
}

int main() {
	AsyncFileIOLibrary lib;
	read_modify_write_async(lib); // non-blocking
	while (!finished) {
		// ... do other operations
	}
	std::cout << "Done" << std::endl;
}