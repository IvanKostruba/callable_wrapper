#pragma once

#include <string>

#include "InputStruct.h"
#include "TaskWrapper.h"
#include "WorkerThread.h"

class VoiceMenuHandler {
public:
	// These methods will return immediately, deferring the actual work.
	void receiveInput(const MenuInput& data);
	void receiveHangup(const HangUp& data);

private:

	std::string fetchMenuSectionPrompt(char digit, const std::string& callId);
	void playVoiceMenuPrompt(
		const std::string& callId, const std::string& prompt);
	// This method will do actual processing of user input by calling the
	// two functions from above.
	void processInput(const MenuInput& data);

	void cleanupCallData(const std::string& callId);
	// This method will do actual processing of hangup by calling the
	// function from above.
	void processHangup(const HangUp& data);

	// The thread where the tasks will run.
	Worker<TaskWrapper> worker_;
};
