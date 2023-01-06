#include "ActiveObject.h"

#include <iostream>

void VoiceMenuHandler::receiveInput(const MenuInput& data) {
	// Instead of processing the input synchronously, we schedule it for later,
	// and return immediately, not wasting the caller's time.
	worker_.addTask(TaskWrapper{[this, data]() { processInput(data); } });
}

void VoiceMenuHandler::receiveHangup(const HangUp& data) {
	// Instead of processing the input synchronously, we schedule it for later,
	// and return immediately, not wasting the caller's time.
	worker_.addTask(TaskWrapper{[this, data]() { processHangup(data); } });
}

std::string VoiceMenuHandler::fetchMenuSectionPrompt(
	char digit,
	const std::string& callId
) {
	// Given the current state and user input, go to the next menu item.
	std::cout << "in the call [" << callId << "] menu item '"
		<< digit << "' selected.\n";
	return callId + "_prompt_" + digit;
}

void VoiceMenuHandler::playVoiceMenuPrompt(
	const std::string& callId,
	const std::string& prompt
) {
	// Command media server to start playing the current menu prompt.
	std::cout << "play prompt [" << prompt << "]\n";
}

void VoiceMenuHandler::processInput(const MenuInput& data) {
	const auto prompt = fetchMenuSectionPrompt(data.digit, data.callId);
	playVoiceMenuPrompt(data.callId, prompt);
}

void VoiceMenuHandler::cleanupCallData(const std::string& callId) {
	// Free the resources that we used to serve the call.
	std::cout << "call [" << callId << "] ended.\n";
}

void VoiceMenuHandler::processHangup(const HangUp& data) {
	cleanupCallData(data.callId);
}
