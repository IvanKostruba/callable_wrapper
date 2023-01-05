#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <system_error>
#include <thread>

template <typename QueueItem>
struct Worker {

    Worker() {
        thread_ = std::thread(
            [this]() {
                while (!stop_) {
                    std::unique_lock<std::mutex> lk(queue_lock_);
                    cv_.wait(lk);
                    if (stop_ || queue_.empty()) {
                        continue;
                    }
                    auto task = QueueItem{ std::move(queue_.front()) };
                    queue_.pop();
                    lk.unlock();
                    task();
                }
            }
        );
    }

    Worker(const Worker& other) = delete;
    Worker(Worker&& other) noexcept = delete;
    Worker& operator=(const Worker& other) = delete;
    Worker& operator=(Worker&& other) noexcept = delete;

    ~Worker() {
        stop_ = true;
        cv_.notify_one();
        if (thread_.joinable()) {
            try {
                thread_.join();
            }
            catch (std::system_error e) {
            }
        }
    }

    void addTask(QueueItem task) {
        {
            std::lock_guard<std::mutex> lk(queue_lock_);
            queue_.emplace(std::move(task));
        }
        cv_.notify_one();
    }

    std::condition_variable cv_;
    std::mutex queue_lock_;
    std::queue<QueueItem> queue_;
    std::thread thread_;
    bool stop_{false};
};
