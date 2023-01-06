#pragma once

#include <string>

#include "InputStruct.h"
#include "WorkerThread.h"
#include "CoroutineTask.h"
#include "TaskWrapper.h"

class VoiceMenuHandlerCoroutines {
public:
	void receiveInput(const MenuInput& data);
	void receiveHangup(const HangUp& data);

private:

	std::string fetchMenuSectionPrompt(char digit, const std::string& callId);
	void playVoiceMenuPrompt(
		const std::string& callId, const std::string& prompt);
	// This is a coroutine now.
	CoroutineTask processInput(const MenuInput data);

	void cleanupCallData(const std::string& callId);
	// This one remains as before but runs in the same thread as coroutines.
	void processHangup(const HangUp data);

	Worker<TaskWrapper> worker_;
};
