#pragma once

#include <string>

struct MenuInput {
	char digit;
	std::string callId;
};

struct HangUp {
	std::string callId;
};
