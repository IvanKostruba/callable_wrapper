#pragma once

// This task class assumes specific name and signature of data processing method.
// 'void process(Data& data);'
template<typename Handler, typename Data>
class SimpleTask {
public:
	SimpleTask(Handler* handler, Data data) : handler_{ handler }, data_{ data }
	{}

	void operator()() {
		handler_->process(data_);
	}

private:
	Handler* handler_;
	Data data_;
};
