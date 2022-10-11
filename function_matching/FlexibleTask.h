#pragma once

#include <tuple>

template <typename R, typename C, typename... Ts>
class Task {
public:
	using callable_type = R(C::*)(Ts...);
	Task(callable_type func, C* instance, Ts... args)
		: func_{ func }, instance_{ instance }, args_{ args... }
	{}
	R operator()() {
		std::apply(
			[this](auto& ...args) {
				std::invoke(func_, instance_, args...);
			},
			args_
				);
	}
private:
	callable_type func_;
	C* instance_;
	std::tuple<Ts...> args_;
};
