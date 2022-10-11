#pragma once

#include <type_traits>

struct vtable {
	void (*run)(void* ptr);
};


template<typename Callable>
constexpr vtable vtable_for{
	[](void* ptr) {
		static_cast<Callable*>(ptr)->operator()();
	}
};


class TaskWrapper {
public:
	TaskWrapper() : vtable_{ nullptr }
	{}

	template<typename Callable>
	TaskWrapper(Callable c)
		: vtable_{ &vtable_for<Callable> }
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
	const vtable* vtable_;
};
