#pragma once

#include <memory>
#include <type_traits>

namespace _detail {

	struct vtable {
		void (*run)(void* ptr);

		void (*destroy)(void* ptr);
		void (*clone)(void* storage, const void* ptr);
		void (*move_clone)(void* storage, void* ptr);
	};


	template<typename Callable>
	constexpr vtable vtable_for{
		[](void* ptr) {
			static_cast<Callable*>(ptr)->operator()();
		},

		[](void* ptr) {
			std::destroy_at(static_cast<Callable*>(ptr));
		},
		[](void* storage, const void* ptr) {
			new (storage) Callable {
				*static_cast<const Callable*>(ptr)};
		},
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

	template<typename Callable>
	TaskWrapper(Callable c)
		: vtable_{ &_detail::vtable_for<Callable> }
	{
		static_assert(sizeof(Callable) < sizeof(buf_),
			"Wrapper buffer is too small.");
		new(&buf_) Callable{ std::move(c) };
	}

	void operator()() {
		vtable_->run(&buf_);
	}

private:
	std::aligned_storage_t<64> buf_;
	const _detail::vtable* vtable_;
};
