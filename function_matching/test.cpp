#include <functional>
#include <iostream>
#include <queue>

#include "SimpleTask.h"
#include "FlexibleTask.h"
#include "TaskWrapper.h"
#include "TaskWrapperComplete.h"


class SimpleTaskHandler {
public:

	struct Data {
		float k;
		float x;
		float b;
	};

	void process(const Data& data) {
		double result = data.k * data.x + data.b;
	}
};


class TaskHandler {
public:
	void foo(float k, float x, float b) {
		double result = k * x + b;
	}
};

void SimpleTaskTest()
{
	SimpleTaskHandler handler;
	std::queue<TaskWrapper> queue;
	for (int i = 0; i < 1000; ++i) {
		queue.push(SimpleTask{ &handler, SimpleTaskHandler::Data{0.3f, 1.4f * i, 33.7f} });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}


void TaskTest()
{
	TaskHandler handler;
	std::queue<TaskWrapper> queue;
	for (int i = 0; i < 1000; ++i) {
		queue.push(Task{ &TaskHandler::foo, &handler, 0.3f, 1.4f * i, 33.7f });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}


void TaskCompleteWrapperTest()
{
	TaskHandler handler;
	std::queue<TaskWrapperComplete> queue;
	for (int i = 0; i < 1000; ++i) {
		queue.push(Task{ &TaskHandler::foo, &handler, 0.3f, 1.4f * i, 33.7f });
	}
	while (!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}


void StdFunctionTest() {
	TaskHandler handler;
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


class Functor {
public:
	Functor() {
		std::cout << "Ctor\n";
	}

	Functor(const Functor& other) {
		std::cout << "Copy ctor\n";
	}

	Functor(Functor&& other) noexcept {
		std::cout << "Move ctor\n";
	}

	Functor& operator=(const Functor& other) {
		std::cout << "Assign copy\n";
	}

	Functor& operator=(Functor&& other) noexcept {
		std::cout << "Assign move\n";
	}

	~Functor() {
		std::cout << "Dtor\n";
	}

	void operator()() {
		std::cout << "operator()\n";
	}
};


int main(int argc, char** argv) {
	SimpleTaskHandler handler;
	auto tsk = SimpleTask{ &handler, SimpleTaskHandler::Data{0.3f, 1.4f, 33.7f} };
	tsk();

	TaskHandler t;
	auto tsk2 = Task{ &TaskHandler::foo, &t, 0.3f, 1.4f, 33.7f };
	tsk2();

	std::queue<TaskWrapper> taskQueue;
	taskQueue.push(TaskWrapper{ tsk });
	taskQueue.push(TaskWrapper{ tsk2 });
	taskQueue.front()();
	taskQueue.pop();
	taskQueue.front()();
	taskQueue.pop();
	taskQueue.push([&tsk]() { tsk(); }); // Works with anything with the right method!
	taskQueue.front()();
	taskQueue.pop();

	{
		std::cout << "Primitive wrapper:\n";
		TaskWrapper wrap{ Functor{} };
		wrap();
		TaskWrapper wrap2{ wrap };
		wrap2();
		TaskWrapper wrap3{ std::move(wrap) };
		wrap3();
	}

	{
		std::cout << "\nComplete wrapper:\n";
		TaskWrapperComplete wrap{ Functor{} };
		wrap();
		TaskWrapperComplete wrap2{ wrap };
		wrap2();
		TaskWrapperComplete wrap3{ std::move(wrap) };
		wrap3();
	}

	return 0;
}
