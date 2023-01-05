#include <functional>
#include <iostream>
#include <queue>
#include <string>

#include "InputStruct.h"
#include "SimpleTask.h"
#include "FlexibleTask.h"
#include "TaskWrapper.h"
#include "CtorLogger.h"
#include "ActiveObject.h"
#include "ActiveObjectCoroutines.h"
#include "ActiveObjectAsync.h"
#include "PromptFetcher.h"


class TaskHandlerWithArbitraryMethodName {
public:
	void foo(float k, float x, float b) {
		double result = k * x + b;
	}
};


int main(int argc, char** argv) {

	// See, how task is created and copied/moved with the wrapper.
	{
		std::cout << "\nComplete wrapper:\n";
		std::cout << "Create and call:\n";
		TaskWrapper wrap{ CtorLogger{} };
		wrap();
		std::cout << "\nMake a copy and call:\n";
		TaskWrapper wrap2{ wrap };
		wrap2();
		std::cout << "\nMove and call:\n";
		TaskWrapper wrap3{ std::move(wrap) };
		wrap3();
		std::cout << "\nEverything goes out of the scope and is destroyed:\n";
	}

	VoiceMenuHandler menuHandler;
	std::thread sender([&menuHandler]() {
		menuHandler.receiveInput(
			MenuInput{ '2', "call_1@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandler.receiveInput(
			MenuInput{ '1', "call_2@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandler.receiveHangup(
			HangUp{ "call_1@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	});
	sender.join();


	std::cout << "\n";
	VoiceMenuHandlerCoroutines menuHandlerCoroutines;
	std::thread senderToCoroutines([&menuHandlerCoroutines]() {
		menuHandlerCoroutines.receiveInput(
			MenuInput{ '5', "call_coro_3@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandlerCoroutines.receiveInput(
			MenuInput{ '4', "call_coro_4@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandlerCoroutines.receiveHangup(
			HangUp{ "call_coro_3@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	});
	senderToCoroutines.join();

	std::cout << "\n";
	PromptFetcher fetcher;
	VoiceMenuHandlerAsync menuHandlerAsync{ fetcher };
	std::thread senderToAsync([&menuHandlerAsync, &fetcher]() {
		menuHandlerAsync.receiveInput(
			MenuInput{ '7', "call_async_9@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandlerAsync.receiveInput(
			MenuInput{ '8', "call_async_8@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// Play the prompt on the fetcher worker thread.
		fetcher.processResponse("call_async_8@ip_addr", "prompt_AAA");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// Play the prompt on the fetcher worker thread.
		fetcher.processResponse("call_async_9@ip_addr", "prompt_BBB");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		menuHandlerAsync.receiveHangup(
			HangUp{ "call_async_8@ip_addr" });
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		});
	senderToAsync.join();

	return 0;
}

// PERFORMANCE BENCHMARK SNIPPETS, see 

//void SimpleTaskTest()
//{
//	VoiceMenuHandler handler;
//	std::queue<TaskWrapper> queue;
//	for (int i = 0; i < 1000; ++i) {
//		queue.push(SimpleTask{ &handler, VoiceMenuHandler::MenuInput{'3', "call_1@ip_addr"} });
//	}
//	while (!queue.empty()) {
//		queue.front()();
//		queue.pop();
//	}
//}

void FlexibleTaskTest()
{
	TaskHandlerWithArbitraryMethodName handler;
	std::queue<TaskWrapper> queue;
	for (int i = 0; i < 1000; ++i) {
		queue.push(FlexibleTask{ &TaskHandlerWithArbitraryMethodName::foo, &handler, 0.3f, 1.4f * i, 33.7f });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}

void TaskCompleteWrapperTest()
{
	TaskHandlerWithArbitraryMethodName handler;
	std::queue<TaskWrapper> queue;
	for (int i = 0; i < 1000; ++i) {
		queue.push(FlexibleTask{ &TaskHandlerWithArbitraryMethodName::foo, &handler, 0.3f, 1.4f * i, 33.7f });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}

void StdFunctionTest() {
	TaskHandlerWithArbitraryMethodName handler;
	std::queue<std::function<void()>> queue;
	float k{ 0.3f };
	float b{ 33.7f };
	for (int i = 0; i < 1000; ++i) {
		float x{ 1.4f * i };
		queue.push([k, x, b, &handler]() { handler.foo(k, x, b); });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}
