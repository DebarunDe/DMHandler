#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeMessageQueue {
private:
    std::deque<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condVar_;

public:
    //constructor
    ThreadSafeMessageQueue() = default;

    //destructor
    ~ThreadSafeMessageQueue() = default;

    //prevent copying
    ThreadSafeMessageQueue(const ThreadSafeMessageQueue&) = delete;
    ThreadSafeMessageQueue& operator=(const ThreadSafeMessageQueue&) = delete;
    
    //allow moving
    ThreadSafeMessageQueue(ThreadSafeMessageQueue&&) = default;
    ThreadSafeMessageQueue& operator=(ThreadSafeMessageQueue&&) = default;

    //accessors
    const std::deque<T>& getQueue() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_;
    }

    //functions
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        condVar_.wait(lock, [this] { return !queue_.empty();});
        T item = std::move(queue_.front());
        queue_.pop_front();
        return item;
    }

    //rval push
    void push(T&& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.emplace_back(std::move(item));
        }
        condVar_.notify_one();
    }

    //lval push
    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.emplace_back(item);
        }
        condVar_.notify_one();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    T top() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) throw std::runtime_error("Queue is empty, cannot access top element.");
        return queue_.front();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }

};