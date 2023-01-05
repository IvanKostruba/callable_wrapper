#pragma once

#include <string>

#include "InputStruct.h"
#include "WorkerThread.h"
#include "CoroutineTask.h"

class VoiceMenuHandlerCoroutines {
public:
	void receiveInput(const MenuInput& data);
	void receiveHangup(const HangUp& data);

private:

	std::string fetchMenuItemPrompt(char digit, const std::string& callId);
	void playVoiceMenuPrompt(
		const std::string& callId, const std::string& prompt);
	CoroutineTask process(const MenuInput data);

	void cleanupCallData(const std::string& callId);
	CoroutineTask process(const HangUp data);

	Worker<std::coroutine_handle<>> worker_;
};
