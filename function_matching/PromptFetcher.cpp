#include "PromptFetcher.h"

#include <iostream>

void PromptFetcher::fetch(
	const std::string& callId,
	char digit,
	wrapper_type callback
) {
	std::cout << "fetch request sent callId [" << callId
		<< "], input = " << digit << ".\n";
	requestCallback.insert(std::make_pair(callId, std::move(callback)));
}

void PromptFetcher::processResponse(
	const std::string& callId,
	const std::string& response
) {
	auto it = requestCallback.find(callId);
	if (it != requestCallback.end()) {
		std::cout << "received response for [" << callId << "]\n";
		// Invoking the callback that we received from an awaiter. The callback
		// will schedule the coroutine resume on the worker thread that we
		// pass here.
		it->second(response, worker_);
	}
}
