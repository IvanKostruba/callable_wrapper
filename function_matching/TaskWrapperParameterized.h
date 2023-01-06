#pragma once

#include <exception>
#include <memory>
#include <type_traits>

// Here you can find a wrapper that can be parameterized just like
// std::function, for example TaskWrapperP<int(float, double)>.

namespace _detail {

	template<typename R>
	struct vtable_p {
		// This is just a base template class, so we can specialize it for a
	    // function signature. This one is not meant to be instantiated.
	};

	template<typename R, typename ...Ts>
	struct vtable_p<R(Ts...)> {
		R(*run)(void* ptr, Ts ...args);

		void (*destroy)(void* ptr);
		void (*clone)(void* storage, const void* ptr);
		void (*move_clone)(void* storage, void* ptr);
	};

	// This template receive return type and arguments list from TaskWrapperP
	// where it's instantiated. They are defined by TaskWrapperP template
	// parameters.
	template<typename Callable, typename R, typename... Ts>
	constexpr vtable_p<R(Ts...)> vtable_p_for{
		[](void* ptr, Ts ...args) -> R {
			return static_cast<Callable*>(ptr)->operator()(args...);
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

template<typename R>
class TaskWrapperP {
	// This is just a base template class, so we can specialize it for a
	// function signature. This one is not meant to be actually instantiated.
};

// Template specialization that matches with function signature.
template<typename R, typename ...Ts>
class TaskWrapperP<R(Ts...)> {
public:
	TaskWrapperP() : vtable_{ nullptr }
	{}

	TaskWrapperP(const TaskWrapperP& other) {
		other.vtable_->clone(&buf_, &other.buf_);
		vtable_ = other.vtable_;
	}

	TaskWrapperP(TaskWrapperP&& other) noexcept {
		other.vtable_->move_clone(&buf_, &other.buf_);
		vtable_ = other.vtable_;
	}

	~TaskWrapperP() {
		if (vtable_) {
			vtable_->destroy(&buf_);
		}
	}

	TaskWrapperP& operator=(const TaskWrapperP& other) {
		if (vtable_) {
			vtable_->destroy(&buf_);
		}
		if (other.vtable_) {
			other.vtable_->clone(&buf_, &other.buf_);
		}
		vtable_ = other.vtable_;
		return *this;
	}

	TaskWrapperP& operator=(TaskWrapperP&& other) noexcept {
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
	TaskWrapperP(Callable c)
		: vtable_{ &_detail::vtable_p_for<Callable, R, Ts...> }
	{
		static_assert(sizeof(Callable) < sizeof(buf_),
			"Wrapper buffer is too small.");
		new(&buf_) Callable{ std::move(c) };
	}

	R operator()(Ts ...args) {
		if (!vtable_) {
			throw std::runtime_error{"Calling unitialized function wrapper!"};
		}
		return vtable_->run(&buf_, args...);
	}

private:
	std::aligned_storage_t<64> buf_;
	const _detail::vtable_p<R(Ts...)>* vtable_;
};
