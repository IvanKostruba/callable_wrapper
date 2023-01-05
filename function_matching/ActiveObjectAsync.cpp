#include "ActiveObjectAsync.h"

#include <iostream>

VoiceMenuHandlerAsync::VoiceMenuHandlerAsync(
	PromptFetcher& fetcher
) : fetcher_{ fetcher } {
}

void VoiceMenuHandlerAsync::receiveInput(const MenuInput& data) {
	// Since the CoroutineTask::promise_type defines its 'initial_suspend'
	// method to return 'std::suspend_always' the coroutine will be in
	// suspended state after its creation, so its body only runs when
	// it is explicitly resumed. Resume will happen in worker task.
	worker_.addTask(process(data).h_);
}

void VoiceMenuHandlerAsync::receiveHangup(const HangUp& data) {
	worker_.addTask(process(data).h_);
}

VoiceMenuHandlerAsync::AwaitablePrompt
VoiceMenuHandlerAsync::fetchMenuItemPrompt(
	char digit,
	const std::string& callId
) {
	// Given the current state and user input, go to next menu item.
	std::cout << "!Coroutine! - in call [" << callId
		<< "] menu item '" << digit << "' selected.\n";
	return AwaitablePrompt{ callId, digit, fetcher_ };
}

void VoiceMenuHandlerAsync::playVoiceMenuPrompt(
	const std::string& callId,
	const std::string& prompt
) {
	// Command media server to start playing current menu prompt.
	std::cout << "play prompt [" << prompt << "]\n";
}

CoroutineTask VoiceMenuHandlerAsync::process(const MenuInput data) {
	// co_await is compiled into the following sequence:
	// if(!AwaitablePrompt::await_ready()) {
	//   AwaitablePrompt::await_suspend(current_coro_handle);
	//   // depending on what await_suspend returns continuation
	//   // may be different.
	// }
	// AwaitablePrompt::await_resume()
	// whatewer await_resume returns becomes the result of co_await
	const auto prompt = co_await fetchMenuItemPrompt(data.digit, data.callId);
	playVoiceMenuPrompt(data.callId, prompt);
	co_return;
	// Coroutine state will be destroyed after this point.
}

void VoiceMenuHandlerAsync::cleanupCallData(const std::string& callId) {
	// Free the resources that we used to serve the call.
	std::cout << "!Coroutine! - call [" << callId << "] ended.\n";
}

CoroutineTask VoiceMenuHandlerAsync::process(const HangUp data) {
	cleanupCallData(data.callId);
	co_return;
	// Coroutine state will be destroyed after this point.
}

//-----------------------------------------------------------------------------

bool VoiceMenuHandlerAsync::AwaitablePrompt::await_ready() {
	// If PromptFetcher had a cache, we could check it here and skip suspension
	// of the coroutine by returning 'true'.
	return false;
}

void VoiceMenuHandlerAsync::AwaitablePrompt::await_suspend(
	std::coroutine_handle<> h
) {
	// When this method is called the coroutine is already suspended, so we can
	// pass it's handle elsewhere.
	fetcher_.fetch(
		callId,
		digit,
		[this, h](const std::string& prompt, worker_type& worker) {
			// co_await resumes execution right before 'await_resume' call
			// so it is safe to assign to prompt here because awaiter lifetime
			// lasts until after 'await_resume' return.
			prompt_ = prompt;
	        // Worker to schedule the coroutine on will be provided externally.
			// Alternatively we could pass coroutine_handle to the fetcher
			// and then the fetcher would be responsible for scheduling it.
			worker.addTask(h);
			// from this point awaiter may be deleted at any moment so we
			// should not touch it (this) anymore.
		}
	);
}

std::string VoiceMenuHandlerAsync::AwaitablePrompt::await_resume() {
	// This will become the result of co_await expression.
	return prompt_;
}
