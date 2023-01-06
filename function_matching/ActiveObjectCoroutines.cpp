#include "ActiveObjectCoroutines.h"

#include <iostream>

void VoiceMenuHandlerCoroutines::receiveInput(const MenuInput& data) {
	// Since the CoroutineTask::promise_type defines its 'initial_suspend'
	// method to return 'std::suspend_always' the coroutine will be in
	// suspended state after its creation, so its body only runs when
	// it is explicitly resumed. Resume will happen in worker task.
	worker_.addTask( processInput(data).h_ );
}

void VoiceMenuHandlerCoroutines::receiveHangup(const HangUp& data) {
	// Note that we use the same queue and worker thread to run normal
	// tasks and coroutines.
	worker_.addTask(TaskWrapper{ [this, data]() { processHangup(data); } });
}

std::string VoiceMenuHandlerCoroutines::fetchMenuSectionPrompt(
	char digit,
	const std::string& callId
) {
	// Given the current state and user input, go to next menu item.
	std::cout << "!Coroutine! - in the call [" << callId
		<< "] menu item '" << digit << "' selected.\n";
	return callId + "_prompt_" + digit;
}

void VoiceMenuHandlerCoroutines::playVoiceMenuPrompt(
	const std::string& callId,
	const std::string& prompt
) {
	// Command media server to start playing current menu prompt.
	std::cout << "play prompt [" << prompt << "]\n";
}

CoroutineTask VoiceMenuHandlerCoroutines::processInput(const MenuInput data) {
	const auto prompt = fetchMenuSectionPrompt(data.digit, data.callId);
	playVoiceMenuPrompt(data.callId, prompt);
	co_return;
	// Coroutine state will be destroyed after this point.
}

void VoiceMenuHandlerCoroutines::cleanupCallData(const std::string& callId) {
	// Free the resources that we used to serve the call.
	std::cout << "!Coroutine! - call [" << callId << "] ended.\n";
}

void VoiceMenuHandlerCoroutines::processHangup(const HangUp data) {
	cleanupCallData(data.callId);
}
