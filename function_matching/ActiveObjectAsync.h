#pragma once

#include <string>

#include "InputStruct.h"
#include "WorkerThread.h"
#include "CoroutineTask.h"
#include "TaskWrapper.h"
#include "PromptFetcher.h"

class VoiceMenuHandlerAsync {
public:
	using worker_type = Worker<TaskWrapper>;

	explicit VoiceMenuHandlerAsync(PromptFetcher& fetcher);

	void receiveInput(const MenuInput& data);
	void receiveHangup(const HangUp& data);

private:

	// In order to be compatible with co_await a function must return an
	// awaiter, that implements three special methods.
	struct AwaitablePrompt
	{
		std::string callId;
		char digit;
		PromptFetcher& fetcher_;
		std::string prompt_;

		// Here they are -
		bool await_ready();
		void await_suspend(std::coroutine_handle<> h);
		std::string await_resume();
	};

	// This will be an asynchronous I/O function. It does not have to be a full
	// coroutine, but have to return an awaiter.
	AwaitablePrompt fetchMenuSectionPrompt(char digit, const std::string& callId);
	void playVoiceMenuPrompt(
		const std::string& callId, const std::string& prompt);
	// This is an asynchronous coroutine now.
	CoroutineTask processInput(const MenuInput data);

	void cleanupCallData(const std::string& callId);
	// This one remains as before but runs in the same thread as coroutines.
	void processHangup(const HangUp data);

	PromptFetcher& fetcher_;
	worker_type worker_;
};
