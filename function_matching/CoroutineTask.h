#pragma once

#include <coroutine>

// This is all the necessary boilerplate you need to transform a function
// into a coroutine. This object has to be the coroutine's return type. 

// This wrapper object is not necessary according to the standard, but
// MSVC compiler is not happy without it. It should be possible to
// return a coroutine_handle instance directly with GCC or LLVM.
struct CoroutineTask {
	// The mandatory element of the coroutine return type - 'promise_type'.
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type {
		CoroutineTask get_return_object() {
			return { handle_type::from_promise(*this) };
		}
		// We want our coroutine to be suspended on creation and resume it
		// later in the worker thread.
		std::suspend_always initial_suspend() noexcept { return {}; }
		// We don't want to suspend the coroutine when it reaches co_return.
		std::suspend_never final_suspend() noexcept { return {}; }
		// Our coroutine does not return any values, so it's promise_type
		// defines the 'return_void' method, otherwise 'return_value' is necessary.
		void return_void() {}
		void unhandled_exception() {}
	};

	// This handle will be used in the worker thread to resume the coroutine.
	handle_type h_;
};
