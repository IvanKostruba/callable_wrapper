#pragma once

#include <memory>
#include <type_traits>

namespace _detail {

	// Definition of self-made virtual table. It holds several function pointers.
	struct vtable {
		// Main part of the logic, this will invoke the stored callable.
		void (*run)(void* ptr);
		// These are necessary for correct copy and destruction of the stored
		// objects.
		void (*destroy)(void* ptr);
		void (*clone)(void* storage, const void* ptr);
		void (*move_clone)(void* storage, void* ptr);
	};

	// Template variable from C++17. We instantiate vtable, initializing the
	// pointers with lambdas that decay to function pointers because they have
	// empty capture lists.
	template<typename Callable>
	constexpr vtable vtable_for{
		[](void* ptr) {
			// Inside each of the lambdas we restore the type information using
			// the information from the template parameters.
			static_cast<Callable*>(ptr)->operator()();
		},

		// Destructor
		[](void* ptr) {
			std::destroy_at(static_cast<Callable*>(ptr));
		},
		// Copy constructor
		[](void* storage, const void* ptr) {
			new (storage) Callable {
				*static_cast<const Callable*>(ptr)};
		},
		// Move constructor
		[](void* storage, void* ptr) {
			new (storage) Callable {
				std::move(*static_cast<Callable*>(ptr))};
		}
	};

}; // namespace _detail

class TaskWrapper {
public:
	TaskWrapper() : vtable_{ nullptr }
	{}

	TaskWrapper(const TaskWrapper& other) {
		other.vtable_->clone(&buf_, &other.buf_);
		vtable_ = other.vtable_;
	}

	TaskWrapper(TaskWrapper&& other) noexcept {
		other.vtable_->move_clone(&buf_, &other.buf_);
		vtable_ = other.vtable_;
	}

	~TaskWrapper() {
		if (vtable_) {
			vtable_->destroy(&buf_);
		}
	}

	TaskWrapper& operator=(const TaskWrapper& other) {
		if (vtable_) {
			vtable_->destroy(&buf_);
		}
		if (other.vtable_) {
			other.vtable_->clone(&buf_, &other.buf_);
		}
		vtable_ = other.vtable_;
		return *this;
	}

	TaskWrapper& operator=(TaskWrapper&& other) noexcept {
		if (vtable_) {
			vtable_->destroy(&buf_);
		}
		if (other.vtable_) {
			other.vtable_->move_clone(&buf_, &other.buf_);
		}
		vtable_ = other.vtable_;
		return *this;
	}

	// This is where the magic happens. We create a virtual table instance
	// that 'remembers' the type information while the callable is stored
	// by placement new in the internal buffer, e.g. as a type-less set
	// of bytes.
	// Placement new essentially implements small buffer optimization.
	// Instead of allocating on the heap, we store data on the stack.
	// This is how we gain in performance.
	template<typename Callable>
	TaskWrapper(Callable c)
		: vtable_{ &_detail::vtable_for<Callable> }
	{
		static_assert(sizeof(Callable) < sizeof(buf_),
			"Wrapper buffer is too small.");
		new(&buf_) Callable{ std::move(c) };
	}

	// This is where we invoke the stored callable.
	void operator()() {
		if (vtable_) {
			vtable_->run(&buf_);
		}
	}

private:
	// We use aligned storage to ensure that regardless of the size and
	// alignment of the object that we store it will be properly aligned.
	std::aligned_storage_t<64> buf_;
	const _detail::vtable* vtable_;
};
