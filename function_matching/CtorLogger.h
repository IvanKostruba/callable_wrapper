#pragma once

#include <iostream>

class CtorLogger {
public:
	CtorLogger() {
		std::cout << "Ctor\n";
	}

	CtorLogger(const CtorLogger& other) {
		std::cout << "Copy ctor\n";
	}

	CtorLogger(CtorLogger&& other) noexcept {
		std::cout << "Move ctor\n";
	}

	CtorLogger& operator=(const CtorLogger& other) {
		std::cout << "Assign copy\n";
	}

	CtorLogger& operator=(CtorLogger&& other) noexcept {
		std::cout << "Assign move\n";
	}

	~CtorLogger() {
		std::cout << "Dtor\n";
	}

	void operator()() {
		std::cout << "operator()\n";
	}
};
