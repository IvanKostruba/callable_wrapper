#pragma once

#include <coroutine>
#include <iostream>
#include <string>
#include <utility>
#include <unordered_map>

#include "TaskWrapperParameterized.h"
#include "WorkerThread.h"

// An imitation of asynchronous I/O.
class PromptFetcher {
public:
	using worker_type = Worker<std::coroutine_handle<>>;
	using wrapper_type = TaskWrapperP<void(const std::string&, worker_type&)>;

	void fetch(
		const std::string& callId, char digit, wrapper_type callback);
	void processResponse(const std::string& callId,const std::string& response);

private:
	std::unordered_map<std::string, wrapper_type> requestCallback;
	// This is not strictly necessary, but I want to demonstrate how a coroutine
	// can be resumed on a different thread, also the PromptFetcher could use
	// it's own coroutines as well.
	worker_type worker_;
};
